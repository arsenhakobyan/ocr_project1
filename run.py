#!/usr/bin/env python3
import sys
import os
import json
import subprocess

url = json.loads(open(sys.argv[1]).read())
dirName = (os.path.dirname(sys.argv[1]))
baseName = (os.path.basename(sys.argv[1]))
outputFile = dirName + "/out_" + str(baseName)
print (url["image_url"])
tmp = "downloaded.jpg"

cmd1 = 'wget -O {0} {1}'.format(tmp, url["image_url"])
subprocess.call(cmd1.split(), shell=False)
cmd2 = './ocr {0}'.format(tmp)
subprocess.call(cmd2.split(), shell=False)
cmd3 = 'cp {0} {1}'.format("output.json", outputFile)
subprocess.call(cmd3.split(), shell=False)

cmd4 = 'tesseract {0} out -psm 6'.format(tmp)
subprocess.call(cmd4.split(), shell=False)
outFile = open("out.txt", 'r')
text = outFile.read()
outText = json.loads(open(outputFile).read())
outText["text"].append(text)

with open(outputFile, "w") as json_file:
        json.dump(outText, json_file, indent=4)
