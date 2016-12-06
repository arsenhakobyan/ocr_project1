#!/usr/bin/env python3
import sys
import os
import json
import subprocess
from subprocess import Popen, PIPE, STDOUT
import binascii


url = json.loads(open(sys.argv[1]).read())
dirName = (os.path.dirname(sys.argv[1]))
baseName = (os.path.basename(sys.argv[1]))
outputFile = dirName + "/out_" + str(baseName)
#print (url["image_url"])
tmp = "downloaded.jpg"

cmd1 = 'wget -O {0} {1}'.format(tmp, url["image_url"])
subprocess.call(cmd1.split(), shell=False, stdout=PIPE, stderr=STDOUT, close_fds=True)
print ("Downloaded the image.")
cmd2 = './ocr {0}'.format(tmp)
subprocess.call(cmd2.split(), shell=False, stdout=PIPE, stderr=STDOUT, close_fds=True)
print ("Applied custom text detection and OCR to the image.")
cmd3 = 'cp {0} {1}'.format("output.json", outputFile)
subprocess.call(cmd3.split(), shell=False, stdout=PIPE, stderr=STDOUT, close_fds=True)

cmd4 = 'tesseract {0} out -psm 6'.format(tmp)
subprocess.call(cmd4.split(), shell=False, stdout=PIPE, stderr=STDOUT, close_fds=True)
print ("Applied Tesseract-OCR to the image.")
outFile = open("out.txt", 'r')
text = outFile.read()
t = text.encode('ascii', 'ignore')
m = str(t).replace("\\n", " ")
outText = json.loads(open(outputFile).read())
outText["text"].append(str(m))
with open(outputFile, "w") as json_file:
        json.dump(outText, json_file, indent=4)
print ("The Combined result has been written in the " + str(outputFile) + " file.")
