import threading
import gtk, gobject

class CodeStore(gtk.ListStore):
    def __init__(self, filename):
        super(CodeStore, self).__init__(gobject.TYPE_INT,
        gobject.TYPE_STRING)

        self.set_sort_column_id(0, gtk.SORT_ASCENDING)
        
        code = [x.rstrip() for x in open(filename).readlines()]
        lineno = 0
        for line in code:
            self.append([lineno, line])
            lineno = lineno + 1

class CodeScroll(gtk.ScrolledWindow):
    def linechanged(self, lineno):
        #Select row val
        selector = self.codelist.get_selection()
        selector.select_path(lineno-1)
        #Make sure it's visible
        self.codelist.scroll_to_cell(lineno-1, None, False)

    def __init__(self, model):
        super(CodeScroll, self).__init__()

        self.codelist = gtk.TreeView(model)
        
        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Line", renderer, text=0)
        self.codelist.append_column(column)

        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Code", renderer, text=1)
        self.codelist.append_column(column)

        self.codelist.show()
        
        self.add(self.codelist)
        self.show()

class SimGUI:

    def hello(self, widget, data=None):
        print "Hey there"

    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def check_curline(self):
        curline = self.curline[0] #Hope this is atomic...
        if curline != self.shownline:
            self.scrolledcode.linechanged(curline)
            self.shownline = curline

        return True

    def __init__(self, curline):

        self.curline = curline
        self.shownline = 1

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)

        self.window.connect("destroy", self.destroy)

        self.code = CodeStore("robot.py")
        self.scrolledcode = CodeScroll(self.code)

        self.button = gtk.Button("Step")
        self.button.connect("clicked", self.hello, None)
        self.button.show()

        self.vbox = gtk.VBox(False, 10)
        self.vbox.pack_start(self.scrolledcode, expand=True, fill=True,
                padding=0)
        self.vbox.pack_start(self.button, expand=False, fill=True, padding=0)
        self.vbox.show()

        self.window.add(self.vbox)

        self.window.show()

        gobject.idle_add(self.check_curline)

        self.window.resize(400, 640)

    def main(self):
        gtk.gdk.threads_init()
        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()


class gtkthread(threading.Thread):
    def __init__(self, curline):
        threading.Thread.__init__(self)
        self.hello = SimGUI(curline)
    def run(self):
        self.hello.main()
