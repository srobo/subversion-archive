from turbogears import controllers, expose, config
from turbogears.feed import FeedController
import cherrypy, model
from sqlobject import sqlbuilder
import logging
import bzrlib.errors, bzrlib.tree, bzrlib.revisionspec
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
from vcs_bzr import ProjectWrite, open_branch, open_repo, WorkingTree

log = logging.getLogger("roboide.controllers")

ZIPNAME = "robot.zip"

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

def get_version():
    p = subprocess.Popen( ["svnversion"],
                          stdout = subprocess.PIPE,
                          stderr = subprocess.PIPE )
    output = p.communicate()
    rval = p.wait()
    ver = output[0].strip()
    if ver != 'exported':
        return 'r'+ver
    return 'Unknown'

class Root(controllers.RootController):
    user = srusers.User()
    fw = fw.FwServe()
    autosave = srautosave.Autosave()
    #feed = Feed()
    switchboard = switchboard.Switchboard()
    version = get_version()

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

    @expose("json")
    def info(self):
        info = dict(Version=self.version)
        return dict(info=info)

    def get_project_path(self, path):
        """
        Get a project name and filepath from a path
        inputs:
            path - str to a file
        returns:
            tuple containing the project and path
        """
        root,project,file_path = path.split(os.path.sep,2)
        return project,file_path

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
        b = open_branch(int(team), project)
        rev_tree = b.basis_tree()   # get latest revision tree for branch

        #Avoid using /tmp by writing into a memory based file
        zipData = StringIO.StringIO()
        zip = zipfile.ZipFile(zipData, "w", zipfile.ZIP_DEFLATED)
        #Need to lock_read before reading any file contents
        rev_tree.lock_read()
        try:
            #Get a list of files in the tree
            files = [f for f in rev_tree.iter_entries_by_dir() if f[1].kind == "file"]
            for filename, file in files:
                #Set external_attr on a ZipInfo to make sure the files are
                #created with the right permissions
                info = zipfile.ZipInfo(filename.encode("ascii"))
                info.external_attr = 0666 << 16L
                #Read the file contents and add to zip
                zip.writestr(info, rev_tree.get_file(file.file_id).read())

            #Need a __init__ in the root of all code exports
            if not "__init__.py" in [f[0].encode("ascii") for f in files]:
                info = zipfile.ZipInfo("__init__.py")
                info.external_attr = 0666 << 16L
                zip.writestr(info, "")

        except:
            return "Error exporting project"
        finally:
            #Always unlock or get GC related errors
            rev_tree.unlock()
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
    def filesrc(self, team, file=None, revision=None):
        """
        Returns the contents of the file.
        """

        file_path = file	#save for later
        project,file = self.get_project_path(file_path)
        curtime = time.time()
        b = open_branch( int(team), project )

        #TODO: Need to security check here! No ../../ or /etc/passwd nautiness trac#208

        autosaved_code = self.autosave.getfilesrc(team, file_path, 1)

        if revision == None or revision == "HEAD":
            revno, revid = b.last_revision_info()
        else:
            revno = int(revision)
            revid = b.get_rev_id(revno)

        if file != None and file != "":  #TODO BZRPORT: URL checking
            #Load file from bzr
            # TODO BZRPORT: mime checking. Bzr doesn't have a mime property so the file will need to be checked with python
            try:
                branch_tree = b.repository.revision_tree(revid)
                file_id = branch_tree.path2id(file)
                b.lock_read()
                code = branch_tree.get_file_text(file_id)
                file_revid = self.get_file_revision(branch_tree, file_id) # get revision the file was last modified
                file_revno = b.revision_id_to_revno(file_revid)
            except:
                code = "Error loading file '%s' at revision %s." % (file, revision)
                file_revno = 0
            # always unlock:
            finally:
                b.unlock()

        else:
            code = "Error loading file: No filename was supplied by the IDE.  Contact an SR admin!"
            revision = 0

        return dict(curtime=curtime, code=code, autosaved_code=autosaved_code, file_rev=str(file_revno), revision=revno, path=file_path,
                name=os.path.basename(file))


    @expose("json")
    @srusers.require(srusers.in_team())
    def gethistory(self, team, file, user = None, offset = 0):
        """
        This function retrieves the bzr history for the given file(s)
        to restrict logs to particular user, supply a user parameter
        a maximum of 10 results are sent to the browser, if there are more than 10
        results available, overflow > 0.
        supply an offset to view older results: 0<offset < overflow; offset = 0 is the most recent logs
        """
        if file[:9] == 'New File ':
            return dict(path=file, history=[])

        file_path = file	#save for later
        project,file = self.get_project_path(file_path)
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
        revisions.reverse()

        return dict(  path=file_path, overflow=overflow, offset=offset, authors=authors,
                      history=[{"author" : r.committer,
                                "date" : time.strftime("%H:%M:%S %d/%m/%Y",
                                                       time.localtime(r.timestamp)),
                                "message" : r.message,
                                "rev" : b.revision_id_to_revno(r.revision_id)}
                              for r in revisions])

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

            message = "Files deleted successfully: "+project+" >\n" + "\n".join(files)

            for f in files:
                self.autosave.delete(team, '/'+project+'/'+f)

            if kind == 'AUTOSAVES':
                return dict(Message = "AutoSaves deleted successfully: \n" + "\n".join(files))

            wt.remove(files)

            # find out current user
            ide_user = str(srusers.get_curuser())

            revproperties = {"authors":ide_user}
            wt.commit('Remove files: '+', '.join(files), revprops=revproperties)
            wt.destroy()

            return dict(Message = message)

    @expose("json")
    @srusers.require(srusers.in_team())
    def savefile(self, team, filepath, rev, message, code):
        """
        Create/update contents of a file and attempt to commit.
        If file has been updated since submitted text was checked out,
            call update_merge to attempt to merge the changes.
        If file has not been updated since client checked it out,
            call commit_file_simple to commit the new version.

        inputs: path - path of file relative to project root.
                rev - revision of file when it was checked out by client.
        """

        project,filepath = self.get_project_path(filepath)

        projWrite = ProjectWrite(team, project, revno=rev)

        projWrite.update_file_contents(filepath, code)

        reloadfiles = "True" # TODO: determine whether or not file list needs refreshing

        try:
            newrevno,newrevid = projWrite.commit(message)
            success = "True"
        except bzrlib.errors.OutOfDateTree:
            # a commit has occurred since code was opened.
            # A merge will need to take place
            code, newrevno, newrevid = projWrite.merge(filepath)
            if len(projWrite.conflicts) == 0:
                # TODO: when committing a merged transform preview affecting more than one file,
                    #       the text changes do not commit despite the merge succeeding and returning correct text.
                    #       solution for now is to open a new transform preview and pump the new code into it.
                pw2 = ProjectWrite(team, project)
                pw2.update_file_contents(filepath, code)
                newrevno, newrevid = pw2.commit(message)
                success = "AutoMerge"
                pw2.destroy()
            else:
                return dict(new_revision=newrevno, code=code,
                    success="Merge", file=filepath, reloadfiles=reloadfiles)
        finally:
            projWrite.destroy()


        return dict(new_revision=str(newrevno), code=code,
                    success=success, file=filepath, reloadfiles=reloadfiles)

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

        autosave_data = self.autosave.getfilesrc(team, project+rootpath)

        def branch_recurse(project, path, entry, files, given_parent_id):
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
                        children_list, next_path, next_entry = branch_recurse(project, next_path, next_entry, files, entry.file_id)
                    except StopIteration: # No more files to iterate through after this one
                        next_entry = None # break after adding this entry
                        children_list = [] # no more items, so there can't be any children

                    entry_list.append({
                                        "name": entry.name,
                                        "path": project+path,
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
                                        "path": project+path,
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

        tree, last_path, last_entry = branch_recurse('/'+project+'/', first_path, first_entry, files, tree_root)

        return dict(tree = tree)

    #create a new directory
    @expose("json")
    @srusers.require(srusers.in_team())
    def newdir(self, team, path, msg):
        project, dirpath = self.get_project_path(path)
        projWrite = ProjectWrite(team, project)

        try:
            projWrite.new_directory(dirpath)
        except pysvn.ClientError: # TODO BZRPORT: replace with bzr error
            return dict( success=0, newdir = path,\
                        feedback="Error creating directory: " + path)

#TODO: try:
        revno,revid = projWrite.commit(msg)

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

        url = srusers.get_svnrepo( team ) + "/" + name

        r.bzrdir.create_branch_convenience(base=url,force_new_tree=False)

        return dict( )

    @expose("json")
    @srusers.require(srusers.in_team())
    def revert(self, team, files, torev, message):

        file_list = files.split(',')
        if len(file_list) == 0:
            return dict(Message = 'Revert failed - no files specified', status = 1)

        project, file = self.get_project_path(file_list[0])
        rev_spec = bzrlib.revisionspec.RevisionSpec.from_string(torev)
        file_list = [self.get_project_path(f)[1] for f in file_list]

        wt = WorkingTree(team, project)
        rev_tree = rev_spec.as_tree(wt.branch)

        wt.revert(file_list, rev_tree)

        # find out current user
        ide_user = str(srusers.get_curuser())

        revproperties = {"authors":ide_user}

        wt.commit(message, revprops=revproperties)
        newrev, id = wt.branch.last_revision_info();
        wt.destroy()

        return dict(new_revision=newrev, code = "", success="Success !!!", status = 0)

#from undelete
        return dict(fail = fail, success = ','.join(success), status = status)

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

        result.reverse()

        return dict(  path=file,\
                      history=[{"author":x.get_apparent_author(), \
                      "date":time.strftime("%Y/%m/%d/%H/%M/%S", \
                      time.localtime(x.timestamp)), \
                      "message":x.message, "rev":b.revision_id_to_revno(x.revision_id)} \
                      for x in result])

    @expose("json")
    @srusers.require(srusers.in_team())
    def move(self, team, src, dest, msg=""):
        #   the source and destination arguments may be directories or files
        #   directories rendered empty as a result of the move are automatically 'pruned'
        #   returns status = 0 on success

        src_proj,src_path = self.get_project_path(src)
        dest_proj,dest_path = self.get_project_path(dest)
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

        # find out current user
        ide_user = str(srusers.get_curuser())

        revproperties = {"authors":ide_user}

        wt.commit('Move '+src_path+' to '+dest_path, revprops=revproperties)
        wt.destroy()

        self.autosave.move(team, src, dest)

        return dict(new_revision="0", status="0", message='Sucessfully moved file '+src+' to '+dest)

    @expose("json")
    @srusers.require(srusers.in_team())
    def copyproj(self, team, src, dest):
        # Create a temporary directory
        tmpdir = tempfile.mkdtemp()
        #open the branch and sprout a new copy, in the temp dir
        b = open_branch(team, src)
        self.createproj(dest, team)
        nb = open_branch(team, dest)
        b.push(nb)
        return dict(status=0)

    @expose("json")
    @srusers.require(srusers.in_team())
    def copy(self, team, src="", dest="", msg="SVN Copy", rev="0"):
        return self.cp(team, src, dest, msg, rev)

    @srusers.require(srusers.in_team())
    def cp(self, team, src="", dest="", msg="Copy", rev="0"):

        project,src = self.get_project_path(src)
        dest_project,dest = self.get_project_path(dest)
        if dest_project != project:
            return dict(new_revision = "0", status="1", message="Copy Failed: Source and destination projects must match")

        if rev == "0":
            rev = None

        projWrite = ProjectWrite(team, project, revno=rev)

        if src == "":
            return dict(new_revision="0", status="1", msg="No Source file/folder specified");
        if dest == "":
            return dict(new_revision="0", status="1", msg="No Destination file/folder specified");

        try:
            projWrite.copy(src, dest)
            new_revno,new_rev_id = projWrite.commit(msg)

        except pysvn.ClientError, e:
            return dict(new_revision = "0", status="1", message="Copy Failed: "+str(e))

        return dict(new_revision = str(new_revno), status="0", message="copy successful")

    @expose("json")
    @srusers.require(srusers.in_team())
    def checkcode(self, team, path, code=0, date=None):

        project,file_path = self.get_project_path(path)
        path,file_name = os.path.split(path)

        # Check out the code
        wt = WorkingTree(int(team), project)
        # Directory we're working in
        td = wt.tmpdir

        if code != 0: #overwrite the version from the repo
            print td+os.path.sep+file_path
            tmpfile = open(td+os.path.sep+file_path, 'w')
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
        wt.destroy()

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
