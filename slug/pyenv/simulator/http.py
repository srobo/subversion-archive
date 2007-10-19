import urllib2
import tempfile

def checkout():
    theurl = 'www.studentrobotics.org/~stephen/ide/checkout'
    protocol = 'http://'
    username = 'stephen'
    password = 'abc'

    authhandler = urllib2.HTTPBasicAuthHandler()
    authhandler.add_password("Testing login", "www.studentrobotics.org", username, password)

    opener = urllib2.build_opener(authhandler)

    pagehandle = opener.open(protocol + theurl)
    tmpname = tempfile.mktemp()
    tmp = open(tmpname, "wb")
    tmp.write(pagehandle.read())
    tmp.close()
    return tmpname
