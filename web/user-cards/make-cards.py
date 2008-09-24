#!/bin/env python
import csv

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
\\setlength{\\unitlength}{1in}
\\begin{picture}(8.5,11)(0.0,0.3937) % revised for 5 vertical per page
  \\thicklines
"""


def footer():
    print """\\end{picture}
\\end{document}
"""

def output(x,y,contents):
    print "\\put(%f,%f) {" % ( x * X_DELTA,
                              y * Y_DELTA )

    print """\\framebox(3.5,2){ % x,y size of box, inches
      \\shortstack[l]{"""
    print contents
    print """} % end shortstack
    } % end makebox
  }"""

users = []

users.append( {"username":"rspanton",
               "password":"dfhgdfgo",
               "email":""} )

header()

for x in [0,1]:
    for y in xrange(0,4):
        output(x,y,"badger")


footer()

