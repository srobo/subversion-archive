from turbogears import controllers, expose
import cherrypy
import logging
import pysvn
import time
import re
import tempfile, shutil
import os
import zipfile
import random
from Queue import Queue
from os.path import join
log = logging.getLogger("roboide.controllers")

REPO = "http://studentrobotics.org/svn/"
ZIPNAME = "robot.zip"
CLIENTS = 5  #How many pysvn clients to put into the pool for use by threads

static_clients = Queue() #This queue will contain unused pysvn clients
#Pop one off the queue to use it, add it to the queue when finished!


def GetClient():
    """
    Get a pysvn client.
    inputs: none
    returns: A pysvn client object setup for logging into the server
    """
    def get_login(realm, username, may_save):
        return True, "test", "testpass", False
    a = pysvn.Client()
    a.callback_get_login = get_login
    return a

#This runs at initialisation of the app
#Add CLIENTS worth of pysvn clients to the static clients queue
for i in range(0, CLIENTS):
    static_clients.put(GetClient())

class Client:
    """
    A wrapper around a pysvn client. This takes a client from the pool and
    wraps calls to its functions. It returns the client to the pool when it's
    done with it.
    """
    def __init__(self):
        """
        On initialisation try to get a client from the pool. Block this thread
        until a client is available.
        """
        self.client = static_clients.get(block = True, timeout = None)
    def __del__(self):
        """
        When the object falls out of scope (at the end of the request) this is
        fired. Put the client back in the pool for other threads to use.
        """
        static_clients.put(self.client)
    def __getattr__(self, name):
        """
        This special method is called when something isn't found in the class
        normally. It returns the named attribute of the pysvn client this class
        is wrapping.
        """
        return getattr(self.client, name)

class Root(controllers.RootController):

    def get_revision(self, revision):
        """
        Get a revision object.
        inputs:
            revision - revision number (convertable with int()). If can not be
            converted, returns HEAD revision
        returns:
            revision object for revision number"""

        try:
            if revision == 0:
                revision = "HEAD"
            rev = pysvn.Revision(pysvn.opt_revision_kind.number, int(revision))
        except (pysvn.ClientError, ValueError, TypeError):
            rev = pysvn.Revision(pysvn.opt_revision_kind.head)
        return rev

    @expose()
    def checkout(self, files):
        """
        This function grabs a set of files and makes a zip available. Should be
        linked to directly.
        inputs:
            files - a comma seperated list of files to do the method on
        returns:
            A zip file as a downloadable file with appropriate HTTP headers
            sent.
        """
        if files == "":
            return ""

        client = Client()
        files = files.split(",")
        rev = self.get_revision("HEAD")

        #Need to check out the folders that contain the files
        #Create a folder to dump everything in
        root = tempfile.mkdtemp()

        dirs = [""] #List of directories already created

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
            f.write(client.cat(REPO + file, rev))
            f.close()

        #Now should have a tree in root.
        #Create a zip file in a temporary directory
        zfile = tempfile.mktemp()
        zip = zipfile.ZipFile(zfile, "w")
        #Walk through the tree of files checked out and add them
        #to the zipfile
        for node, dirs, files in os.walk(root):
            for name in files:
                #If the file is in the root directory
                if node == root:
                    #Add it named just its name
                    zip.write(os.path.join(node, name), name)
                else:
                    #Add it with a suitable path
                    zip.write(os.path.join(node, name),
                          node[len(root)+1:]+"/"+name)
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
        #Get rid of the temporary zipfile
        os.unlink(zfile)
        #Return the data
        return zipdata

    @expose("json")
    def filesrc(self, file=None, revision="HEAD"):
        """
        Returns the contents of the file.
        Turns out the action parameter can be edit. Not sure how this is
        useful - won't it always be edit?
        TODO: Cope with revision other than head.
        """
        curtime = time.time()
        client = Client()
        
        #TODO: Need to security check here! No ../../ or /etc/passwd nautiness

        rev = self.get_revision(revision)

        if file != None and file != "" and client.is_url(REPO + file):
            #Load file from SVN
            mime = ""
            try:
                mime = client.propget("svn:mime-type", REPO+file, revision=rev).values()[0]
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
                        ver = client.log(REPO + file, limit=1, revision_start=rev)[0]["revision"]
                        code = client.cat(REPO + file, revision=rev)
                        ver2 = client.log(REPO + file, limit=1,
                                revision_start=rev)[0]["revision"]
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

        return dict(curtime=curtime, code=code, revision=revision, path=file,
                name=os.path.basename(file))

    #TODO: Create an action that uses client.log to return a JSON list of
    #previous file revisions for a mochikit drop down
    @expose("json")
    def gethistory(self, file):
        c = Client()

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

    def checkoutintotmpdir(self, client, revision, base):
        tmpdir = tempfile.mkdtemp()
        rev = self.get_revision(revision)
        print REPO + base
        print rev
        client.checkout(REPO + base, tmpdir, recurse=False, revision=rev)
        return tmpdir

    @expose("json")
    def polldata(self,files = ""):
        """Returns data that needs polling by the client"""
        #Default data
        r = {}

        if files != "":
                files = files.split(",")
                client = Client()

                rev = 0
                for file in files:
                    r[file] = {}
                    try:
                        if file != None and client.is_url( REPO + file ):
                            info = client.info2( REPO + file )[0][1]
                            r[file]["rev"] = info["last_changed_rev"].number
                    except pysvn.ClientError:
                        print "Could not get information for %s" % file

        return r

    @expose("json")
    def savefile(self, file, rev, message, code):
        """Write a commit of one file.
        1. SVN checkout the file's directory
        2. Dump in the new file data
        3. Commit that directory with the new data and the message
        4. Wipe the directory

        TODO: Usernames.
        """
        client = Client()
        reload = "false"
        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness
        path = os.path.dirname(file)
        basename = os.path.basename(file)
        rev = self.get_revision(rev)

        if not client.is_url(REPO + path): #new dir needed...
            reload = "true"
            if not self.create_svn_dir(client, path):
	            return dict(new_revision="0", code = "",\
			                success="Error creating new directory",
                            reloadfiles="false")
        try:
            tmpdir = self.checkoutintotmpdir(client, rev, path + "/")
        except pysvn.ClientError:
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

    def create_svn_dir(self, client, path):
        """Creates an svn directory if one doesn't exist yet
        returns false on client error, which should never happen.
        inputs:
            client - a pysvn client
            path - path to the directory to be created
        returns:
            true if the path is created
        TODO: This looks like it's a c function. Would be more pythonic to let
        errors happen and cascade up, presuming success.
        """
        upperpath = os.path.dirname(path)
        print "X"
        print upperpath
        print "X"

        if not client.is_url(REPO + upperpath):
            if not self.create_svn_dir(client, upperpath): #recursion, yeah!
                return false #forward error
            try:
                client.mkdir(REPO + path, "new dir " + upperpath)
            except pysvn.ClientError:
                return false
        else:
            return true

    @expose("json")
    def filelist(self):
        """
        Returns a directory tree of the current repository.
        inputs: None
        returns: A tree as a list of files/directory objects:
            {children : [{path : filepath
                          kind : FOLDER or FILE
                          children : [list as above]
                          name : name of file}, ...]}
        """    

        client = Client()
        
        #This returns a flat list of files
        #This is sorted, so a directory is defined before the files in it
        files = client.ls(REPO, recurse=True)
        
        #Start off with a directory to represent the root of the path
        head = dict(name="HEAD",path="",kind="FOLDER",children={})

        #Go through each file, creating appropriate directories and files
        #In a tree structure based around dictionaries
        for details in files:
            filename = details["name"][len(REPO):] #Strip off the repo URL
            basename = os.path.basename(filename)  #/etc/bla - returns bla
            top = head  #for each file recurse from the head. TODO: slow?
            for path in filename.split("/"):
                #Go through each section of the path, trying to go down into
                #directories. If they don't exist, create them
                try:
                    top = top["children"][path]
                except KeyError:
                    #This happens if the node doesn't exist. If so, create it
                    if details["kind"] == pysvn.node_kind.file:
                        kind = "FILE"
                    else:
                        kind = "FOLDER"

                    top["children"][path] = dict(name=basename,
                                              path=filename,
                                              kind=kind,
                                              children={})
                                                

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
        
        return dict(children=[dicttolist(head)])

    @expose(template="roboide.templates.files")
    def index(self):
        """
        TODO: Remove this, make index served statically.
        """
        client = Client()
        info = client.info(os.getcwd())
        return dict(rev="RoboIDE revision: " + str(info["revision"].number))
