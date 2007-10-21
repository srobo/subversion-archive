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

