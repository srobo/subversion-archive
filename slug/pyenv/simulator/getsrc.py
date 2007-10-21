import gtk, sys, os, os.path

class SourceLoader():
    def delete_event(self, widget, event, data=None):
        return False

    def destroy(self, widget, data=None):
        gtk.main_quit()

    def fileselected(self, widget, data=None):
        self.ziploc.set_text(self.filesel.get_filename())
        self.filesel.destroy()
        self.window.set_sensitive(True)
        self.checklocalzip()

    def checklocalzip(self, widget=None, data=None):
        loc = self.ziploc.get_text()
        if loc[-3:] == "zip":
            self.uselocalzip.set_sensitive(True)
        else:
            self.uselocalzip.set_sensitive(False)

    def fileselcancel(self, widget, data=None):
        self.filesel.destroy()
        self.window.set_sensitive(True)

    def findzip(self, widget, data=None):
        self.window.set_sensitive(False)
        self.filesel = gtk.FileSelection("Locate zip file")
        #TODO: Make same as the last time this was used
        self.filesel.set_filename("robot.zip")
        self.filesel.ok_button.connect("clicked", self.fileselected)
        self.filesel.cancel_button.connect("clicked", self.fileselcancel)
        self.filesel.connect("destroy", self.fileselcancel)
        self.filesel.show()

    def openlocal(self, widget, data=None):
        #1 Check file exists
        loc = self.ziploc.get_text()
        if os.access(loc, os.R_OK):
            #Add it to the search path
            sys.path.insert(0, loc)
            try:
                import robot
                self.success = loc
                self.window.destroy()
                return
            except:
                pass
        
        d = gtk.Dialog("Error", None, gtk.DIALOG_MODAL,
                (gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
        l = gtk.Label("Could not open zip file.")
        l.show()
        d.vbox.pack_start(l, False, True, 0)
        d.connect("response", lambda x, y: d.destroy())
        d.run()

    def __init__(self):
        self.success = False
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
    
        vbox = gtk.VBox(False, 10)
        
        localhbox = gtk.HBox(False, 10)
        ziplab = gtk.Label("Zip location")
        ziplab.show()
        localhbox.pack_start(ziplab, expand=False,
                fill=True, padding=0)

        self.ziploc = gtk.Entry()
        self.ziploc.connect("changed", self.checklocalzip)
        self.ziploc.show()
        localhbox.pack_start(self.ziploc, expand=True, fill=True, padding=0)

        self.findzipbutton = gtk.Button("Browse")
        self.findzipbutton.connect("clicked", self.findzip)
        self.findzipbutton.show()
        localhbox.pack_start(self.findzipbutton, expand=False, fill=True, padding=0)

        self.uselocalzip = gtk.Button("Open")
        self.uselocalzip.connect("clicked", self.openlocal)
        self.uselocalzip.set_sensitive(False)
        self.uselocalzip.show()
        localhbox.pack_start(self.uselocalzip, expand=False, fill=True,
                padding=0)
        
        localhbox.show()
        vbox.pack_start(localhbox, expand=False, fill=True, padding=0)
        vbox.show()
        self.window.add(vbox)
        self.window.show()

    def main(self):
        gtk.main()

if __name__ == "__main__":
    sl = SourceLoader()
    sl.main()
    print sl.success
