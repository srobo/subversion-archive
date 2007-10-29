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
        theurl = 'ide.studentrobotics.org/checkout'
        protocol = 'http://'

        try:
            authhandler = urllib2.HTTPBasicAuthHandler()
            authhandler.add_password("Student Robotics IDE", "ide.studentrobotics.org",
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


if __name__ == "__main__":
    from getpass import getpass
    import Queue
    q = Queue.Queue()
    bg = BGDownloader(raw_input("Username: "), getpass("Password: "), q)
    bg.start()
    bg.join()
    print q.get()
