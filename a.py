#!/usr/bin/env python3
import cv2
import sys
import os

image = cv2.imread(sys.argv[1])
w = image.shape[0]
h = image.shape[1]
gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY) # grayscale
laplacian = cv2.Laplacian(gray, cv2.CV_8UC1)
cv2.imshow("laplacian", laplacian) 
ret2, thresh = cv2.threshold(laplacian, 150, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
#thresh = cv2.adaptiveThreshold(laplacian, 255, cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY, 11, 2)
cv2.imshow("thresh", thresh) 
dilate_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (int(0.15 * w), int(0.023 * h)))
dilated = cv2.dilate(thresh, dilate_kernel) # dilate
cv2.imshow("dilated", dilated) 
erode_kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (int(0.01 * w), int(0.011 * h)))
eroded = cv2.erode(dilated,erode_kernel) # erode
cv2.imshow("eroded", eroded) 
im, contours, hierarchy = cv2.findContours(eroded, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE) # get contours

# for each contour found, draw a rectangle around it on original image
text = ""
for contour in contours:
    # get rectangle bounding contour
    [x,y,w,h] = cv2.boundingRect(contour)

    ## discard areas that are too large // Not needed for now
    #if h>300 and w>300:
    #    continue

    ## discard areas that are too small // Not needed for now
    #if h<40 or w<40:
    #    continue

    # draw rectangle around contour on original image
    cv2.rectangle(image,(x,y),(x+w,y+h),(255,0,255),2)
    im  = image[y:y+h-1,x:x+w-1]
    cv2.imwrite("a.jpg", im)
    cmd = "tesseract a.jpg output -psm 6"
    os.system(cmd)
    f = open('output.txt', 'r')
    #print (f.read())
    text += (f.read())

print(text)

# write original image with added contours to disk  
cv2.imshow("contoured.jpg", image) 
cv2.waitKey(0)
