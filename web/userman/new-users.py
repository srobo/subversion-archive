#!/bin/env python
import sr, csv

def username(fname,lname):
    return "%s%s" % (fname[0], lname)

def user_exists(uname):
    u = sr.user(uname)
    return u.in_db

r = csv.reader( open("users.csv", "r") )
w = csv.writer( open("users-assigned.csv", "w+") )

teachers = sr.group("teachers")
students = sr.group("students")

for row in r:
    # Ignore comment lines
    if len(row) < 3 or row[0].strip()[0] == "#":
        continue

    fname = row[0].strip()
    lname = row[1].strip()
    email = row[2].strip()
    college_num = int(row[3].strip())

    # Decode the teacher field ("yes" => True)
    tstr = row[5].strip().lower()
    if len(tstr) > 0 and tstr[0] == "y":
        teacher = True
    else:
        teacher = False

    if row[4].strip() != "":
        uname = row[4].strip()
    else:
        unum = 1
        uname = username(fname,lname)

        while user_exists(uname):
            unum = unum + 1
            uname = "%s%i" % (username(fname, lname), unum)

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

    colg = sr.group("college-%i" % college_num)
    colg.user_add( user )
    colg.save()

    w.writerow( [ fname, lname, email, college_num, tstr, uname, pw ] )


