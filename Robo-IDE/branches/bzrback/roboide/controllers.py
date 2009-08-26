from turbogears import controllers, expose, config
from turbogears.feed import FeedController
import cherrypy, model
from sqlobject import sqlbuilder
import logging
import bzrlib.branch, bzrlib.repository, bzrlib.workingtree, bzrlib.memorytree, bzrlib.tree, bzrlib.errors, bzrlib.progress, bzrlib.merge, bzrlib.generate_ids
import pysvn    # TODO BZRPORT: remove once all pysvn code removed
import time, datetime, StringIO
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
import fw, switchboard
import string

log = logging.getLogger("roboide.controllers")

ZIPNAME = "robot.zip"

class ProjectWrite():
    """
    A class for making modifications to the project, ie those ending in a commit.
    """
    def __init__(self, team, project, revid=None):
        """
        Open a TranformPreview object for a revision of the project.
        """
        self.b = open_branch(team, project)

        if revid == None:
            # If no revid was specified, use latest
            revid = self.b.last_revision()

        self.rev_tree = self.b.repository.revision_tree(revid)

        self.TransPrev = bzrlib.transform.TransformPreview(self.rev_tree)
        self.PrevTree = self.TransPrev.get_preview_tree()

        self.revid = revid
        self.team = team
        self.project = project
        self.conflicts = []

    def _update_tree(self):
        """
        Update PreviewTree when new entries have been added.
        """
        self.PrevTree = self.TransPrev.get_preview_tree()

    def _new_dir(self, path):
        """
        Private method to create a directory and parents.
        Any methods making use of this must ensure PrevTree is updated afterwards
            so that it contains the new paths.
        inputs:
            path - path to the directory to be created, relative to tree root
        """
        path = path.strip("/") # remove leading and trailing /

        #dirs = os.path.dirname(path).split('/')
        dirs = path.split("/")

        parent_trans_id = self.TransPrev.root # start at root of tree

        for i in range(0, len(dirs)):
            # path = cheese,
                # then path = cheese/peas,
                    # then path = cheese/peas/bananas
            dir_path = os.path.sep.join(dirs[:i+1])
            dir_name = dirs[i]

            #info of the form ('file'/'directory'/'missing', size, exec, sha) (sha not working)
            info = self.PrevTree.path_content_summary(dir_path)

            if info[0] == 'file':
                #A file with the name of the requested directory already exists
                raise Exception # TODO: proper error
            elif info[0] == 'missing':
                dir_id = bzrlib.generate_ids.gen_file_id(dir_name)
                trans_id = self.TransPrev.new_directory(dir_name, parent_trans_id, dir_id)
            elif info[0] == 'directory':
                trans_id = self.TransPrev.trans_id_tree_path(dir_path)
            else:
                raise Exception # this should never happen!

            parent_trans_id = trans_id

        # return transaction id of topmost dir
        return parent_trans_id

    def merge(self):
        """
        Attempt to merge with latest revision of branch.
        """
        revid_latest = self.b.last_revision()

        merger = bzrlib.merge.Merger.from_revision_ids(
                        bzrlib.progress.DummyProgress(),
                        self.PrevTree,
                        revid_latest,
                        base = self.revid, # this is important!
                        other_branch = self.b,
                        tree_branch = self.b)

        merger.merge_type = bzrlib.merge.Merge3Merger
        tree_merger = merger.make_merger()
        tt2 = tree_merger.make_preview_transform()

        # update the object
        self.TransPrev = tt2

        self.revid = revid_latest
        self.conflicts = tree_merger.cooked_conflicts
        return

    def commit(self, message=""):
        """
        Commit changed tree.
        """
        if not self.revid == self.b.last_revision():
            return None # cannot commit, tree not up to date
        if not len(self.conflicts) == 0:
            return None # cannot commit, conflicts remain

        self.b = open_branch(self.team, self.project) # TODO: why do we have to do this? locate cause of write-only transaction error
        self.b.lock_write()

        try:
            if hasattr(self.PrevTree, "commit"):
                # As of bzr 1.18 PreviewTrees have built-in commit method.
                #self.PrevTree.set_parent_ids([ self.revid ]) # needed here?
                revid_new = self.PrevTree.commit(message)
            else:
                self.PrevTree.set_parent_ids([ self.revid ])
                revprops = {"branch-nick":self.b.nick} # is this necessary?
                builder = self.b.get_commit_builder([self.revid], revprops = revprops)

                changes = list(builder.record_iter_changes(
                                self.PrevTree, self.revid, self.TransPrev.iter_changes()))
                builder.finish_inventory()
                revid_new = builder.commit(message)
                revno_new = self.b.revision_id_to_revno(self.revid)
                self.b.set_last_revision_info(revno_new, revid_new)
        finally:
            # always unlock branch
            # NOTE: an exception during unlock() here can mask other exceptions during try.
            #   see ie http://bugs.launchpad.net/bzr/+bug/230902/comments/2
            # TODO: check for original exception
            self.b.unlock()
#            pass

        self.revid = revid_new
        return revid_new # should we delete TransPrev as it is no longer up to date?

    def new_directory(self, path):
        """
        Creates a file or directory, and any parent directories required.
        Will automatically update PrevTree.
        """
        trans_id = self._new_dir(path)

        # update preview tree
        self._update_tree()

        return self.PrevTree.path2id(path)


    def update_file_contents(self, path, contents, create=True):
        """
        Replace the contents of a file.
        inputs:
            path - path of file to be written
            create - when True if file doesn't exist it will be created, as well as parent directories.
        """
        parent_path = os.path.dirname(path)
        file_name = os.path.basename(path)

        file_id = self.PrevTree.path2id(path)

        if file_id == None:
            # file doesn't exist yet

            if create is not True:
                # don't create a new file
                raise Exception # TODO real exception

            parent_id = self.PrevTree.path2id(parent_path)

            if parent_id == None:
                parent_trans_id = self._new_dir(parent_path)
            elif parent_id == "TREE_ROOT":
                parent_trans_id = self.TransPrev.root
            else:
                parent_trans_id = self.TransPrev.trans_id_file_id(parent_id)

            file_id = bzrlib.generate_ids.gen_file_id(file_name)

            print "new generated file id: %s" % file_id # TODO temp, remove
            print "parent trans id: %s" % parent_trans_id

            self.TransPrev.new_file(file_name, parent_trans_id, contents, file_id)

            self._update_tree() # update PrevTree to reflect new file

        else:
            trans_id = self.TransPrev.trans_id_file_id(file_id)

            # delete existing contents
            self.TransPrev.delete_contents(trans_id)

            # add new contents
            self.TransPrev.create_file(contents, trans_id)

        return

    def destroy(self):
        """
        Clean up.
        """
        pass

def open_branch(team, project):
    """
    Open the project branch for the team.

    TODO: Check the logged in user has permission to do this!
    """
    repoloc = srusers.get_svnrepo( team )
    branchloc = repoloc + "/" + project
    return bzrlib.branch.Branch.open(branchloc)

def open_repo(team):
    """
    Open the team repository.

    TODO: Check the logged in user has permission to do this.
    """
    repoLoc = srusers.get_svnrepo( team )
    return bzrlib.repository.Repository.open(repoLoc)

def open_memory_tree(team, project, revid=None):
    """
    Open an in-memory tree for the project.
    """
    # First open the branch
    b = open_branch(team, project)

    if revid == None:
        # If no revid was specified, use latest
        revid = b.last_revision()

    return bzrlib.memorytree.MemoryTree(b, revid)


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
    switchboard = switchboard.Switchboard()

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

        b = open_branch( int(team) )

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

        b = open_branch( int(team), project )

        try:
            if revno == -1 or revno == "-1" or revno == "HEAD": #TODO BZRPORT: stop anything calling "HEAD" string
                rev_id = b.last_revision()
            else:
                rev_id = b.get_rev_id( int(revno) )

        except (TypeError):     # TODO BZRPORT: add bzr exception
            print "Getting ID for revno: %s failed, returning latest revision id."  % revno
            rev_id = b.last_revision()

        return rev_id

    def get_file_revision(self, tree, fileid):
        """
        Get the id of the revision when the file was last modified.
        inputs: tree - a bzrlib tree of some kind
                fileid - file id of file in tree
        outputs: revid - revision id
        """
        return bzrlib.tree.Tree._file_revision(tree, fileid) # static method for some reason

    @expose()
    @srusers.require(srusers.in_team())
    def checkout(self, team, project, simulator=False):
        """
        This function grabs a set of files and makes a zip available. Should be
        linked to directly.
        inputs:
            team & project - code to retrieve
            simulator - true if code is being delivered to a simulator.
        returns:
            A zip file as a downloadable file with appropriate HTTP headers
            sent.
        """
        mt = open_memory_tree(int(team), project)

        #Avoid using /tmp by writing into a memory based file
        zipData = StringIO.StringIO()
        zip = zipfile.ZipFile(zipData, "w", zipfile.ZIP_DEFLATED)
        #Need to lock_read before reading any file contents
        mt.lock_read()
        try:
            #Get a list of files in the tree
            files = [f for f in mt.iter_entries_by_dir() if f[1].kind == "file"]
            for filename, file in files:
                #Set external_attr on a ZipInfo to make sure the files are
                #created with the right permissions
                info = zipfile.ZipInfo(filename.encode("ascii"))
                info.external_attr = 0666 << 16L
                #Read the file contents and add to zip
                zip.writestr(info, mt.get_file(file.file_id).read())

            #Need a __init__ in the root of all code exports
            if not "__init__.py" in [f[0].encode("ascii") for f in files]:
                info = zipfile.ZipInfo("__init__.py")
                info.external_attr = 0666 << 16L
                zip.writestr(info, "")

        except:
            return "Error exporting project"
        finally:
            #Always unlock or get GC related errors
            mt.unlock()
        zip.close()
        #Seek back to start of file so read() works later on
        zipData.seek(0)

        if not simulator:
            """
            The zipfile delivered to the robot is the contents of the
            repository as a zip inside another zip that contains firmware.
            """
            #Get a copy of the firmware zip, drop the code zip (in zipData)
            #in it and then put the resulting zip back into zipData
            sysZipData = open(config.get("robot.packagezip")).read()
            sysZipBuffer = StringIO.StringIO(sysZipData)
            
            sysZip = zipfile.ZipFile(sysZipBuffer, "a")
            info = zipfile.ZipInfo(ZIPNAME)
            info.external_attr = 0666 << 16L
            sysZip.writestr(info, zipData.read())
            sysZip.close()

            sysZipBuffer.seek(0)
            zipData = StringIO.StringIO(sysZipBuffer.read())
        
        #Set up headers for correctly serving a zipfile
        cherrypy.response.headers['Content-Type'] = \
                "application/x-download"
        cherrypy.response.headers['Content-Disposition'] = \
                'attachment; filename="' + ZIPNAME + '"'

        #Return the data
        return zipData.read()

    @expose("json")
    @srusers.require(srusers.in_team())
    def filesrc(self, team, project, file=None, revision=-1):
        """
        Returns the contents of the file.
        """

        curtime = time.time()
        b = open_branch( int(team), project )

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
                file_revid = self.get_file_revision(branch_tree, file_id) # get revision the file was last modified
                file_revno = b.revision_id_to_revno(file_revid)
            except:
                code = "Error loading file '%s' at revision %s." % (file, revision)
                revision = 0
            # always unlock:
            finally:
                b.unlock()

        else:
            code = "Error loading file: No filename was supplied by the IDE.  Contact an SR admin!"
            revision = 0

        return dict(curtime=curtime, code=code, autosaved_code=autosaved_code, revision=str(file_revno), path=file,
                name=os.path.basename(file))


    @expose("json")
    @srusers.require(srusers.in_team())
    def gethistory(self, team, project, file, user = None, offset = 0):
        """
        This function retrieves the bzr history for the given file(s)
        to restrict logs to particular user, supply a user parameter
        a maximum of 10 results are sent to the browser, if there are more than 10
        results available, overflow > 0.
        supply an offset to view older results: 0<offset < overflow; offset = 0 is the most recent logs
        """
        b = open_branch(int(team), project)
        revisions = [b.repository.get_revision(r) for r in b.revision_history()]

        #Get a list of authors
        authors = list(set([r.committer for r in revisions]))
        
        #If a user is passed, only show revisions committed by that user
        if user != None:
            revisions = [r for r in revisions if r.committer == user]
    
        #Only show revisions where the delta touches file
        fileid = b.basis_tree().path2id(file)
        if fileid == None:
            #File not found
            return dict()

        def revisionTouchesFile(revision):
            """
            Return true if the revision changed a the file referred to in fileid.
            """
            delta = b.get_revision_delta(b.revision_id_to_revno(revision.revision_id))
            return delta.touches_file_id(fileid)
        revisions = filter(revisionTouchesFile, revisions)

        #Calculate offsets for paging
        try:
            offset = int(offset)
        except ValueError:
            #Someone passed a string
            return dict()
        start = offset*10
        end = start + 10
        maxval = len(revisions)
        if maxval%10 > 0:
            overflow = maxval/10 +1
        else:
            overflow = maxval/10

        revisions = revisions[start:end]

        return dict(  path=file, overflow=overflow, offset=offset, authors=authors,
                      history=[{"author" : r.committer,
                                "date" : time.strftime("%H:%M:%S %d/%m/%Y",
                                                       time.localtime(r.timestamp)),
                                "message" : r.message,
                                "rev" : b.revision_id_to_revno(r.revision_id)}
                              for r in revisions])

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
    def delete(self, team, project, files, kind = 'SVN'):
        """
        Delete files from the repository, and prune empty directories.
        inputs: files - comma seperated list of paths
                kind - one of 'SVN' or 'AUTOSAVES'
        returns (json): Message - a message to show the user
        """

        if files != "":
            files = files.split(",")
            wt = WorkingTree(int(team), project)

            message = "Files deleted successfully: \n" + "\n".join(files)

            for f in files:
                self.autosave.delete(team, '/'+project+'/'+f)

            if kind == 'AUTOSAVES':
                return dict(Message = "AutoSaves deleted successfully: \n" + "\n".join(files))

            wt.remove(files)
            wt.commit('Remove files: '+', '.join(files))

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
    def savefile(self, team, project, filepath, rev, message, code):
        """
        Create/update contents of a file and attempt to commit.
        If file has been updated since submitted text was checked out,
            call update_merge to attempt to merge the changes.
        If file has not been updated since client checked it out,
            call commit_file_simple to commit the new version.

        inputs: path - path of file relative to project root.
                rev - revision of file when it was checked out by client.
        """

#        current_file_revid = self.get_file_revision(basis_tree,fileid)
#        current_file_revno = b.revision_id_to_revno(current_file_revid)

#        if str(current_file_revno) == str(rev):
#        # File has not been modified since the client opened it
#            return self.commit_file_simple(team, project, filepath, rev, message, code, fileid)
#        else:
#            return self.update_merge(team, project, filepath, rev, message, code)

        projWrite = ProjectWrite(team, project)

        projWrite.update_file_contents(filepath, code)

        newrevid = projWrite.commit(message)
        newrevno = projWrite.b.revision_id_to_revno(newrevid)

        success = "True"
        reloadfiles = True
        return dict(new_revision=str(newrevno), code=code,
                    success=success, file=filepath, reloadfiles=reloadfiles)

    def update_merge(self, team, project, filepath, rev, message, code):
        """Attempt to merge some file data with latest revision.
        1. Checkout the branch into a temporary directory
        2. Dump in the new file data
        3. Update file in working copy
        Then either:
        4. Return merge-flagged text if manual merge required
        or
        4. Commit file
        then always:
        5. Delete the temp directory
        """
        print "savefile: going down checkout2tmpdir route."

        #1. Get working tree of branch in temp dir
        wt = WorkingTree(int(team), project)
        reload = "false"

        #TODO: Check for path naugtiness trac#208
        path = os.path.dirname(filepath)
        basename = os.path.basename(filepath)
        fullpath = wt.tmpdir + "/" + filepath

        #2. Dump in the new file data
        target = open(fullpath, "wt")
        target.write(code)
        target.close()

        # try to update
        try:
            conflicts = wt.update()
        except:
            wt.destroy()
            return dict(code=code, success="false", file=filepath, reloadfiles=reload)

#        print "conflicts: " + str(conflicts)
        if conflicts == 0:
            try:
                newrevid = wt.commit(message)
                success = "True"
            except:
                wt.destroy()
                return dict(code=code, success="false", file=filepath, reloadfiles=reload)
        else:
            #Throw the new contents of the file back to the client for
            #tidying, then they can resubmit
            success = "Merge"
            #Grab the merged text.
            mergedfile = open(join(wt.tmpdir, basename), "rt")
            code = mergedfile.read()
            mergedfile.close()

        # find revision number from id
        newrevno = wt.branch.revision_id_to_revno(newrevid)

        #4. Destroy working tree checkout, remove the autosaves
        wt.destroy()
        self.autosave.delete(team, filepath)

        return dict(new_revision=str(newrevno), code=code,
                    success=success, file=filepath, reloadfiles=reload)


    def commit_file_simple(self,team, project, filepath, rev, message, code, fileid):
        """
        Modify contents of a file, or create a new one, in non-merge situations only.
        """
        print "savefile: going down MemoryTree route."

        reload = "false"
        memtree = open_memory_tree(int(team), project)

        memtree.lock_write()
        try:
            # check to see if file exists
            if fileid == None:
                directory = os.path.dirname(filepath)
                self.create_dir(memtree, directory) # create dir if it doesn't exist
                memtree.add([filepath], kinds=['file'])
                fileid = memtree.path2id(filepath)
                reload = "true"
            memtree.put_file_bytes_non_atomic(fileid,code)
            newrevid = memtree.commit(message)
        finally:
            memtree.unlock()

        newrevno = memtree.branch.revision_id_to_revno(newrevid)

        return dict(new_revision=str(newrevno), code=code,
                    success="True", file=filepath, reloadfiles=reload)


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

        b = open_branch( int(team), project )

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

#    #create a new directory
#    @expose("json")
#    @srusers.require(srusers.in_team())
#    def newdir(self, team, project, path, msg):
#        memtree = open_memory_tree(int(team), project)

#        try:
#            created = self.create_dir(memtree, path, msg)
#        except pysvn.ClientError: # TODO BZRPORT: replace with bzr error
#            return dict( success=0, newdir = path,\
#                        feedback="Error creating directory: " + path)

#        if created: # directory was created
#            return dict( success=1, newdir = path,\
#                    feedback="Directory successfully created")
#        else: # directory wasn't created because it already existed
#            return dict( success=0, newdir = path,\
#                    feedback="Directory " + path + " already exists")

    #create a new directory
    @expose("json")
    @srusers.require(srusers.in_team())
    def newdir(self, team, project, path, msg):
        projWrite = ProjectWrite(team, project)

        try:
            projWrite.new_directory(path)
        except pysvn.ClientError: # TODO BZRPORT: replace with bzr error
            return dict( success=0, newdir = path,\
                        feedback="Error creating directory: " + path)

#TODO: try:
        revid = projWrite.commit(msg)
        print "New Revision:"
        print revid

        return dict( success=1, newdir = path,\
                feedback="Directory successfully created")


#        else: # directory wasn't created because it already existed
#            return dict( success=0, newdir = path,\
#                    feedback="Directory " + path + " already exists")


    @expose("json")
    @srusers.require(srusers.in_team())
    def projlist(self, team):
        """Returns a list of projects"""

        try:
            r = open_repo( int(team) )
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

        r = open_repo(int(team))

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

        month = int(mnth)+1
        year = int(yr)
        b = open_branch(team, file)

        try:
            log = b.repository.get_revisions(b.revision_history())
        except:
            logging.debug("Log failed for %s" % file)
            print "failed to retrieve log"
            return dict(path=file, history=[])

        if len(log) == 0: #if there's nothing there
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
            now = now.fromtimestamp(y.timestamp);
            if (start <= now < end):
                result.append(y)


        return dict(  path=file,\
                      history=[{"author":x.get_apparent_author(), \
                      "date":time.strftime("%Y/%m/%d/%H/%M/%S", \
                      time.localtime(x.timestamp)), \
                      "message":x.message, "rev":42} \
                      for x in result])

    @expose("json")
    @srusers.require(srusers.in_team())
    def move(self, team, src, dest, msg=""):
        #   the source and destination arguments may be directories or files
        #   directories rendered empty as a result of the move are automatically 'pruned'
        #   returns status = 0 on success

        r,src_proj,src_path = src.split('/',2)
        r,dest_proj,dest_path = dest.split('/',2)
        if src_proj != dest_proj:
            return dict(new_revision="0", status="1", message="Source and destination projects must match")

        wt = WorkingTree(int(team), src_proj)

        if not wt.has_filename(src_path):
            return dict(new_revision="0", status="1", message="Source file/folder doesn't exist: "+src)

        if not wt.has_filename(os.path.dirname(dest_path)):
            return dict(new_revision="0", status="1", message="Destination folder doesn't exist: "+os.path.dirname(dest))

        if wt.has_filename(dest_path):
            return dict(new_revision="0", status="1", message="Destination already exists: "+dest)

        wt.rename_one(src_path, dest_path)
        wt.commit('Move '+src_path+' to '+dest_path)

        self.autosave.move(team, src, dest)

        return dict(new_revision="0", status="0", message='Sucessfully moved file '+src+' to '+dest)

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

        root,project,file_path = path.split('/',2)
        path,file_name = os.path.split(path)

        # Check out the code
        wt = WorkingTree(int(team), project)
        # Directory we're working in
        td = wt.tmpdir

        if code != 0: #overwrite the version from the repo
            print td+file_path
            tmpfile = open(td+file_path, 'w')
            tmpfile.write(str(code))
            tmpfile.close()

        print 'temp_dir: '+td+"\nfile_path: "+file_path

        # Check out the dummified SR library too
        shutil.copy( config.get("checker.file"), td )

        # Run pychecker
        p = subprocess.Popen( ["pychecker", "-e", "Error", file_path ],
                              cwd = td,
                              stdout = subprocess.PIPE,
                              stderr = subprocess.PIPE )
        output = p.communicate()

        rval = p.wait()

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
                if not line in ['', '\n', 'Processing '+os.path.splitext(file_path)[0]+'...']:
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
