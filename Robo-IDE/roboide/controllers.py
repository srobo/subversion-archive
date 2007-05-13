from turbogears import controllers, expose
import cherrypy
import logging
import pysvn
import time
import re
import tempfile, shutil
import os
from os.path import join
log = logging.getLogger("roboide.controllers")

REPO = "http://studentrobotics.org/svn/"

class Root(controllers.RootController):
    @expose("json")
    def filesrc(self, file=None, revision="HEAD"):
        """
        Returns the contents of the file.
        Turns out the action parameter can be edit. Not sure how this is
        useful - won't it always be edit?
        TODO: Cope with revision other than head.
        """
        curtime = time.time()
        client = pysvn.Client()
        
        #TODO: Need to security check here! No ../../ or /etc/passwd nautiness

        if file != None and file != "" and client.is_url(REPO + file):
            #Load file from SVN
            mime = ""
            try:
                mime = client.propget("svn:mime-type", REPO+file).values()[0]
            except pysvn.ClientError:
                code = "Error getting mime type"
                revision = 0
            except IndexError:
                pass

            if mime == "application/octet-stream":
                code = "Binary File"
                revision = 0
            else:
                #Ugforge doesn't support locking, so do this the hard way...
                while True:
                    try:
                        ver = client.log(REPO + file, limit=1)[0]["revision"]
                        code = client.cat(REPO + file)
                        ver2 = client.log(REPO + file, limit=1)[0]["revision"]
                        if ver2.number == ver.number:
                            revision = ver.number
                            break
                        else:
                            print "Collision catting %s. Should be v rare!" % \
                                REPO + file
                    except pysvn.ClientError:
                        code = "No file loaded."
                        revision = 0
        else:
            code = "No File Loaded"
            revision = 0

        return dict(curtime=curtime, code=code, revision=revision, path=file)

    #TODO: Create an action that uses client.log to return a JSON list of
    #previous file revisions for a mochikit drop down
    @expose("json")
    def gethistory(self, file):
        c = pysvn.Client()
        try:
            log = c.log(REPO+file)
        except:
            print "LOG FAILED"
            return dict([])

        return dict(history=[{"author":x["author"], \
                      "date":time.strftime("%H:%M:%S %d/%m/%Y", \
                      time.localtime(x["date"])), \
                      "message":x["message"], "rev":x["revision"].number} \
                      for x in log])

    @expose("json")
    def savefile(self, file, rev, message, code):
        """Write a commit of one file.
        1. SVN checkout the file's directory
        2. Dump in the new file data
        3. Commit that directory with the new data and the message
        4. Wipe the directory

        TODO: Usernames.
        """
        try:
            rev = int(rev)
            if rev == 0:
                raise
        except:
            return dict(new_revision=str(0), code="",
                    success="Invalid revision")
   
            
        client = pysvn.Client()
        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness
        path = os.path.dirname(file)
        basename = os.path.basename(file)
        tmpdir = tempfile.mkdtemp()
        #This returns a revision number. Always 0. Great.
        try:
            client.checkout(REPO + path, tmpdir, recurse=False, \
                revision=pysvn.Revision(pysvn.opt_revision_kind.number, int(rev)))
        except pysvn.ClientError:
            #Some fool (me) passed in a filename that doesn't exist.
            return dict(new_revision="0", code="", success="Invalid filename")

        #2. Dump in the new file data
        target = open(join(tmpdir, basename), "wt")
        target.write(code)
        target.close()

        #3. Commit the new directory
        try:
            newrev = client.checkin([tmpdir], message)
            newrev = newrev.number
            success = "True"
            code = ""
        except pysvn.ClientError:
            #Can't commit - merge issues
            #Need to bring local copy up to speed
            #Hopefully this means a merge!
            #Throw the new contents of the file back to the client for
            #tidying, then they can resubmit
            newrev = client.update(tmpdir)[0] #This comes back as a list.
            if newrev == None:
                #No update to be made.
                success = "True"
                code = ""
                newrev = 0
            else:
                success = "Merge"
                #Grab the merged text.
                mergedfile = open(join(tmpdir, basename), "rt")
                code = mergedfile.read()
                mergedfile.close()
                newrev = newrev.number

        #4. Wipe the directory
        shutil.rmtree(tmpdir)

        return dict(new_revision=str(newrev), code=code,
                    success=success)

    @expose(template="roboide.templates.files")
    def index(self):
        #Really need to seperate this out in a min
        client = pysvn.Client()
        
        files = client.ls(REPO, recurse=True)

        class Node (object):
            def __init__(self, name, path, kind):
                self.name = name
                self.path = path
                self.kind = kind
                self.children = {}
                
        head = Node("HEAD", "", "")

        for details in files:
            filename = details["name"][len(REPO):] #Strip off the repo URL
            basename = filename.split("/")[-1:][0]
            top = head 
            for path in filename.split("/"):
                try: 
                    top = top.children[path]
                except KeyError:
                    top.children[path] = Node(basename, filename, details["kind"])

        return dict(tree=head)
