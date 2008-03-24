import urllib2
import tempfile
import threading
import logging

class BGDownloader(threading.Thread):
    def __init__(self, username, password, q):
        super(BGDownloader, self).__init__()
        self.username = username
        self.password = password
        self.q = q

    def run(self):
        logging.debug("Starting download")
        theurl = 'ide.studentrobotics.org/checkout?simulator=True&team=20'
        protocol = 'http://'

        try:
            authhandler = urllib2.HTTPBasicAuthHandler()
            authhandler.add_password("Student Robotics IDE", "ide.studentrobotics.org",
                                    self.username, self.password)

            opener = urllib2.build_opener(authhandler)

            logging.debug("Downloading file")
            pagehandle = opener.open(protocol + theurl)
            tmpname = tempfile.mktemp()
            logging.debug("Connection opened. Writing to %s" % tmpname)
            tmp = open(tmpname, "wb")
            tmp.write(pagehandle.read())
            tmp.close()
            logging.debug("%s closed" % tmpname)
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
