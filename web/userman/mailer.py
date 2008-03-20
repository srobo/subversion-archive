#!/bin/env python
import smtplib, getpass
import sr

fromaddr = "rspanton@studentrobotics.org"
#toaddr = "rspanton@gmail.com"
smtpserver = "ugsmtp.ecs.soton.ac.uk"
username = "rds204"
subject = "Welcome to Student Robotics " 

#smtp_pass = getpass.getpass("SMTP Server password:")

def def_psource():
    return getpass.getpass("SMTP Server password:")

psource = def_psource

def set_psource( fn ):
    global psource
    psource = fn

the_pass = None
def get_pass():
    global the_pass
    if the_pass == None:
        the_pass = psource()
    return the_pass

def email( fromaddr, toaddr, subject, msg, smtp_pass = None ):
    if smtp_pass == None:
        smtp_pass = get_pass()

    msg = "From: %s\r\nTo: %s\r\nSubject: %s\r\n\r\n%s" % (fromaddr, toaddr, subject, msg)

    server = smtplib.SMTP(smtpserver)
    server.ehlo()
    server.starttls()
    server.ehlo()
    server.login(username, smtp_pass)
    print server.sendmail(fromaddr, toaddr, msg)
    try:
        server.quit()
    except smtplib.sslerror:
        pass

def email_pass( user,p, smtp_pass = None ):
    msg = """Hello %s,

You now have a shiny new Student Robotics account.  This will let you
access the Student Robotics facilities, including the forums and the
software development environment for your robot.

Your username is: %s
Your password is: %s

Your registered email address is %s.
If you want to change this, or you have any problems with your account
please get in touch - by emailing accounts@studentrobotics.org.

You'll almost certainly want to change your password.  You can do this
at http://studentrobotics.org/passwd/

Please introduce yourself on the forums by posting a message to the
'Introduce Yourself' forum.  You can find the forums on the website
(http://www.studentrobotics.org).  We hope that you will use these
forums for discussing the competition with other teams.  There will be
a prize for the team that best participates in the online community.

Please note that the contents of these forums are monitored by Student
Robotics mentors and school teachers and that all posts are associated
with your username.

You will soon have a fantastic new simulator so that you can
start practising your coding before you get the electronics kit.  The
electronics kit is on schedule to be available at the end of November.

Thanks,

Rob Spanton

---

Student Robotics President
rspanton@studentrobotics.org
""" % ( user.cname, user.username, p, user.email )

    email( fromaddr, user.email, subject, msg, smtp_pass )

