#!/usr/bin/env python
import sys, csv

if(sys.argv.__len__() < 4):
    print "error usage is:"
    print sys.argv[0],"partlistin SR-certified outfile"
    sys.exit(-1)


inpath = sys.argv[1]
srpath = sys.argv[2]
outpath = sys.argv[3]

infile = open(inpath,"r") # eagle bom file
srfile = open(srpath,"r") # csv listing SR approved parts

lib ={}
bom={}
fail = 0

sr_csv = csv.DictReader( srfile )

for line in sr_csv:
    if line["sr-code"].strip()[0] == "#":
        continue
    c = line["sr-code"]
    # Remove the code from the dictionary
    del line["sr-code"]

    lib[c] = line

# EAGLE specific stuff
for i in range(8): # skip eagle header
    infile.readline()

error = 0
found = 0

for line in infile:
    fields = line.split()
    sr_id = fields[1] # eg SRcr10k
    board_id = fields[0] # eg R1

    if ( not (lib.has_key(sr_id))):
        print "Error, ",board_id," ",sr_id, "Is not a valid SR component, please re-align your mind"
        fail = 1
        error = error+1
    else:
        #print sr_id ,"ok", line
        bom.setdefault(sr_id,[])
        bom[sr_id].append(board_id)
        found = found +1

# for i in bom:
#     print i, bom[i]

print found," Parts found in ",srpath
if (fail == 1):
    print "Fail, ",error," parts not identified"
    sys.exit(-2)


