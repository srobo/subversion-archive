from turbogears import controllers, expose, config
from turbogears.feed import FeedController
import cherrypy, model
from sqlobject import sqlbuilder
import logging
import bzrlib.branch, bzrlib.repository, bzrlib.workingtree, bzrlib.memorytree, bzrlib.errors
import pysvn    # TODO BZRPORT: remove once all pysvn code removed
import time, datetime
import re
import tempfile, shutil
import os, sys, os.path
import zipfile
import random
from os.path import join
from cherrypy.lib.cptools import serveFile
import subprocess
import sr
import autosave as srautosave
import user as srusers
import fw
import string

log = logging.getLogger("roboide.controllers")

ZIPNAME = "robot.zip"

class Branch:
    """
    A wrapper around a bzr branch. This creates a branch object associated with a bzr branch and wraps calls to
    its functions.
    """
    client = None
    def __init__(self, team, project):
        """
        Create a bzr branch object and associate it with a user
        """

        repoloc = srusers.get_svnrepo( team ) # TODO BZRPORT: do we want Repo to be a string?

        branchloc = repoloc + "/" + project

        b = bzrlib.branch.Branch.open(branchloc)

        #Using self.__dict__[] to avoid calling setattr in recursive death
        self.__dict__["branch"] = b

    def __setattr__(self, name, val):
        """
        This special method is called when setting a value that isn't found
        elsewhere. It sets the same named value of the bzrlib class.
        """
        #BE CAREFUL - assigning class-scope variables anywhere in this class
        #causes instant setattr recursion death
        setattr(self.branch, name, val)

    def __getattr__(self, name):
        """
        This special method is called when something isn't found in the class
        normally. It returns the named attribute of the bzrlib class this class
        is wrapping.
        """
        return getattr(self.branch, name)

class Repo:
    """
    A wrapper around a bzr repository.
    """
    def __init__(self, team):

        self.__dict__["REPOLOC"] = srusers.get_svnrepo( team )

        r = bzrlib.repository.Repository.open(self.__dict__["REPOLOC"])

        #Using self.__dict__[] to avoid calling setattr in recursive death
        self.__dict__["repo"] = r

    def __setattr__(self, name, val):
        """
        This special method is called when setting a value that isn't found
        elsewhere. It sets the same named value of the bzrlib class.
        """
        #BE CAREFUL - assigning class-scope variables anywhere in this class
        #causes instant setattr recursion death
        setattr(self.repo, name, val)

    def __getattr__(self, name):
        """
        This special method is called when something isn't found in the class
        normally. It returns the named attribute of the bzrlib class this class
        is wrapping.
        """
        return getattr(self.repo, name)

class WorkingTree:
    """
    A wrapper around the WorkingTree class that checks out a working copy into a temp directory.
    """
    def __init__(self, team, project):

        # First open the branch
        repo = srusers.get_svnrepo( team ) # TODO BZRPORT: do we want Repo to be a string?
        branchloc = repo + "/" + project
        b = bzrlib.branch.Branch.open(branchloc)

        # Create a temporary directory
        tmpdir = tempfile.mkdtemp()

        # Lightweight checkout into the temp directory
        b.create_checkout(tmpdir, lightweight=True)

        # Open checkout as working tree object
        wt = bzrlib.workingtree.WorkingTree.open(tmpdir)

        #Using self.__dict__[] to avoid calling setattr in recursive death
        self.__dict__["workingtree"] = wt
        self.__dict__["tmpdir"] = tmpdir

    def __setattr__(self, name, val):
        """
        This special method is called when setting a value that isn't found
        elsewhere. It sets the same named value of the bzrlib class.
        """
        #BE CAREFUL - assigning class-scope variables anywhere in this class
        #causes instant setattr recursion death
        setattr(self.workingtree, name, val)

    def __getattr__(self, name):
        """
        This special method is called when something isn't found in the class
        normally. It returns the named attribute of the bzrlib class this class
        is wrapping.
        """
        return getattr(self.workingtree, name)

    def destroy(self):
        """
        Delete temporary directory.
        """
        shutil.rmtree(self.tmpdir)

class MemoryTree:
    """
    A wrapper around the MemoryTree class.
    """
    def __init__(self, team, project, revid=None):

        # First open the branch
        repo = srusers.get_svnrepo( team ) # TODO BZRPORT: do we want Repo to be a string?
        branchloc = repo + "/" + project
        b = bzrlib.branch.Branch.open(branchloc)

        if revid==None:
            # If no revid was specified, use latest
            revid = b.last_revision()

        mt = bzrlib.memorytree.MemoryTree(b, revid)

        #Using self.__dict__[] to avoid calling setattr in recursive death
        self.__dict__["memorytree"] = mt

    def __setattr__(self, name, val):
        """
        This special method is called when setting a value that isn't found
        elsewhere. It sets the same named value of the bzrlib class.
        """
        #BE CAREFUL - assigning class-scope variables anywhere in this class
        #causes instant setattr recursion death
        setattr(self.memorytree, name, val)

    def __getattr__(self, name):
        """
        This special method is called when something isn't found in the class
        normally. It returns the named attribute of the bzrlib class this class
        is wrapping.
        """
        return getattr(self.memorytree, name)

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
    fw = fw.FwServe()
    autosave = srautosave.Autosave()
    #feed = Feed()

    if config.get("simulator.enabled"):    # if simulation is enabled import the simulator controller
        import sim
        sim = sim.Sim()

    @expose()
    def index(self):
        """
        In case the apache rewrite rule isn't paying attention, serve up the
        index file from here.
        """
        loc = os.path.join(os.path.dirname(__file__), "static/index.html")
        return serveFile(loc)

    def get_rev_object(self, team, rev_id=""):
        """
        Get a revision object for a given bzr revision id.
        If no revision id is provided, get latest revision.
        inputs:
            rev_id - str in bzr rev_id format, ie returned by get_rev_id()
        returns:
            revision object for given revision id."""

        b = Branch( int(team) )

        if rev_id == "":
            rev_id = b.last_revision()

        try:
            rev = b.repository.get_revision(rev_id)
        except: #TODO BZRPORT: implement exception properly, revision number failure etc
            print "Get revision failed, returned latest revision object"
            rev = b.repository.get_revision( b.last_revision() )

        return rev

    def get_rev_id(self, team, project, revno=-1):
        """
        Get revision ID string from revision number.
        inputs:
            revno - revision number convertable with int().
                    if revno is -1 or not supplied, get latest revision id.
        returns:
            revision id string
        """

        b = Branch( int(team), project )

        try:
            if revno == -1 or revno == "-1" or revno == "HEAD": #TODO BZRPORT: stop anything calling "HEAD" string
                rev_id = b.last_revision()
            else:
                rev_id = b.get_rev_id( int(revno) )

        except (TypeError):     # TODO BZRPORT: add bzr exception
            print "Getting ID for revno: %s failed, returning latest revision id."  % revno
            rev_id = b.last_revision()

        return rev_id


    @expose()
    @srusers.require(srusers.in_team())
    def checkout(self, team, project, simulator=False):
        """
        This function grabs a set of files and makes a zip available. Should be
        linked to directly.
        inputs:
            files - a comma seperated list of files to do the method on
        returns:
            A zip file as a downloadable file with appropriate HTTP headers
            sent.
        """
        pass    #TODO BZRPORT: Implement!

        client = Client(int(team))
        rev = self.get_revision("HEAD")

        # Directory to work in
        root = tempfile.mkdtemp()

        # Checkout the code
        client.export(client.REPO + "/%s" % project,
                      root + "/code",
                      revision=rev,
                      recurse=True)

        # Check if __init__.py exists in user code, if it doesn't insert blank file before checkout
        if not os.path.exists(root+"/code/__init__.py"):
            f = open(root+"/code/__init__.py", 'w')
            f.close()


        # (internal) robot.zip to contain the code
        zfile = tempfile.mktemp()
        zip = zipfile.ZipFile(zfile, "w")

        def add_dir_to_zip(head, zip):
            #Walk through the tree of files checked out and add them
            #to the zipfile
            for node, dirs, files in os.walk(head):
                for name in files:
                    #Skip .svn directories
                    if ".svn" in node:
                        continue
                    #If the file is in the root directory
                    src = os.path.join(node, name)
                    if node == head:
                        #Add it named just its name
                        zip.write(src, name, compress_type = zipfile.ZIP_DEFLATED)
                    else:
                        #Add it with a suitable path
                        archname = node[len(head):]+'/'+name
                        zip.write(src, archname, compress_type = zipfile.ZIP_DEFLATED)

        add_dir_to_zip(root + "/code", zip)
        zip.close()

        # Remove the temporary dir
        shutil.rmtree(root)

        if not simulator:
            """The robot expects a zipfile containing libraries, with another zipfile inside."""
            #Create a second zip file placing the user zip in the system zip
            syszfile = tempfile.mktemp()
            syszip = zipfile.ZipFile(syszfile, "w")
            add_dir_to_zip(config.get("svn.packagedir"), syszip)

            syszip.write(zfile, ZIPNAME)
            syszip.close()

            #Read the data in from the temporary zipfile
            zipdata = open(syszfile, "rb").read()
            os.unlink(syszfile)
        else:
            zipdata = open(zfile, "rb").read()

        os.unlink(zfile)

        #Set up headers for correctly serving a zipfile
        cherrypy.response.headers['Content-Type'] = \
                "application/x-download"
        cherrypy.response.headers['Content-Disposition'] = \
                'attachment; filename="' + ZIPNAME + '"'

        #Return the data
        return zipdata

    @expose("json")
    @srusers.require(srusers.in_team())
    def filesrc(self, team, project, file=None, revision=-1):
        """
        Returns the contents of the file.
        Turns out the action parameter can be edit. Not sure how this is
        useful - won't it always be edit?
        """

        curtime = time.time()
        b = Branch( int(team), project )

        #TODO: Need to security check here! No ../../ or /etc/passwd nautiness trac#208

        autosaved_code = self.autosave.getfilesrc(team, file, 1)

#        rev = self.get_revision(revision) #TODO BZRPORT

        if file != None and file != "":  #TODO BZRPORT: URL checking
            #Load file from bzr
            # TODO BZRPORT: mime checking. Bzr doesn't have a mime property so the file will need to be checked with python
            try:
#                revno, rev_id = b.last_revision_info()  #TODO BZRPORT: getting latest revision info
                branch_tree = b.basis_tree()    #TODO BZRPORT: This is taking most recent tree. Allow other revisions!
                file_id = branch_tree.path2id(file)
                b.lock_read()
                code = branch_tree.get_file_text(file_id)
                revision = 0 #TODO BZRPORT: actual revsion
            except: #TODO BZRPORT: Catch bzr exception
                code = "Error loading file '%s' at revision %s." % (file, revision)
                revision = 0
            # always unlock:
            finally:
                b.unlock()

        else:
            code = "Error loading file: No filename was supplied by the IDE.  Contact an SR admin!"
            revision = 0

        return dict(curtime=curtime, code=code, autosaved_code=autosaved_code, revision=revision, path=file,
                name=os.path.basename(file))


    @expose("json")
    @srusers.require(srusers.in_team())
    def gethistory(self, team, file, user = None, offset = 0):
        #This function retrieves the svn log output for the given file(s)
        #to restrict logs to particular user, supply a user parameter
        #a maximum of 10 results are sent to the browser, if there are more than 10
        #results available, overflow > 0.
        #supply an offset to view older results: 0<offset < overflow; offset = 0 is the most recent logs
        pass    #TODO BZRPORT: Implement!

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

#    def checkoutintotmpdir(self, branch, revision, base): # TODO BZRDIR: replace with WorkingTree object
#
#        tmpdir = tempfile.mkdtemp()
#        client.checkout(client.REPO + base, tmpdir, recurse=False, revision=revision)
#        branch.create_checkout(tmpdir, lightweight=True)
#        return tmpdir

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
        pass    #TODO BZRPORT: Implement!

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
    def delete(self, team, files, kind = 'SVN'):
        """
        Delete files from the repository, and prune empty directories.
        inputs: files - comma seperated list of paths
                kind - one of 'SVN' or 'AUTOSAVES'
        returns (json): Message - a message to show the user
        """
        pass    #TODO BZRPORT: Implement!

        if files != "":
            files = files.split(",")
            client = Client(int(team))

            #This is called to get a log message for the deletion
            def cb():
                return True, "Files deleted: "+', '.join(files)
            client.callback_get_log_message = cb

            urls = [client.REPO + str(x) for x in files]

            message = "Files deleted successfully: \n" + "\n".join(files)

            for f in files:
                self.autosave.delete(team, f)

            if kind == 'AUTOSAVES':
                return dict(Message = "AutoSaves deleted successfully: \n" + "\n".join(files))

            paths = list(set([os.path.dirname(file) for file in files]))

            try:
                client.remove(urls)
                #Prune empty directories. Get data from filelist and then build a list of empty directories.
                for dir in paths:
                    if len(client.ls(client.REPO + dir)) == 0:
                        #The directory is empty, OK to delete it

                        #jmorse - don't prune project dirs, this offends gui
                        if dir.encode("iso-8859-1").find('/', 1) == -1:
                            continue

                        log.debug("Deleting empty directory: " + client.REPO + dir)
                        client.remove(client.REPO + dir)
                        message += "\nRemove empty directory " + dir

            except pysvn.ClientError:
                message = "Error deleting files."

            return dict(Message = message)

    @expose("json")
    @srusers.require(srusers.in_team())
    def undelete(self, team, files, rev='0'):
        """
        UnDelete files from the repository - basically grabs a list of them,
        then uses copy to re-instate them, one by one
        TODO: do all files in one go trac#335
        inputs: files - comma seperated list of paths
                rev - the revision to undelete from
        returns (json): Message - a message to show the user
        """
        pass    #TODO BZRPORT: Implement!

        if files != "":
            files = files.split(",")
            client = Client(int(team))
            fail = {}
            success = []
            status = 0

            for f in files:
                result = self.cp(team, f, f, 'Undelete file '+f, rev)
                print result
                if int(result['status']) > 0:
                    fail[f] = result['message']
                    status = status + 1
                else:
                    success.append(f)

            return dict(fail = fail, success = ','.join(success), status = status)

        else:
            return dict(Message = 'Undeleted failed - no files specified', status = 1)

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
        pass    #TODO BZRPORT: Implement!

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
    def savefile(self, team, project, file, rev, message, code): # TODO: why is rev specified here?
        """Write a commit of one file.
        1. SVN checkout the file's directory
        2. Dump in the new file data
        3. Commit that directory with the new data and the message
        4. Wipe the directory
        """

        #1. Get working tree of branch in temp dir
        wt = WorkingTree(int(team), project)
        reload = "false"

        #TODO: Check for path naugtiness trac#208
        path = os.path.dirname(file)
        basename = os.path.basename(file)
        fullpath = wt.tmpdir + "/" + file

        if wt.path2id(path) is None: # directory doesn't exist, new dir needed...
            reload = "true"
            try:
                self.create_dir(client, path)
            except pysvn.ClientError: # TODO BZRPORT: bzr error needed
                return dict(new_revision="0", code = "",\
                            success="Error creating new directory",
                            reloadfiles="false")

#        try:
#            tmpdir = self.checkoutintotmpdir(client, rev, path)
#        except pysvn.ClientError:
#            try:
#                shutil.rmtree(tmpdir)
#            except:
#                pass
#            return dict(new_revision="0", code="", success="Invalid filename",
#                        reloadfiles="false")

        existing_file = wt.path2id(file) # if it is a new file this will return None

        #2. Dump in the new file data
        target = open(fullpath, "wt")
        target.write(code)
        target.close()

        # If file doesn't exist it needs to be added
        if existing_file is None:
            wt.add(file)
            reload = "true"

        #3. Commit the new directory
        try:
            newrev = wt.commit(message)
            success = "True"
        except bzrlib.errors.OutOfDateTree:
            pass # TODO BZRPORT: this needs discussion. Will this ever happen?
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

        #4. Destroy working tree checkout, remove the autosaves
        wt.destroy()
        self.autosave.delete(team, file)

        return dict(new_revision=str(newrev), code=code,
                    success=success, file=file, reloadfiles=reload)

    def create_dir(self, memtree, path, msg=""): # TODO BZRPORT: error checking
        """Creates an svn directory if one doesn't exist yet
        inputs:
            memtree - a memorytree object
            path - path to the directory to be created, relative to tree root
        returns: True if directory created, false if it already existed
        """

        # check if path already exists - if it doesn't path2id will return None
        # if branch has no revisions yet revision_history will return an empty list
        if memtree.path2id(path) is None or len(memtree.branch.revision_history()) == 0:
            upperpath = os.path.dirname(path)
            #Recurse to ensure folder parents exist
            if not upperpath == "/":
                self.create_dir(memtree, upperpath)

            memtree.lock_write() # lock tree for modification
            try:
                # first perform special dance in the case that branch has no commits yet
                if len(memtree.branch.revision_history()) == 0:
                    memtree.add("") # special dance

                # make directory (added automatically)
                memtree.mkdir(path)

                # commit
                memtree.commit("New directory " + path + " created. Notes: " + msg)

            finally: # always unlock tree
                memtree.unlock()
            return True
        else:
            return False # directory already existed
            

    @expose("json")
    @srusers.require(srusers.in_team())
    def filelist(self, team, project, rootpath="/", rev=-1, date=0):
        """
        Returns a directory tree of the current repository.
        inputs: project - the bzr branch
                rootpath - to return file from a particular directory within the branch (recursive)
        returns: A tree as a list of files/directory objects:
            { tree : [{path : filepath
                       kind : FOLDER or FILE
                       children : [list as above]
                       name : name of file}, ...]}
        """

        b = Branch( int(team), project )

        target_rev_id = self.get_rev_id(team, project ,rev)
        self.user.set_setting('project.last', project)

        try:
            rev_tree = b.repository.revision_tree(target_rev_id)
        except:
            return { "error" : "Error getting revision tree" }

        # Get id of root folder from which to list files. if it is not found it will return None
        rootid = rev_tree.path2id(rootpath)

        try:
            rev_tree.lock_read()
            # Get generator object containing file information from base rootid. If rootid=None, will return from root.
            files = rev_tree.inventory.iter_entries(rootid)
        except: # TODO BZRPORT: Proper error handling
            return { "error" : "Error getting file list" }
        # Always unlock tree:
        finally:
            rev_tree.unlock()

        autosave_data = self.autosave.getfilesrc(team, project)

        def branch_recurse(path, entry, files, given_parent_id):
            """
            Travels recursively through a generator object provided by revision_tree.inventory.iter_items.
            Iter_items returns child items immediately after their parents, so by checking the parent_id field of the item with the actual id of the directory item that called it, we can check if we are still within that directory and therefore need to add the item as a child.
            This function will return a list of all children of a particular branch, along with the next items for analysis.
            Whenever it encounters a directory it will call itself to find the children.
            inputs: path - path of item to be analysed first
                    entry - InventoryEntry-derived object of item to be analysed first
                    files - generator object created by iter_items
                    given_parent_id - id (string) of calling directory
            returns: entry_list - list of children. if given_parent_id does not match entry.parent_id, this will be an empty list.
                     path - path of item that has not yet been added to the tree
                     entry - the entry object that has not yet been added to the tree.
                             if given_parent_id did not match entry.parent_id, then path and entry returned will be the same as path and entry called.
            """

            entry_list = []

            while entry.parent_id == given_parent_id: # is a child of parent

                if entry.kind == "directory":
                    try:
                        next_path, next_entry = files.next()
                        children_list, next_path, next_entry = branch_recurse(next_path, next_entry, files, entry.file_id)
                    except StopIteration: # No more files to iterate through after this one
                        next_entry = None # break after adding this entry
                        children_list = [] # no more items, so there can't be any children

                    entry_list.append({
                                        "name": entry.name,
                                        "path": path,
                                        "kind": "FOLDER",
                                        "autosave": 0,  # No autosave data for directories
                                        "rev": "-1", #TODO BZRPORT: what's this show/for? yes, i know revision, i mean, current, or when it was created?
                                        "children": children_list})

                    if next_entry is None:
                        break # there are no more iterations so break
                    else:
                        path = next_path
                        entry = next_entry  # now we'll use the returned entry


                else:
                    if path in autosave_data:
                        autosave_info = autosave_data[path]
                    else:
                        autosave_info = 0
                    entry_list.append({
                                        "name": entry.name,
                                        "path": path,
                                        "kind": "FILE",
                                        "autosave": autosave_info,
                                        "rev": "-1", #TODO BZRPORT: what's this show/for? yes, i know revision, i mean, current, or when it was created?
                                        "children": []})

                    try:
                        path, entry = files.next()  # grab next entry
                    except StopIteration: # No more files to iterate through
                        break

            return entry_list, path, entry

        # Determine tree_root string to pass to recursing function as a parent id
        if rootid == None:
            tree_root = "TREE_ROOT"
        else:
            tree_root = rootid

        try:
            first_path, first_entry = files.next()  # grab next entry
        except StopIteration:   # StopIteration caught on first pass: project tree must be empty
            return dict(tree = [])

        tree, last_path, last_entry = branch_recurse(first_path, first_entry, files, tree_root)

        return dict(tree = tree)

    #create a new directory
    @expose("json")
    @srusers.require(srusers.in_team())
    def newdir(self, team, project, path, msg):

        memtree = MemoryTree(int(team), project)

        try:
            created = self.create_dir(memtree, path, msg)
        except pysvn.ClientError: # TODO BZRPORT: replace with bzr error
            return dict( success=0, newdir = path,\
                        feedback="Error creating directory: " + path)

        if created: # directory was created
            return dict( success=1, newdir = path,\
                    feedback="Directory successfully created")
        else: # directory wasn't created because it already existed
            return dict( success=0, newdir = path,\
                    feedback="Directory " + path + " already exists")

    @expose("json")
    @srusers.require(srusers.in_team())
    def projlist(self, team):
        """Returns a list of projects"""

        try:
            r = Repo( int(team) )
        except:
            #No repository present
            return dict(projects = [])

        self.user.set_setting('team.last', team)

        projects = []

        branches = r.find_branches()

        for branch in branches:
            projects.append(branch.nick)

        return dict( projects = projects )

    @expose("json")
    @srusers.require(srusers.in_team())
    def createproj(self, name, team):
        """Creates new project directory"""

        r = Repo(int(team))

        if name.find(".") != -1:
            """No ../../ nastyness"""
            return nil

        url = r.REPOLOC + "/" + name

        r.bzrdir.create_branch_convenience(base=url,force_new_tree=False)

        return dict( )

    @expose("json")
    @srusers.require(srusers.in_team())
    def revert(self, team, file, torev, message):
        pass    #TODO BZRPORT: Implement!

        torev=int(torev)
        client = Client(int(team))

        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness trac#208
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
        pass    #TODO BZRPORT: Implement!

        month = int(mnth)+1
        year = int(yr)
        c = Client(int(team))
        try:
            log = c.log(c.REPO+file)
        except:
            logging.debug("Log failed for %s" % c.REPO+file)
            print "failed to retrieve log"
            return dict(path=file, history=[])

        if log[0]['revision'].number == 0: #if there's nothing there
            return dict(path=file, history=[])

        #get a list of users based on log authors
        start = datetime.datetime(year, month, 1, 0, 0, 0)

        if(month >=12):
            end = datetime.datetime(year+1, 1, 1, 0, 0, 0) #watchout for rollover
        else:
            end = datetime.datetime(year, month+1, 1, 0, 0, 0)

        result = []

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
        pass    #TODO BZRPORT: Implement!

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
            #Prune empty directories.
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

        self.autosave.move(team, src, dest)

        return dict(new_revision="0", status="0", message=message)

    @expose("json")
    @srusers.require(srusers.in_team())
    def copy(self, team, src="", dest="", msg="SVN Copy", rev="0"):
        return self.cp(team, src, dest, msg, rev)

    @srusers.require(srusers.in_team())
    def cp(self, team, src="", dest="", msg="SVN Copy", rev="0"):
        pass    #TODO BZRPORT: Implement!

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

    @expose("json")
    @srusers.require(srusers.in_team())
    def checkcode(self, team, path, code=0, date=None):
        pass    #TODO BZRPORT: Implement!

        client = Client(int(team))
        rev = self.get_revision("HEAD")
        file_name = os.path.basename(path)
        path = os.path.dirname(path)

        # Directory to work in
        td = tempfile.mkdtemp()

        # Check out the code
        print "Checking out %s" % (client.REPO + path)
        client.export(client.REPO + path,
                      td + "/code",
                      revision=rev,
                      recurse=True)

        if code != 0: #overwrite the version from the svn
            print td+"/code/"+file_name
            tmpfile = open(td+"/code/"+file_name, 'w')
            tmpfile.write(str(code))
            tmpfile.close()

        print 'temp_dir: '+td+"\nfile_name: "+file_name

        # Check out the dummified SR library too
        shutil.copy( config.get("checker.file"), td + "/code" )

        # Run pychecker
        p = subprocess.Popen( ["pychecker", "-e", "Error", file_name ],
                              cwd = "%s/code" % td,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE )
        output = p.communicate()

        rval = p.wait()

        # Remove the temporary directory
        shutil.rmtree(td)

        if rval == 0:
            return dict( errors = 0 )
        else:
            chk_warnings = []
            chk_errors = []

            #pychecker outputs two parts: one warnings, one the processing resuls
            warn_part = output[0].split('\n')
            proc_part = output[1].split('\n')

            for line in warn_part: #simply grab the lines of interest - we can do more in JS
                if not line in ['', '\n', 'Warnings...']:
                    chk_warnings.append(line)

            for line in proc_part:
                if not line in ['', '\n', 'Processing '+os.path.splitext(file_name)[0]+'...']:
                    chk_errors.append(line)

            return dict( messages = chk_warnings, err = chk_errors, path = path, file = file_name, errors = 1 )

    @expose("json")
    def autocomplete(self, str, nocache):
        print str
        return "{}"

    @expose("json")
    @srusers.require(srusers.in_team())
    def robolog(self, team, last_received_ping=0):
        """
        Return the log being appended direct from the robot.
        offset is the position to stream from.
        """
        # First check to see if live robot logging is enabled on the server
        # If it isn't enabled then return "disabled" to tell client not to poll
        if not config.get("robolog.enabled"):
            return {"ping" : 0,
                    "data" : "",
                    "present" : 0,
                    "disabled" : True}
 
        try:
            team = int(team)
        except ValueError:
            log.error("Invalid team value")
            return {"ping" : 0,
                    "data" : "",
                    "present" : 0}

        try:
            last_received_ping = int(last_received_ping)
        except ValueError:
            log.error("Invalid last_received_ping")
            return {"ping" : 0,
                    "data" : "",
                    "present" : 0}

        log.debug("RoboIde team = %d", team)
        log.debug("RoboIde last_received_ping = %d", last_received_ping)

        if not (team in srusers.getteams()):
            log.error("Team not in the users teams")
            log.error(srusers.getteams())
            return {"ping" : 0,
                    "data" : "",
                    "present" : 0}
        
        try:
            team = model.TeamNames.get(id=team)
        except:
            #Fake team!
            log.debug("Team not found")
            return {"ping" : 0,
                    "data" : "",
                    "present" : 0}
 

        try:
            present = model.RoboPresent.selectBy(team=team)[0].present
        except:
            present = False

        most_recent_ping = 0
        most_recent_ping_date = None

        robologs = model.RoboLogs.selectBy(team=team)
        robologs = robologs.orderBy(sqlbuilder.DESC(model.RoboLogs.q.id))
        most_recent_ping = robologs[0].id

        log.debug("Robot presence is: %d" % present)

        last_received_ping = int(last_received_ping)
        logs = model.RoboLogs.select(sqlbuilder.AND(model.RoboLogs.q.team == team,
                                         model.RoboLogs.q.id > last_received_ping))
        
        data = "\n".join([l.value for l in logs])

        data = data.replace('&', '&amp;')
        data = data.replace('"', '&quot;')
        data = data.replace("'", '&#39;')
        data = data.replace(">", '&gt;')
        data = data.replace("<", '&lt;')

        return {"data" : data,
                "present" : int(present),
                "ping" : most_recent_ping}
