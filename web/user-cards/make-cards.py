#!/bin/env python
import csv, sys, sr

X_DELTA = 3.54
Y_DELTA = 2.04

def header():
    print """\\documentclass{article}

\\newcommand{\\verticalcards}{5}

% LaTeX manual page 163
\\textwidth 22.00cm % 21.59cm
\\textheight 27.94cm
% Adjust topmargin to fit both 8-1/2x11 and A4 paper:
\\topmargin -1.25in % was -1.0

\\headheight 0in
\\headsep 0in
\\oddsidemargin -0.5in % ok fits A4 paper

\\usepackage{graphics}

\\pagestyle{empty} % removes page numbers
\\begin{document}
\\noindent
\\setlength{\\unitlength}{1in}"""

def page_header():
    print """\\begin{picture}(8.5,11)(0.0,0.3937) % revised for 5 vertical per page
  \\thicklines """

def page_footer():
    print """\\end{picture}\clearpage"""

def footer():
    print """\\end{document}"""

def output(x,y,info):
    user = info["user"]
    pw = info["password"]
    print "\\put(%f,%f) {" % ( x * X_DELTA,
                              y * Y_DELTA )

    print """\\framebox(3.5,2)[c]{"""
    print """\\shortstack[l]{"""
    print "\\textbf{Student Robotics Login Details}\\\\"
    print "\\vspace{3pt} \\\\"

    print "%s %s\\\\" % ( user.cname, user.sname )
    print "Username: \\texttt{%s} \\\\" % user.username
    print "Password: \\texttt{%s}" % pw
    print "\\vspace{3pt} \\\\"
    print "Use these details on the Student Robotics website: \\\\"
    print "\\texttt{http://www.studentrobotics.org/}"
    print "\\vspace{6pt} \\\\"
    print "Where you will find the IDE, forums, "
    print "tutorials and \\\\ documentation, and the  "
    print "ability to change your \\\\ password."

    print """} % end shortstack"""
    print """} % end makebox
  }"""

users = []

r = csv.reader( open(sys.argv[1], "r") )
for row in r:
    uname = row[5].strip()
    pw = row[6].strip()

    user = sr.user( uname )

    users.append( { "user":user, "password":pw } ) 
#     { "fname" : ,
#       "lname" : "Bennellick",
#       "username":"tbennellick",
#       "password":"0hjsd-0js",
#       "email":""} )

header()

x = 0
y = 0
COLS = 2
ROWS = 5

for user in users:
    if x == 0 and y == 0:
        page_header()

    output(x,y,user)

    x = x + 1
    if x == COLS:
        x = 0
        y = y + 1

        if y == ROWS:
            y = 0
            page_footer()

page_footer()


footer()

