#!/usr/bin/env python3
import sys
import os
import json
import urllib.request

url = json.loads(open(sys.argv[1]).read())
print (url["image_url"])

urllib.request.urlretrieve(url["image_url"], "downloaded.jpg")
cmd = "tesseract downloaded.jpg output -psm 6"
os.system(cmd)
f = open('output.txt', 'r')
print (f.read())
