import gtk, gobject

class CodeStore(gtk.ListStore):
    def __init__(self, filename, contents):
        super(CodeStore, self).__init__(gobject.TYPE_INT,
        gobject.TYPE_STRING, gobject.TYPE_STRING)

        self.set_sort_column_id(0, gtk.SORT_ASCENDING)
        
        lines = [x.rstrip() for x in contents.split("\n")]
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

