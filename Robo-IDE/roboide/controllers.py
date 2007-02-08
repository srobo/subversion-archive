from turbogears import controllers, expose
import logging
import pysvn
import time
import re
import tempfile, shutil
from os.path import join
log = logging.getLogger("roboide.controllers")

REPO = "file:///home/stephen/ecssr/web/repo/"

class Root(controllers.RootController):
    @expose(template="roboide.templates.filesrc", format="html-straight")
    def filesrc(self, file=None, action=None, language="generic",
        engine="msie", revision="HEAD"):
        """
        Returns the contents of the file.
        Turns out the action parameter can be edit. Not sure how this is
        useful - won't it always be edit?
        TODO: Cope with revision other than head.
        """
        curtime = time.time()
        if file != None:
            #Load file from SVN
            #TODO: Need to security check here! No ../../ or /etc/passwd nautiness
            client = pysvn.Client()
            #TODO: Could replace this with a lock?
            while True:
                ver = client.info2(REPO + file)[0][1]["rev"]
                code = client.cat(REPO + file)
                ver2 = client.info2(REPO + file)[0][1]["rev"]
                if ver.number == ver2.number:
                    break
                print "Revision change whilst catting. This can " + \
                    "happen but should be very unusual. Will cat again."
            revision = ver.number
        else:
            code = "No File Loaded"
            revision = 0

        #Need to tidy up the file for its journey through html
        #code = re.sub("/&/","&amp;",code)
        #code = re.sub("/</","&lt;",code)
        #code = re.sub("/>/","&gt;",code)
        #The following lines are in the PHP, commented out. Won't implement
        #for now
        #$code = preg_replace("/\r\n/","<br>",$code); // opera and khtml
        #engines
        print code
        return dict(curtime=curtime, code=code, language=language, \
                    engine=engine, revision=revision, path=file)

    #TODO: Create an action that uses client.log to return a JSON list of
    #previous file revisions for a mochikit drop down


    @expose("json")
    def savefile(self, file, rev, message, code):
        """Write a commit of one file.
        1. SVN checkout the file's directory
        2. Dump in the new file data
        3. Commit that directory with the new data and the message
        4. Wipe the directory

        TODO: Usernames. Check rev is a number.
        """
        print "********************************"
        client = pysvn.Client()
        #1. SVN checkout of file's directory
        #TODO: Check for path naugtiness
        path = file[:file.rfind("/")]
        basename = file[file.rfind("/")+1:] 
        tmpdir = tempfile.mkdtemp()
        print path, basename, tmpdir
        #This returns a revision number. Always 0. Great.
        try:
            client.checkout(REPO + path, tmpdir, recurse=False, \
                revision=pysvn.Revision(pysvn.opt_revision_kind.number, int(rev)))
        except pysvn.ClientError:
            #Some fool (me) passed in a filename that doesn't exist.
            return dict(new_revision="0", code="", success=False)

        #2. Dump in the new file data
        target = open(join(tmpdir, basename), "wt")
        target.write(code)
        target.close()

        #3. Commit the new directory
        try:
            newrev = client.checkin([tmpdir], message)
            success = True
            code = ""
        except pysvn.ClientError:
            #Can't commit - merge issues
            #Need to bring local copy up to speed
            #Hopefully this means a merge!
            #Throw the new contents of the file back to the client for
            #tidying, then they can resubmit
            newrev = client.update(tmpdir)[0] #This comes back as a list.
            print "###################"
            print newrev
            success = False
            #Grab the merged text.
            mergedfile = open(join(tmpdir, basename), "rt")
            code = mergedfile.read()
            mergedfile.close()
            #This has to be returned

        #4. Wipe the directory
        shutil.rmtree(tmpdir)

        #Need merge management here
        return dict(new_revision=str(newrev.number), code=code,
                    success=success)

    @expose(template="roboide.templates.files")
    def index(self):
        #Really need to seperate this out in a min
        client = pysvn.Client()
        
        files = client.info2(REPO, recurse=True)

        class Node (object):
            def __init__(self, name, path, kind):
                self.name = name
                self.path = path
                self.kind = kind
                self.children = {}
                
        head = Node("HEAD", "", "")

        for (filename,details) in files:
            basename = filename.split("/")[-1:][0]
            if basename != "repo":
                top = head 
                for path in filename.split("/"):
                    try: 
                        top = top.children[path]
                    except KeyError:
                        top.children[path] = Node(basename, filename, details["kind"])

        return dict(tree=head)
