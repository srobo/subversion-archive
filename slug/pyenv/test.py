import sys
import select
import os
import subprocess

sp = subprocess.Popen("./testcam", bufsize = 1, stdout = subprocess.PIPE)

fifo = sp.stdout.fileno()

text = ""

while 1:
    if select.select([fifo], [], [], 0) == ([], [], []):
        continue
    print os.read(fifo, 1),
