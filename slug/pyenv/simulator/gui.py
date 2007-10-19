import threading
import gtk, gobject

class CodeStore(gtk.ListStore):
    def __init__(self, filename):
        super(CodeStore, self).__init__(gobject.TYPE_INT,
        gobject.TYPE_STRING, gobject.TYPE_STRING)

        self.set_sort_column_id(0, gtk.SORT_ASCENDING)
        
        code = [x.rstrip() for x in open(filename).readlines()]
        lineno = 0
        for line in code:
            self.append([lineno, None, line])
            lineno = lineno + 1

class CodeScroll(gtk.ScrolledWindow):
    def linechanged(self, lineno):
        #Select row val
        selector = self.codelist.get_selection()
        selector.select_path(lineno-1)
        #Make sure it's visible
        self.codelist.scroll_to_cell(lineno-1, None, False)

    def rowactivated(self, treeview, path, view_column):
        lineno = path[0]
        if self.model[lineno][1] == gtk.STOCK_STOP:
            self.model[lineno][1] = None
            self.breaklock.acquire()
            try:
                self.breakpoints.pop(lineno+1)
            except KeyError:
                pass
            self.breaklock.release()
        else:
            self.model[lineno][1] = gtk.STOCK_STOP
            self.breaklock.acquire()
            self.breakpoints[lineno+1] = 1
            self.breaklock.release()

    def __init__(self, model, breakpoints, breaklock):
        super(CodeScroll, self).__init__()

        self.breaklock = breaklock
        self.breakpoints = breakpoints

        self.model = model
        self.codelist = gtk.TreeView(model)
        
        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Line", renderer, text=0)
        self.codelist.append_column(column)

        renderer = gtk.CellRendererPixbuf()
        column = gtk.TreeViewColumn("", renderer, stock_id=1)
        self.codelist.append_column(column)

        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Code", renderer, text=2)
        self.codelist.append_column(column)

        self.codelist.connect("row-activated", self.rowactivated)

        self.codelist.show()
        
        self.add(self.codelist)
        self.show()

class SimGUI:

    def step(self, widget, data=None):
        self.stepevent.set()

    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def check_curline(self):
        curline = self.getcurline() #Hope this is atomic...
        if curline != self.shownline:
            self.scrolledcode.linechanged(curline)
            self.shownline = curline

        return True
    
    def check_debug(self):
        if self.getdebugmode() == self.running:
            self.runtoggle(None, None)
        return True
    
    def runtoggle(self, button, data=None):
        if self.running:
            self.runbutton.set_active(False)
            self.stepbutton.set_sensitive(True)
            self.running = False
            self.setdebugmode(True)
        else:
            self.runbutton.set_active(True)
            self.stepbutton.set_sensitive(False)
            self.running = True
            self.setdebugmode(False)
            self.stepevent.set()

    def __init__(self, getcurline, stepevent, breakpoints, breaklock,
            setdebugmode, getdebugmode):

        self.getcurline = getcurline
        self.shownline = 1
        self.setdebugmode = setdebugmode
        self.getdebugmode = getdebugmode

        self.stepevent = stepevent

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)

        self.window.connect("destroy", self.destroy)

        self.code = CodeStore("robot.py")
        self.scrolledcode = CodeScroll(self.code, breakpoints, breaklock)

        self.stepbutton = gtk.Button("Step")
        self.stepbutton.connect("clicked", self.step, None)
        self.stepbutton.set_sensitive(False)
        self.stepbutton.show()


        self.runbutton = gtk.ToggleButton("Run")
        self.runbutton.set_active(True)
        self.running = True
        self.runbutton.connect("toggled", self.runtoggle, None)
        self.runbutton.show()

        self.hbox = gtk.HBox(False, 10)
        self.hbox.pack_start(self.runbutton, expand=True, fill=True, padding=0)
        self.hbox.pack_start(self.stepbutton, expand=True, fill=True, padding=0)
        self.hbox.show()

        self.vbox = gtk.VBox(False, 10)
        self.vbox.pack_start(self.scrolledcode, expand=True, fill=True,
                padding=0)
        self.vbox.pack_start(self.hbox, expand=False, fill=True, padding=0)
        self.vbox.show()

        self.window.add(self.vbox)

        self.window.show()

        gobject.idle_add(self.check_debug)
        gobject.idle_add(self.check_curline)

        self.window.resize(400, 640)

    def main(self):
        gtk.gdk.threads_init()
        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()


class gtkthread(threading.Thread):
    def __init__(self, getcurline, stepevent, breakpoints, breaklock,
            setdebugmode, getdebugmode):
        threading.Thread.__init__(self)
        self.hello = SimGUI(getcurline, stepevent, breakpoints, breaklock,
                setdebugmode, getdebugmode)
    def run(self):
        self.hello.main()
