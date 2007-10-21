import urllib2
import tempfile
import threading

class BGDownloader(threading.Thread):
    def __init__(self, username, password, q):
        super(BGDownloader, self).__init__()
        self.username = username
        self.password = password
        self.q = q

    def run(self):
        theurl = 'www.studentrobotics.org/~stephen/ide/checkout'
        protocol = 'http://'

        try:
            authhandler = urllib2.HTTPBasicAuthHandler()
            authhandler.add_password("Testing login", "www.studentrobotics.org",
                                    self.username, self.password)

            opener = urllib2.build_opener(authhandler)

            pagehandle = opener.open(protocol + theurl)
            tmpname = tempfile.mktemp()
            tmp = open(tmpname, "wb")
            tmp.write(pagehandle.read())
            tmp.close()
            self.q.put(tmpname)
        except:
            self.q.put("")

