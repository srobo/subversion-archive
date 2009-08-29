import bzrlib.branch, bzrlib.repository, bzrlib.workingtree, bzrlib.memorytree, bzrlib.tree, bzrlib.errors, bzrlib.progress, bzrlib.merge, bzrlib.generate_ids, bzrlib.revision
import os
import user as srusers
import tempfile

class ProjectWrite():
    """
    A class for making modifications to the project, ie those ending in a commit.
    """
    def __init__(self, team, project, revid=None, revno=None):
        """
        Open a TranformPreview object for a revision of the project.
        """
        self.b = open_branch(team, project)

        if revid == None:
            if revno == None or revno == 0 or revno == "0":
                # If no revid and no revno was specified, use latest
                revid = self.b.last_revision()
            else:
                # revno was specified, use that
                revid = self.b.get_rev_id(int(revno))

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

        # update the objects
        self.TransPrev = tt2
        self._update_tree()

        self.revid = revid_latest
        self.conflicts = tree_merger.cooked_conflicts
        return

    def get_file_text(self, path):
        """
        Returns text from a file in the tree
        """
        file_id = self.PrevTree.path2id(path)

        if file_id == None:
            # file doesn't exist
            raise Exception # TODO

        return self.PrevTree.get_file_text(file_id)

    def commit(self, message=""):
        """
        Commit changed tree.
        """
        last_revno, last_revid = self.b.last_revision_info()

        if not self.revid == last_revid:
            bzrlib.errors.OutOfDateTree # cannot commit, tree not up to date
        if not len(self.conflicts) == 0:
            return None # cannot commit, conflicts remain

        self.b = bzrlib.branch.Branch.open(self.b.base)
        self.b.lock_write()

        try:
            if hasattr(self.PrevTree, "commit"):
                # As of bzr 1.18 PreviewTrees have built-in commit method.
                #self.PrevTree.set_parent_ids([ self.revid ]) # needed here?
                revid_new = self.PrevTree.commit(message)
            else:
                if last_revid == bzrlib.revision.NULL_REVISION:
                    parent_ids = [] # no existing commits on this branch
                else:
                    parent_ids = [self.revid]
                revprops = {"branch-nick":self.b.nick} # is this necessary?
                builder = self.b.get_commit_builder(parent_ids, revprops = revprops)

                changes = list(builder.record_iter_changes(
                                self.PrevTree, self.revid, self.TransPrev.iter_changes()))
                builder.finish_inventory()
                revid_new = builder.commit(message)
                revno_new = last_revno + 1
                self.b.set_last_revision_info(revno_new, revid_new)
        finally:
            # always unlock branch
            # NOTE: an exception during unlock() here can mask other exceptions during try,
            # so try:unlock to absorb this and allow original exception through.
            # TODO: more elegant solution
            try:
                self.b.unlock()
            except:
                pass

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
            contents - string to insert into file
            create - when True (default) if file doesn't exist it will be created, as well as parent directories.
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

            self.TransPrev.new_file(file_name, parent_trans_id, contents, file_id)

            self._update_tree() # update PrevTree to reflect new file

        else:
            trans_id = self.TransPrev.trans_id_file_id(file_id)

            # delete existing contents
            self.TransPrev.delete_contents(trans_id)

            # add new contents
            self.TransPrev.create_file(contents, trans_id)

        return

    def copy(self, from_path, to_path):
        """
        Make a copy of a file. TODO: whole directories.
        """

        file_contents = self.get_file_text(from_path)

        if self.PrevTree.path2id(to_path) is not None:
            # target file already exists
            raise Exception # TODO

        self.update_file_contents(to_path, file_contents)

        return

    def destroy(self):
        """
        Clean up.
        """
        self.TransPrev.finalize()

    def list_files(self, include_root=False, from_dir=None, recursive=True):
        """
        List all files as (path, class, kind, id, entry).
        :param include_root: if True, do not return an entry for the root
        :param from_dir: start from this directory or None for the root
        :param recursive: whether to recurse into subdirectories or not
        """
        return list(self.PrevTree.list_files(include_root=include_root, from_dir=from_dir, recursive=recursive))

def open_branch(team, project):
    """
    Open the project branch for the team.

    TODO: Check the logged in user has permission to do this!
    """
    repoLoc = srusers.get_svnrepo( team )
    branchloc = repoLoc + "/" + project
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
    NOTE: DEPRECATED. Use ProjWrite instead.
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
