from turbogears import config, expose
import model
import cherrypy
import sr
import sha, datetime, time
import os.path
from cherrypy.lib.cptools import serveFile

def datetime_to_stamp(d):
    return int( time.mktime( d.timetuple() ) )

class FwServe(object):
    @expose("json")
    def devices(self):
        """Get a list of devices"""
        s = model.FirmwareTargets.select()
        devices = [x.name for x in list(s)]
        return { "devices": devices }

    def __check_rev(self, rev):
        """Check that a string describing a revision is valid."""
        if rev == "dev":
            return True

        if len(rev) < len("svn:0"):
            return False

        if rev[0:4] != "svn:":
            return False

        # Check the revision number is all numbers
        if False in [x.isdigit() for x in rev[4:]]:
            return False

        return True

    def __find_device(self, device):
        """Returns the ID of the given device"""
        r = model.FirmwareTargets.select( model.FirmwareTargets.q.name == device )
        
        if r.count() == 0:
            return False

        if r.count() > 1:
            raise "More than one device registered with the name '%s'" % device

        return r[0].id

    def __find_new_ver(self, device_id):
        """Find a new, unused firmware version."""
        r = model.FirmwareBlobs.select( model.FirmwareBlobs.q.device == device_id )
        
        if r.count() == 0:
            return 0

        return r.max("version") + 1

    def __add_state(self, fw_id, message, state):
        """Add a status change message to a firmware image"""
        f = model.FirmwareState( fw_id = fw_id,
                                 date = datetime.datetime.now(),
                                 message = message,
                                 state = state )

    def __get_state(self, fw_id):
        """Get the state of the given device image."""
        r = model.FirmwareState.select( model.FirmwareState.q.fw_id == fw_id ).orderBy("-date")
        if r.count() == 0:
            return "NONE"

        latest = r[0]
        
        return latest.state

    # In future will require login:
    @expose("json")
    def req_version(self,device,desc,revision):
        """Get a new version number for a firmware image."""
        
        if not self.__check_rev(revision):
            return {"ERROR": "Invalid VC revision string"}

        dev_id = self.__find_device(device)
        if dev_id == False:
            return {"ERROR": "Device '%s' not found" % device}

        version = self.__find_new_ver(dev_id)

        nver = model.FirmwareBlobs( device = dev_id,
                                    version = version,
                                    firmware = "JAM",
                                    revision = revision,
                                    desc = desc )


        self.__add_state( fw_id = nver.id,
                          message = "Version number allocated",
                          state = "ALLOCATED" )

        return { "version" : version }

    # In future will require login:
    @expose("json")
    def upload(self,device,version,firmware):
        """Upload a firmware image to the system."""
        version = int(version)

        dev_id = self.__find_device(device)
        if dev_id == False:
            return {"ERROR": "Device '%s' not found" % device}

        # Check the version is correct
        r = model.FirmwareBlobs.selectBy( device = dev_id, version = version )
        if r.count() == 0:
            return {"ERROR": "Version %i does not exist" % version}
        fw = r[0]

        if self.__get_state(fw.id) != "ALLOCATED":
            return {"ERROR": "Firmware already uploaded"}
        
        data = firmware.file.read()

        s = sha.new()
        s.update( data )

        # Use the SHA1 as the filename
        fw.firmware = s.hexdigest()

        f = open( "%s/%s" % (config.get("firmware.dir"), fw.firmware), "w" )
        f.write( data )
        f.close()

        self.__add_state( fw.id, "Firmware uploaded", "DEVEL" )

        return { "sha1": s.hexdigest() }

    @expose()
    def get(self,device,version):
        """Return the firmware image for the given device."""
        version = int(version)
        dev_id = self.__find_device( device )

        if dev_id == False:
            return {"ERROR" : "Invalid device '%s'" % device}

        r = model.FirmwareBlobs.selectBy( version = version, device = dev_id )

        if r.count() == 0:
            return {"ERROR" : "Version '%i' doens't exist for device '%s'." % (version, device)}

        cherrypy.response.headers['Content-Type'] = "application/x-download"
        cherrypy.response.headers['Content-Disposition'] = 'attachment; filename="%s-%i"' % ( device, version )

        f = open( "%s/%s" % (config.get("firmware.dir"), r[0].firmware), "r" )

        return f.read()

    @expose("json")
    def info(self,device,version):
        """Return information about the given firmware image version."""
        version = int(version)

        dev_id = self.__find_device(device)
        if dev_id == False:
            return {"ERROR": "Device '%s' not found" % device}

        r = model.FirmwareBlobs.selectBy( device = dev_id, version = version )
        if r.count() == 0:
            return {"ERROR": "Version %i does not exist" % version}
        fw = r[0]

        r = model.FirmwareState.selectBy( fw_id = fw.id ).orderBy("date")
        log = []
        for entry in r:
            log.append( { "time": datetime_to_stamp(entry.date),
                          "message": entry.message,
                          "state": entry.state } )

        info = { "desc": fw.desc,
                 "state": r[-1].state,
                 "log" : log }

        if info["state"] != "ALLOCATED":
            "The hash and size only make sense once the file's been uploaded "
            h = sha.new()
            f = open( "%s/%s" % (config.get("firmware.dir"), fw.firmware), "r" )
            h.update( f.read() )

            info["sha1"] = h.hexdigest();
            info["size"] = os.path.getsize( "%s/%s" % (config.get("firmware.dir"), fw.firmware));

        return info

    @expose("json")
    def images(self,device):
        """Return information about the firmwares for the given device."""
        dev_id = self.__find_device(device)
        if not dev_id:
            return {"ERROR": "Device '%s' not found" % device}

        state = {}
        for x in [ "ALLOCATED", 
                   "DEVEL", 
                   "TESTING", 
                   "SHIPPING", 
                   "FAILED", 
                   "OLD_RELEASE",
                   "SUPERCEDED",
                   "NONE" ]:
            state[x] = []

        for fw in model.FirmwareBlobs.selectBy( device = dev_id ):
            state[self.__get_state(fw.id)].append(fw.version)

        return state

    @expose("json")
    def sign(self,device,version,signature):
        """ """
        return "Firmware signing not yet implemented!"


