import pysvn

client = pysvn.Client()

REPO = "file:///home/stephen/ecssr/web/repo/"

files = client.info2(REPO, recurse=True)

class Node (object):
    def __init__(self, name):
        self.name = name
        self.children = {}

    def __repr__(self):
        txt = "Node: %s" % self.name
        for child in self.children.iteritems():
            txt = txt + "\n" + repr(child)
        return txt

    def __getitem__(self, key):
        return self.children[key]

    def __setitem__(self, key, item):
        self.children[key] = item

head = Node("HEAD")

for (filename,details) in files:
    basename = filename.split("/")[-1:][0]
    if basename != "repo":
        top = head
        for path in filename.split("/"):
            try:
                top = top[path]
            except KeyError:
                top[path] = Node(basename)
                break

print head
for foo in head.children.iteritems():
    print foo
    print
