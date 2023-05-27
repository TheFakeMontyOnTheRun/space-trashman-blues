#!/usr/bin/env python

import glob
import sys

textureList = []

def getTextureList(path):
    print( "get texture list: " + str(path) )

    f = open(str(path))

    for line in f:
        textureName = "src/" + str(line.replace("\n", "")).replace(".img", ".png")
        textureList.append(textureName)

    f.close()


def cleanup():

    texturesNotFound = []
    existingTextures = glob.glob('src/*.png')

    for textureName in textureList:
        if textureName not in existingTextures:
            print("Texture not found " + str(textureName))
            texturesNotFound.append(textureName)

    if len(texturesNotFound ) > 0:
        sys.exit(1)

def checkMap(textureListPath):
    global textureList
    textureList = []
    getTextureList(textureListPath)
    cleanup()
    print ("--OK!--")

if __name__ == "__main__":
    if len(sys.argv) == 1:
        for i in range(0, 24):
            iAsString = str(i)
            checkMap("assets/tiles" + iAsString + ".lst")
    else:
        if len(sys.argv) == 5:
            checkMap( sys.argv[1])
        else:
            print( "parms [textures]" )

