import gtk, sys, os, os.path, gobject
import http, Queue

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
                self.success = loc, False
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

    def download(self, widget, data=None):
        self.inq = Queue.Queue()
        username = self.tu.get_text()
        password = self.tp.get_text()
        self.tu.set_sensitive(False)
        self.tp.set_sensitive(False)
        self.netbut.set_sensitive(False)
        self.downloader = http.BGDownloader(username, password, self.inq)
        self.downloader.start()
        gobject.idle_add(self.checkdownload)

    def checkdownload(self):
        try:
            fileloc = self.inq.get_nowait()
            if fileloc == "":
                d = gtk.Dialog("Error", None, gtk.DIALOG_MODAL,
                        (gtk.STOCK_OK, gtk.RESPONSE_ACCEPT))
                l = gtk.Label("Could not download robot code from Student Robotics")
                l.show()
                d.vbox.pack_start(l, False, True, 0)
                d.connect("response", lambda x, y: d.destroy())
                d.run()
                self.tu.set_sensitive(True)
                self.tp.set_sensitive(True)
                self.netbut.set_sensitive(True)
            else:
                sys.path.insert(0, fileloc)
                try:
                    import robot
                    print fileloc
                    self.success = fileloc, True
                    self.window.destroy()
                    return
                except:
                    pass
            return False
        except Queue.Empty:
            return True

    def checkfields(self, widget, data=None):
        if len(self.tu.get_text()) > 0 and \
           len(self.tp.get_text()) > 0:
               self.netbut.set_sensitive(True)
        else:
               self.netbut.set_sensitive(False)

    def __init__(self):
        self.success = False, False
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
    
        vbox = gtk.VBox(False, 10)
        ll = gtk.Label("Use local file:")
        ll.show()
        vbox.pack_start(ll, expand=False, fill=True, padding=0)
        
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
        ln = gtk.Label("or download from http://ide.studentrobotics.org:")
        ln.show()
        vbox.pack_start(ln, expand=False, fill=True, padding=0)

        nethbox = gtk.HBox(False, 10)

        lu = gtk.Label("Username:")
        lu.show()
        nethbox.pack_start(lu, expand=False, fill=True, padding=0)

        self.tu = gtk.Entry()
        self.tu.connect("changed", self.checkfields)
        self.tu.show()
        nethbox.pack_start(self.tu, expand=True, fill=True, padding=0)

        lp = gtk.Label("Password:")
        lp.show()
        nethbox.pack_start(lp, expand=False, fill=True, padding=0)

        self.tp = gtk.Entry()
        self.tp.connect("changed", self.checkfields)
        self.tp.show()
        nethbox.pack_start(self.tp, expand=True, fill=True, padding=0)

        self.netbut = gtk.Button("Download")
        self.netbut.connect("clicked", self.download)
        self.netbut.set_sensitive(False)
        self.netbut.show()
        nethbox.pack_start(self.netbut, expand=False, fill=True, padding=0)

        nethbox.show()
        vbox.pack_start(nethbox, expand=False, fill=True, padding=0)
        vbox.show()
        self.window.add(vbox)
        self.window.show()

    def main(self):
        gtk.main()

if __name__ == "__main__":
    sl = SourceLoader()
    sl.main()
    print sl.success
