import StringIO, sys
import code
import threading, Queue
import gtk, gobject

class LocalsStore(gtk.ListStore):
    def __init__(self, locals):
        super(LocalsStore, self).__init__(gobject.TYPE_STRING,
                gobject.TYPE_STRING)

        self.set_sort_column_id(0, gtk.SORT_ASCENDING)
        
        for k, v in locals.iteritems():
            self.append([k, v])

class LocalsScroll(gtk.ScrolledWindow):
    def __init__(self, model):
        super(LocalsScroll, self).__init__()

        self.model = model
        self.localslist = gtk.TreeView(model)
        
        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Variable", renderer, text=0)
        self.localslist.append_column(column)

        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn("Value", renderer, text=1)
        self.localslist.append_column(column)

        self.localslist.show()
        
        self.add(self.localslist)
        self.show()
    def set_model(self, model):
        self.localslist.set_model(model)

class CodeStore(gtk.ListStore):
    def __init__(self, filename):
        super(CodeStore, self).__init__(gobject.TYPE_INT,
        gobject.TYPE_STRING, gobject.TYPE_STRING)

        self.set_sort_column_id(0, gtk.SORT_ASCENDING)
        
        lines = [x.rstrip() for x in open(filename).readlines()]
        lineno = 0
        for line in lines:
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
                self.breakpoints.pop((self.path, lineno))
            except KeyError:
                pass
            self.breaklock.release()
        else:
            self.model[lineno][1] = gtk.STOCK_STOP
            self.breaklock.acquire()
            self.breakpoints[(self.path, lineno)] = 1
            self.breaklock.release()

    def __init__(self, model, path, breakpoints, breaklock):
        super(CodeScroll, self).__init__()

        self.breaklock = breaklock
        self.breakpoints = breakpoints

        self.path = path

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
        self.tosimq.put("STEP")

    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()
        sys.exit(0)

    def checkq(self):
        #TODO: Get a signal on the queue having an item on it
        try:
            curfile, curline, locals = self.fromsimq.get_nowait()
            if curfile in self.files:
                self.pages[curfile].linechanged(curline)
                self.codepages.set_current_page(self.files.index(curfile))

            if len(locals) > 0:
                #In debug mode
                self.localsdict = locals

                self.locals = LocalsStore(self.localsdict)
                self.scrolledlocals.set_model(self.locals)
                
                if self.running:
                    self.cmdtext.set_sensitive(True)
                    self.runbutton.set_active(False)
                    self.stepbutton.set_sensitive(True)
                    self.running = False
            else:
                if not self.running:
                    self.runbutton.set_active(True)
                    self.stepbutton.set_sensitive(False)
                    self.cmdtext.set_sensitive(False)
                    self.running = True

        except Queue.Empty:
            pass
        return True

    def runtoggle(self, button, data=None):
        if self.running:
            self.tosimq.put("STOP")
        else:
            self.tosimq.put("RUN")
                        
    def writetocodewin(self, text):
        iter = self.cmdoutbuf.get_end_iter()
        self.cmdoutbuf.insert(iter, text)
        self.scrolloutputtobottom()

    def check_stdout(self):
        if self.s.len > self.spos:
            curlen = self.s.len
            self.s.seek(self.spos)
            txt = self.s.read(curlen-self.spos)
            self.spos = curlen
            self.writetocodewin(txt)
        return True
    
    def scrolloutputtobottom(self):
        self.cmdoutput.scroll_to_mark(self.cmdoutbuf.get_insert(), 0)
    
    def processcmd(self, widget, data=None):
        ii = code.InteractiveInterpreter(locals=self.localsdict)
        ii.write = self.writetocodewin
        ii.runsource(self.cmdtext.get_text())

    def __init__(self, breakpoints, breaklock, fromsimq, tosimq, files):

        self.running = False
        self.fromsimq = fromsimq
        self.tosimq = tosimq

        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)

        self.window.connect("destroy", self.destroy)
        
        self.codepages = gtk.Notebook()
        self.pages = {}
        self.files = files

        for file in self.files:
            codestore = CodeStore(file)
            scrolledcode = CodeScroll(codestore, file, breakpoints, breaklock)
            self.pages[file] = scrolledcode
            self.codepages.append_page(scrolledcode, gtk.Label(file))

        self.codepages.show()

        self.locals = LocalsStore({})
        self.scrolledlocals = LocalsScroll(self.locals)

        self.stepbutton = gtk.Button("Step")
        self.stepbutton.connect("clicked", self.step, None)
        self.stepbutton.set_sensitive(True)
        self.stepbutton.show()

        self.runbutton = gtk.ToggleButton("Run")
        self.runbutton.set_active(self.running)
        self.runbutton.connect("toggled", self.runtoggle, None)
        self.runbutton.show()

        self.hbox = gtk.HBox(False, 10)
        self.hbox.pack_start(self.runbutton, expand=True, fill=True, padding=0)
        self.hbox.pack_start(self.stepbutton, expand=True, fill=True, padding=0)
        self.hbox.show()

        self.cmdtext = gtk.Entry()
        self.cmdtext.set_sensitive(False)
        self.cmdtext.connect("activate", self.processcmd)
        self.cmdtext.show()
        self.cmdoutput = gtk.TextView()
        self.cmdoutput.set_editable(False)
        self.cmdoutput.set_wrap_mode(gtk.WRAP_CHAR)
        self.cmdoutbuf = self.cmdoutput.get_buffer()
        self.cmdoutput.show()
        self.cmdoutscroll = gtk.ScrolledWindow()
        self.cmdoutscroll.add(self.cmdoutput)
        self.cmdoutscroll.set_policy(gtk.POLICY_NEVER, gtk.POLICY_ALWAYS)
        self.cmdoutscroll.show()

        self.vbox = gtk.VBox(False, 10)
        self.vbox.pack_start(self.codepages, expand=True, fill=True,
                padding=0)
        self.vbox.pack_start(self.hbox, expand=False, fill=True, padding=0)
        self.vbox.pack_start(self.scrolledlocals, expand=True, fill=True,
                padding=0)
        self.vbox.pack_start(self.cmdtext, expand=False, fill=True, padding=0)
        self.vbox.pack_start(self.cmdoutscroll, expand=True, fill=True, padding=0)
        self.vbox.show()

        self.window.add(self.vbox)

        self.window.show()

        self.s = StringIO.StringIO()
        self.spos = 0
        self.stdout = sys.stdout
        self.stderr = sys.stderr
        #sys.stdout = self.s
        #sys.stderr = self.s

        gobject.idle_add(self.checkq)
        gobject.idle_add(self.check_stdout)

        self.window.resize(400, 640)

    def main(self):
        gtk.gdk.threads_init()
        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()


class gtkthread(threading.Thread):
    def __init__(self, breakpoints, breaklock, fromsimq, tosimq, files):
        threading.Thread.__init__(self)
        self.gui = SimGUI(breakpoints, breaklock, fromsimq, tosimq, files)
    def run(self):
        self.gui.main()
