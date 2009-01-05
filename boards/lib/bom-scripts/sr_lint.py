#!/usr/bin/env python
import sys, parts_db

if(sys.argv.__len__() < 4):
    print "Usage is:"
    print sys.argv[0],"eagle-partlistin parts-db outfile"
    print "Check all components in a parts list exist."
    sys.exit(1)

inpath = sys.argv[1]
srpath = sys.argv[2]
outpath = sys.argv[3]

lib = parts_db.Db(srpath)
infile = open(inpath,"r") # eagle bom file

bom = {}
fail = 0

# skip eagle header
for i in range(8):
    l = infile.readline()
    # line 5 starts with "EAGLE" in EAGLE parts lists.
    if i == 4 and l[0:5] != "EAGLE":
        print "Error: Parts list is not EAGLE parts list."
        sys.exit(3)
        
error = 0
found = 0

for line in infile:
    fields = line.split()
    sr_id = fields[1].lower() # eg SRcr10k
    board_id = fields[0].strip() # eg R1

    if ( not (lib.has_key(sr_id))):
        print "Error (%s): '%s' not in SR component database, please re-align your mind" % (board_id, sr_id)
        fail = 1
        error = error+1
    else:
        #print sr_id ,"ok", line
        bom.setdefault(sr_id,[])
        bom[sr_id].append(board_id)
        found = found +1

print found,"parts found in",srpath
if (fail == 1):
    print "Fail:", error, "parts not identified"
    sys.exit(2)


