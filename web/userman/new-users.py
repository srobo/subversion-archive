#!/bin/env python
import sr, csv, mailer

def username(fname,lname):
    s = "%s%s" % (fname[0], lname)
    return s.lower()

def user_exists(uname):
    u = sr.user(uname)
    return u.in_db

r = csv.reader( open("users2010.csv", "r") )
w = csv.writer( open("users2010-assigned.csv", "w+") )

teachers = sr.group("teachers")
students = sr.group("students")

for row in r:
    # Ignore comment lines
    if len(row) < 3 or row[0].strip()[0] == "#":
        continue

    fname = row[2].strip()
    lname = row[3].strip()
    email = row[4].strip().lower()
    college_num = int(row[1].strip())

    # Decode the teacher field ("yes" => True)
    tstr = row[0].strip().lower()
    if len(tstr) > 0 and tstr[0] == "y":
        teacher = True
    else:
        teacher = False

    unum = 1
    uname = username(fname,lname)

    while user_exists(uname):
        unum = unum + 1
        uname = "%s%i" % (username(fname, lname), unum)

    print """ college:%i uname:"%s" fname:"%s" lname:"%s" email:"%s" teacher:"%s" """ % ( college_num, uname, fname, lname, email, teacher )

    user = sr.user(uname)
    user.cname = fname
    user.sname = lname
    user.email = email
    user.save()

    pw = sr.users.GenPasswd()
    user.set_passwd( new = pw )

    print "Created/updated user %s" % (uname)

    if teacher:
        teachers.user_add( user )
        teachers.save()
    else:
        students.user_add( user )
        students.save()

    # Add user to the temporary team:
    tmpteam = sr.group( "team%i" % (2000 + college_num) )
    tmpteam.user_add( user )
    tmpteam.save()

    colg = sr.group("college-%i" % college_num)
    colg.user_add( user )
    colg.save()

    mailer.email_pass( user, pw )

    w.writerow( [ fname, lname, email, college_num, tstr, uname, pw ] )
