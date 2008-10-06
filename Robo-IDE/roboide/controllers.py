from turbogears import controllers, expose, config
from turbogears.feed import FeedController
import cherrypy, model
import logging
import pysvn
import time, datetime
import re
import tempfile, shutil
import os, sys, os.path
import zipfile
import random
from os.path import join
from cherrypy.lib.cptools import serveFile
import sr
import user as srusers

log = logging.getLogger("roboide.controllers")

ZIPNAME = "robot.zip"
SYSFILES = "/srv/sysfiles"

class Client:
    """
    A wrapper around a pysvn client. This creates a client and wraps calls to
    its functions.
    """
    client = None
    def __init__(self, team):
        """
        Create a pysvn client and use it
        """
        def get_login(realm, username, may_save):
            user = get_curuser()
            return True, user, "", False

        c = pysvn.Client()
        c.callback_get_login = get_login
        c.set_store_passwords(False)
        c.set_auth_cache(False)
        
        #Using self.__dict__[] to avoid calling setattr in recursive death
        self.__dict__["client"] = c
        if not team in srusers.getteams():
            raise RuntimeError("User can not access team %d" % team)

        self.__dict__["REPO"] = srusers.get_svnrepo( team )

    def is_url(self, url):
        """Override the default is_url which just tells you if the url looks
        sane. This tries to get info on the file...
        """
        try:
            self.client.info2(url)
            return True
        except pysvn.ClientError:
            return False

    def __setattr__(self, name, val):
        """
        This special method is called when setting a value that isn't found
        elsewhere. It sets the same named value of the pysvn client
        """
        #BE CAREFUL - assigning class-scope variables anywhere in this class
        #causes instant setattr recursion death
        setattr(self.client, name, val)

    def __getattr__(self, name):
        """
        This special method is called when something isn't found in the class
        normally. It returns the named attribute of the pysvn client this class
        is wrapping.
        """
        return getattr(self.client, name)

class Feed(FeedController):
    def get_feed_data(self):
        entries = [dict(author    = {"name" : x["author"]},
                        published = datetime.datetime.fromtimestamp(x["date"]),
                        title     = "Revision %d" % x["revision"].number,
                        summary   = x["message"],
                        link      = "http://ide.studentrobotics.org/")
                        for x in Client().log(REPO, limit=10)[0:10]]
        return dict(
            title = "RoboIde Data",
            link = "http://ide.studentrobotics.org/",
            author = {"name": "Student Robotics", "email": "info@studentrobotics.org"},
            id = "http://ide.studentrobotics.org",
            subtitle = "Recent commits to the repository",
            entries = entries
        )

class Root(controllers.RootController):
    user = srusers.User()
    #feed = Feed()

    @expose()
    def index(self):
        """
        In case the apache rewrite rule isn't paying attention, serve up the
        index file from here.
        """
        loc = os.path.join(os.path.dirname(__file__), "static/index.html")
        return serveFile(loc)

    def get_revision(self, revision):
        """
        Get a revision object.
        inputs:
            revision - revision number (convertable with int()). If can not be
            converted, returns HEAD revision
        returns:
            revision object for revision number"""

        try:
            if revision == 0 or revision == "0":
                revision = "HEAD"
            rev = pysvn.Revision(pysvn.opt_revision_kind.number, int(revision))
        except (pysvn.ClientError, ValueError, TypeError):
            rev = pysvn.Revision(pysvn.opt_revision_kind.head)
        return rev

    @expose()
    @srusers.require(srusers.in_team())
    def checkout(self, team, files="", simulator=False):
        """
        This function grabs a set of files and makes a zip available. Should be
        linked to directly.
        inputs:
            files - a comma seperated list of files to do the method on
        returns:
            A zip file as a downloadable file with appropriate HTTP headers
            sent.
        """
        client = Client(int(team))
        files = files.split(",")
        rev = self.get_revision("HEAD")

        #Need to check out the folders that contain the files
        #Create a folder to dump everything in
        root = tempfile.mkdtemp()

        dirs = [""] #List of directories already created

        if files == [""]:
            client.export(client.REPO,
                    root + "/all",
                    revision=rev,
                    recurse=True)
        else:
            for file in files:
                path = os.path.dirname(file)
                if not path in dirs:
                    #If the directory path isn't in the dirs list
                    #Need to create a directory for it
                    #Makedirs creates parent directories as necessary
                    os.makedirs(os.path.join(root, path))
                    #TODO: Is there a os.path to do this safely?
                    pathparts = path.split("/")
                    
                    #Need to put all created directories in dirs
                    for i in range(0, len(pathparts)):
                        #e.g. if path was moo/poo/loo
                        #subdir is:
                        #moo
                        #moo/poo
                        #moo/poo/loo
                        subdir = "/".join(pathparts[0:i+1])
                        if not subdir in dirs:
                            dirs.append(subdir)
                #Directory exists, copy file into it
                f = open(os.path.join(root, file), "wb")
                f.write(client.cat(client.REPO + file, rev))
                f.close()

        #Now should have a tree in root.
        #Create a zip file in a temporary directory
        zfile = tempfile.mktemp()
        zip = zipfile.ZipFile(zfile, "w")

        #Check for an all directory on its own in the root
        head = root
        if(os.listdir(root) == ["all"]):
            head = root + "/all"

        def add_dir_to_zip(head, zip):
            #Walk through the tree of files checked out and add them
            #to the zipfile
            for node, dirs, files in os.walk(head):
                for name in files:
                    #Skip .svn directories
                    if ".svn" in node:
                        continue
                    #If the file is in the root directory
                    if node == head:
                        #Add it named just its name
                        zip.write(os.path.join(node, name), name)
                    else:
                        #Add it with a suitable path
                        zip.write(os.path.join(node, name),
                            node[len(root)+1:]+"/"+name)
        
        add_dir_to_zip(head, zip)
        zip.close()
        #Zipfile now ready to be shipped.
        #Clean up root dir
        shutil.rmtree(root)

        #Set up headers for correctly serving a zipfile
        cherrypy.response.headers['Content-Type'] = \
                "application/x-download"
        cherrypy.response.headers['Content-Disposition'] = \
                'attachment; filename="' + ZIPNAME + '"'

        #Read the data in from the temporary zipfile
        zipdata = open(zfile, "rb").read()

        if not simulator:
            #Create a second zip file placing the user zip in the system zip
            syszfile = tempfile.mktemp()
            syszip = zipfile.ZipFile(syszfile, "w")
            add_dir_to_zip(SYSFILES, syszip)

            syszip.write(zfile, ZIPNAME)
            #Get rid of the temporary zipfile
            syszip.close() 

            #Read the data in from the temporary zipfile
            zipdata = open(syszfile, "rb").read()
            os.unlink(syszfile)

        os.unlink(zfile)

        #Return the data
        return zipdata

    @expose("json")
    @srusers.require(srusers.in_team())
    def filesrc(self, team, file=None, revision="HEAD"):
        """
        Returns the contents of the file.
        Turns out the action parameter can be edit. Not sure how this is
        useful - won't it always be edit?
        TODO: Cope with revision other than head.
        """
        curtime = time.time()
        client = Client(int(team))
        
        #TODO: Need to security check here! No ../../ or /etc/passwd nautiness

        rev = self.get_revision(revision)

        if file != None and file != "" and client.is_url(client.REPO + file):
            #Load file from SVN
            mime = ""
            try:
                mime = client.propget("svn:mime-type", client.REPO+file, revision=rev).values()[0]
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
                        ver = client.log(client.REPO + file, limit=1, revision_start=rev)[0]["revision"]
                        code = client.cat(client.REPO + file, revision=rev)
                        ver2 = client.log(client.REPO + file, limit=1,
                                revision_start=rev)[0]["revision"]
                        if ver2.number == ver.number:
                            revision = ver.number
                            break
                        else:
                            print "Collision catting %s. Should be v rare!" % \
                                client.REPO + file
                    except pysvn.ClientError:
                        code = "No file loaded."
                        revision = 0
        else:
            code = "No File Loaded"
            revision = 0

        return dict(curtime=curtime, code=code, revision=revision, path=file,
                name=os.path.basename(file))

    @expose("json")
    @srusers.require(srusers.in_team())
    def gethistory(self, team, file, user = None, offset = 0):
        #This function retrieves the svn log output for the given file(s)
        #to restrict logs to particular user, supply a user parameter
        #a maximum of 10 results are sent to the browser, if there are more than 10
        #results available, overflow > 0.
        #supply an offset to view older results: 0<offset < overflow; offset = 0 is the most recent logs
        offset = int(offset)
        c = Client(int(team))
        try:
            log = c.log(c.REPO+file)
        except:
            logging.debug("Log failed for %s" % c.REPO+file)
            return dict(path=file,history=[])
        authors = []
        #get a list of users based on log authors
        for y in log:
            if y['author'] not in authors:
                authors.append(y['author'])
                
        #narrow results by user (if supplied)
        result = []
        if user != None:
    	    for x in log:
                if(x['author'] == user) or (user == None):
    	            result.append(x)
        else:
            result = log[:]

        #if many results, split into pages of 10 and return appropriate 
        start = offset*10
        end = start + 10
        maxval = len(result)
        if maxval%10 > 0:  
            overflow = maxval/10 +1
        else:
            overflow = maxval/10

        result = result[start:end]

        return dict(  path=file, overflow=overflow, offset=offset, authors=authors,\
	                  history=[{"author":x["author"], \
	                  "date":time.strftime("%H:%M:%S %d/%m/%Y", \
	                  time.localtime(x["date"])), \
	                  "message":x["message"], "rev":x["revision"].number} \
	                  for x in result])

    def checkoutintotmpdir(self, client, revision, base):
        tmpdir = tempfile.mkdtemp()
        client.checkout(client.REPO + base, tmpdir, recurse=False, revision=revision)
        return tmpdir

    @expose("json")
    @srusers.require(srusers.in_team())
    def polldata(self, team, files = "",logrev=None):
        """Returns poll data:
            inputs: files - comma seperated list of files the client needs info
            on
            returns (json): A dictionary with an entry for each file (path is
            the key). Each value is a dictionary with information. The only key
            is revision, with a value of an integer of the current revision
            number in the repo"""
        #Default data
        r = {}
        l = {}
        client = Client(int(team))

        if files != "":
            files = files.split(",")
            rev = 0
            for file in files:
                r[file] = {}
                try:
                    info = client.info2( client.REPO + file )[0][1]
                    r[file]["rev"] = info["last_changed_rev"].number
                except pysvn.ClientError:
                    pass

        if logrev != None:
            try:
                newlogs = client.log(client.REPO, discover_changed_paths=True,
                    revision_end=pysvn.Revision(pysvn.opt_revision_kind.number,
                        int(logrev)+1))

                l =[{"author":x["author"], \
                        "date":time.strftime("%H:%M:%S %d/%m/%Y", \
                        time.localtime(x["date"])), \
                        "message":x["message"], "rev":x["revision"].number,
                        "changed_paths":[(c.action, c.path) for c in \
                            x.changed_paths]} for x in newlogs]
            except pysvn.ClientError:
                #No commits recently, no data to return
                pass

        return dict(files=r, log=l)
    
    @expose("json")
    @srusers.require(srusers.in_team())
    def delete(self, team, files):
        """
        Delete files from the repository, and prune empty directories.
        inputs: files - comma seperated list of paths
        returns (json): Message - a message to show the user
        """
        if files != "":
            files = files.split(",")
            client = Client(int(team))

            #This is called to get a log message for the deletion
            def cb():
                return True, "Files deleted"
            client.callback_get_log_message = cb

            urls = [client.REPO + str(x) for x in files]
            
            message = "Files deleted successfully: \n" + "\n".join(files)

            paths = list(set([os.path.dirname(file) for file in files]))

            try:
                client.remove(urls)
                #TODO: Need to prune empty directories. Get data from filelist
                #and then build a list of empty directories.
                for dir in paths:
                    if len(client.ls(client.REPO + dir)) == 0:
                        #The directory is empty, OK to delete it
                        log.debug("Deleting empty directory: " + client.REPO + dir)
                        client.remove(client.REPO + dir)
                        message += "\nRemove empty directory " + dir
                    
            except pysvn.ClientError:
                message = "Error deleting files."

            return dict(Message = message)
    
    @expose("json")
    @srusers.require(srusers.in_team())
    def fulllog(self, team):
        """Get a full log of file changes
            inputs: None
            returns (JSON): log - a list of dictionaries of:
                date, message, author, changed_paths, revision
                    changed_paths is a list of dicts:
                        action, path
        """
        client = Client(int(team))
        log = client.log(client.REPO, discover_changed_paths=True)

        return dict(log=[{"author":x["author"], \
                      "date":time.strftime("%H:%M:%S %d/%m/%Y", \
                      time.localtime(x["date"])), \
                      "message":x["message"], "rev":x["revision"].number,
                      "changed_paths":[(c.action, c.path) for c in \
                          x.changed_paths]} for x in log])

    @expose("json")
    @srusers.require(srusers.in_team())
    def savefile(self, team, file, rev, message, code):
        """Write a commit of one file.
        1. SVN checkout the file's directory
        2. Dump in the new file data
        3. Commit that directory with the new data and the message
        4. Wipe the directory

        TODO: Usernames.
        """
        client = Client(int(team))
        reload = "false"
        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness
        path = os.path.dirname(file)
        basename = os.path.basename(file)
        rev = self.get_revision("HEAD") #Always check in over the head to get
        #old revisions to merge over new ones

        if not client.is_url(client.REPO + path): #new dir needed...
            reload = "true"
            try:
                self.create_svn_dir(client, path)
            except pysvn.ClientError:
                return dict(new_revision="0", code = "",\
			                success="Error creating new directory",
                            reloadfiles="false")

        try:
            tmpdir = self.checkoutintotmpdir(client, rev, path)
        except pysvn.ClientError:
            try:
                shutil.rmtree(tmpdir)
            except:
                pass

            return dict(new_revision="0", code="", success="Invalid filename",
                        reloadfiles="false")

        #2. Dump in the new file data
        target = open(join(tmpdir, basename), "wt")
        target.write(code)
        target.close()

        #2 1/2: use client.add if we're adding a new file, ready for checkin
        try:
            client.add([join(tmpdir, basename)],
                       recurse=False)
            reload = "true"
        except pysvn.ClientError:
            pass

        #3. Commit the new directory
        try:
            newrev = client.checkin([tmpdir], message)
            if newrev == None:
                raise pysvn.ClientError
            newrev = newrev.number
            success = "True"
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
                    success=success, file=file, reloadfiles=reload)

    def create_svn_dir(self, client, path, msg=""):
        """Creates an svn directory if one doesn't exist yet
        inputs:
            client - a pysvn client
            path - path to the directory to be created
        returns: None, may through a pysvn.ClientError
        """

        if not client.is_url(client.REPO + path):
            upperpath = os.path.dirname(path)
            #Recurse to ensure folder parents exist
            self.create_svn_dir(client, upperpath)

            client.mkdir(client.REPO + path, "New Directory: " + path + " Notes: " + msg)

    @expose("json")
    @srusers.require(srusers.in_team())
    def filelist(self, team, rootpath="/", rev=0):
        """
        Returns a directory tree of the current repository.
        inputs: None
        returns: A tree as a list of files/directory objects:
            { tree : [{path : filepath
                       kind : FOLDER or FILE
                       children : [list as above]
                       name : name of file}, ...]}
        """    
        client = Client(int(team))
        
        if int(rev) == 0:
            target_rev = pysvn.Revision( pysvn.opt_revision_kind.head )
        else:
            target_rev = pysvn.Revision( pysvn.opt_revision_kind.number, int(rev))    
            
        peg_rev = pysvn.Revision( pysvn.opt_revision_kind.head );    
        
        print("target revision: ")
        
        if rootpath == "":
            rootpath = "/"

        if rootpath[0] != "/":
            rootpath = "/%s" % rootpath

        #This returns a flat list of files
        #This is sorted, so a directory is defined before the files in it
        try:
            files = client.list(client.REPO + rootpath, revision=target_rev, recurse=True)
        except pysvn.ClientError, e:
            print str(e)
            return { "error" : "Error accessing repository" }
        
        #Start off with a directory to represent the root of the path
        tree = dict( name = os.path.basename(rootpath),
                     path = rootpath,
                     children={} )

        #Go through each file, creating appropriate directories and files
        #In a tree structure based around dictionaries
        for details in [x[0] for x in files]:
            filename = details["repos_path"]

            # For some reason, pysvn returns "//" on the front of the paths
            if filename[0:2] == "//":
                filename = filename[1:] 

            basename = os.path.basename(filename)  #/etc/bla - returns bla

            short_fname = filename[len(rootpath):]
            top = tree

            for path in [x for x in short_fname.split("/") if len(x) > 0]:
                #Go through each section of the path, trying to go down into
                #directories. If they don't exist, create them

                if not top["children"].has_key( path ):
                    if details["kind"] == pysvn.node_kind.file:
                        kind = "FILE"
                    else:
                        kind = "FOLDER"

                    top["children"][path] = dict( name = basename,
                                                  path = filename,
                                                  kind = kind,
                                                  rev = details["created_rev"].number,
                                                  children = {} )
                top = top["children"][path]

        def dicttolist(tree):
            """Recursively change a dict containing values into a list of those
            values, and the same again for the dict contained in the children
            value.
            inputs: A dictionary of dictionaries. Each sub-dictionary to have a
            children dictionary (or at least a children : None)
            returns: That data changed into lists
            """
            try:
                #TODO: Need to sort here?
                #try and pull out child nodes into a list
                tree["children"] = tree["children"].values()
            except AttributeError:
                return tree
            
            #For each child node, try to apply this function to them
            for i in range(0, len(tree["children"])):
                try:
                    tree["children"][i] = dicttolist(tree["children"][i])
                except AttributeError:
                    pass
            return tree
        
        tree = dicttolist(tree)["children"]
        return dict(tree=tree)

	#create a new directory

    @expose("json")
    @srusers.require(srusers.in_team())
    def newdir(self, team, path, msg):
        client = Client(int(team))

        if not client.is_url(client.REPO + path):
            try:
                self.create_svn_dir(client, path, msg)
            except pysvn.ClientError:
                return dict( success=0, newdir = path,\
			                feedback="Error creating new directory")

        return dict( success=1, newdir = path,\
	                feedback="Directory successfully created")

    @expose("json")
    @srusers.require(srusers.in_team())
    def projlist(self, team):
        """Returns a list of projects"""
        client = Client(int(team))

        dirs = client.list( client.REPO, recurse = False)
        projects = []

        for details in [x[0] for x in dirs]:
            name = os.path.basename( details["repos_path"] )
            if name != "" and details["kind"] == pysvn.node_kind.dir:
                projects.append(name)

        return dict( projects = projects )

    @expose("json")
    @srusers.require(srusers.in_team())
    def revert(self, team, file, torev, message):
        torev=int(torev)
        client = Client(int(team))

        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness
        path = os.path.dirname(file)
        basename = os.path.basename(file)
        rev = self.get_revision("HEAD") #Always check in over the head to get
        #old revisions to merge over new ones

        if not client.is_url(client.REPO + path): #requested revision of dir that doesn't exist
            return dict(new_revision="0", code = "",\
			                success="Error reverting file - file doesn't exist")

        try:
            tmpdir = self.checkoutintotmpdir(client, rev, path)
        except pysvn.ClientError:
            try:    
                #wipe temp directory
                shutil.rmtree(tmpdir)
            except:
                pass

            return dict(new_revision="0", code = "",\
			                success="Error reverting file - could check out tmp dir")
        #2. Do a merge
        #revision we want to go back to
        revertto = pysvn.Revision( pysvn.opt_revision_kind.number, torev)
        #current revision (Head)
        revertfrom = pysvn.Revision( pysvn.opt_revision_kind.head )
        try:
            client.merge(client.REPO+path, \
                revertfrom,\
                client.REPO+path,\
                revertto, \
                tmpdir)
        except pysvn.ClientError:
                #wipe temp directory           
                shutil.rmtree(tmpdir)
                print "ClientError, returning";
                return dict();   
        print "didn't throw merge exception"
        #2 1/2: use client.add if we're adding a new file, ready for checkin
        try:
            if os.path.isdir(file) :
                client.add(tmpdir, recurse=True)
            else:
                client.add(join(tmpdir, basename), recurse=False)
        except pysvn.ClientError:
            try:    
                #wipe temp directory
                shutil.rmtree(tmpdir)
            except:
                pass
            return dict(new_revision="0", code = "",\
			                success="Error reverting file(s) - Are you already at the current revision ")
			                
        #3. Commit the new directory
        try:
            newrev = client.checkin([tmpdir], message)
            if newrev == None:
                raise pysvn.ClientError
            newrev = newrev.number
            success = "True"
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
                newrev = 0
            elif os.path.isdir(join(tmpdir, basename)) == True:
                success = "Merge"
                #Grab the merged text.                
                mergedfile = open(join(tmpdir, basename), "rt")
                code = mergedfile.read()
                mergedfile.close()
                newrev = newrev.number
                return dict(new_revision=newrev, code = "",\
		                        success="Merge Issues")
        #4. Wipe the directory
        shutil.rmtree(tmpdir)

        return dict(new_revision=newrev, code = "",\
		                success="Success !!!")
		                
    @expose("json")
    @srusers.require(srusers.in_team())
    def calendar(self, mnth, yr, file, team):
        #returns data for calendar function

        month = int(mnth)+1
        year = int(yr)
        c = Client(int(team))
        try:
            log = c.log(c.REPO+file)
        except:
            logging.debug("Log failed for %s" % c.REPO+file)
            print "failed to retrieve log"
            return dict(path=file, history=[])
        
        #get a list of users based on log authors
        start = datetime.datetime(year, month, 1, 0, 0, 0)

        if(month >=12):
            end = datetime.datetime(year+1, 1, 1, 0, 0, 0) #watchout for rollover
        else:
            end = datetime.datetime(year, month+1, 1, 0, 0, 0)
        
        result = []
        
        print "i'm here"
                 
        for y in log:
            now = datetime.datetime(2000, 1, 1);    #create a dummy datetime
            now = now.fromtimestamp(y["date"]);
            if (start <= now < end):
                result.append(y)
                

        return dict(  path=file,\
                      history=[{"author":x["author"], \
                      "date":time.strftime("%Y/%m/%d/%H/%M/%S", \
                      time.localtime(x["date"])), \
                      "message":x["message"], "rev":x["revision"].number} \
                      for x in result])
                      
    @expose("json")
    @srusers.require(srusers.in_team())
    def move(self, team, src, dest, msg=""):
        #   the source and destination arguments may be directories or files
        #   directories rendered empty as a result of the move are automatically 'pruned'
        #   returns status = 0 on success
        client = Client(int(team))
        source = client.REPO +src
        destination = client.REPO + dest        
        
        #log message callback - needed by client.move
        def cb():
            return True, str(msg)
            
        client.callback_get_log_message = cb       
        
        #message what gets returned to the browser
        message=""
            
        if not client.is_url(os.path.dirname(source)):
            return dict(new_revision="0", status="1", message="Source file/folder doesn't exist: "+src)
            
        if not client.is_url(os.path.dirname(destination)):
            return dict(new_revision="0", status="1", message="Destination file/folder doesn't exist: "+dest)
            
        try:
            client.move(source, destination, force=True)
            #TODO: Need to prune empty directories. 
            print "not failed yet\n"
            if len(client.ls(os.path.dirname(source))) == 0:
                #The directory is empty, OK to delete it
                log.debug("Deleting empty directory: " + source)
                client.remove(os.path.dirname(source))
                message += "\nRemove empty directory " + src
            message +="\n successfully moved file"
                
        except pysvn.ClientError, e:
            message = "Error moving files. :: "+str(e)
            return dict(new_revision="0", status="0", message=message) 
                        
        return dict(new_revision="0", status="0", message=message)                              

    @expose("json")
    @srusers.require(srusers.in_team())
    def copy(self, team, src="", dest="", msg="SVN Copy", rev="0"):
        
        src_rev = int(rev)                
        
        client = Client(int(team))
        source = client.REPO +src
        destination = client.REPO + dest                 
        
        def cb():
            return True, str(msg)
            
        client.callback_get_log_message = cb 
        
        if src_rev == 0:
            source_rev = pysvn.Revision( pysvn.opt_revision_kind.head)
        else:
            source_rev = pysvn.Revision( pysvn.opt_revision_kind.number, src_rev)
            
        if not client.is_url(os.path.dirname(source)):
            return dict(new_revision="0", status="1", msg="No Source file/folder specified");
        if not client.is_url(os.path.dirname(destination)):
            return dict(new_revision="0", status="1", msg="No Destination file/folder specified");
            
        try:
            client.copy(source, destination, source_rev);

        except pysvn.ClientError, e: 
            return dict(new_revision = "0", status="1", message="Copy Failed: "+str(e))
        
        return dict(new_revision = "0", status="0", message="copy successful")

