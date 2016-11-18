#!/usr/bin/env python3
import sys
import os
import json
import urllib.request

url = json.loads(open(sys.argv[1]).read())
dirName = (os.path.dirname(sys.argv[1]))
baseName = (os.path.basename(sys.argv[1]))
outputFile = dirName + "/out_" + str(baseName)
print (url["image_url"])

urllib.request.urlretrieve(url["image_url"], "downloaded.jpg")
cmd = "tesseract downloaded.jpg output -psm 6"

from PIL import Image
import pytesseract
im = Image.open("downloaded.jpg")
text = pytesseract.image_to_string(im, config="-psm 6")
print (text)
dictionary = {"text": text}
with open(outputFile, "w") as json_file:
    json.dump(dictionary, json_file)
