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
from os.path import join
log = logging.getLogger("roboide.controllers")

REPO = "http://studentrobotics.org/svn/"
ZIPNAME = "robot.zip"

#This dictionary contains information on what zip files are
#ready to ship. This should be in a db or similiar so multiple
#processes can access it. It currently will fail with multiple
#CherryPy processes

zips = {}

static_client = None
def ProtectedClient():
    global static_client
    def get_login(realm, username, may_save):
        return True, "test", "testpass", False

    if static_client != None:
        return static_client
    
    a = pysvn.Client()
    a.callback_get_login = get_login

    static_client = a
    return a

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
            rev = pysvn.Revision(pysvn.opt_revision_kind.number, int(revision))
        except (pysvn.ClientError, ValueError):
            rev = pysvn.Revision(pysvn.opt_revision_kind.head)
        return rev

    @expose()
    def checkout(self, files):
        """
        This function grabs a set of files and makes a zip available.
        inputs:
            ROT13 etc...)
            files - a comma seperated list of files to do the method on
        returns (JSON):
            status - A string to display to the user on completion."""
        if files == "":
            return ""

        client = ProtectedClient()
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
        client = ProtectedClient()
        
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
        c = ProtectedClient()

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
        #This returns a revision number. Always 0. Great.
        rev = self.get_revision(revision)
        client.checkout(REPO + base, tmpdir, recurse=False, revision=rev)
        return tmpdir

    @expose("json")
    def polldata(self,files = ""):
        """Returns data that needs polling by the client"""
        #Default data
        r = {}

        if files != "":
                files = files.split(",")
                client = ProtectedClient()

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
        client = ProtectedClient()
        reload = "false"
        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness
        path = os.path.dirname(file)
        basename = os.path.basename(file)

        if not client.is_url(REPO + path): #new dir needed...
            reload = "true"
            if not self.create_svn_dir(client, path):
	            return dict(new_revision="0", code = "",\
			                success="Error creating new directory",
                            reloadfiles="false")
        try:
            tmpdir = self.checkoutintotmpdir(client, rev, path)
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
        #Creates an svn directory if one doesn't exist yet
        #returns false on client error, which should never happen
        upperpath = os.path.dirname(path)

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
        client = ProtectedClient()
        
        files = client.ls(REPO, recurse=True)

        class Node (object):
            def __init__(self, name, path, kind):
                self.name = name
                self.path = path
                if kind == pysvn.node_kind.file:
                    self.kind = "FILE"
                else:
                    self.kind = "FOLDER"
                self.children = {}
                
        head = dict(name="HEAD",path="",kind="FOLDER",children={})

        for details in files:
            filename = details["name"][len(REPO):] #Strip off the repo URL
            basename = os.path.basename(filename)
            top = head 
            for path in filename.split("/"):
                try: 
                    top = top["children"][path]
                except KeyError:
                    if details["kind"] == pysvn.node_kind.file:
                        kind = "FILE"
                    else:
                        kind = "FOLDER"

                    top["children"][path] = dict(name=basename,
                                              path=filename,
                                              kind=kind,
                                              children={})
                                                

        def dicttolist(tree):
            try:
                tree["children"] = tree["children"].values()
            except AttributeError:
                return tree
            
            for i in range(0, len(tree["children"])):
                try:
                    tree["children"][i] = dicttolist(tree["children"][i])
                except AttributeError:
                    pass
            return tree

        return dict(children=[dicttolist(head)]) 

    @expose(template="roboide.templates.files")
    def index(self):
        return dict()
