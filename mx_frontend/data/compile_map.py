#!/usr/bin/env python

import sys

textureList = []

textureIndices = {
    "null" : 0xFF
}
tilesProperties = {}

geometryIndex = {
    "null" : 0,
    "cube": 1,
    "leftnear": 2,
    "leftfar": 3,
    "floor": 4,
    "rampnorth": 5,
    "rampeast": 6,
    "rampsouth": 7,
    "rampwest": 8
}

class Properties:
    needsAlpha = False
    blocksVisibility = False
    blocksMovement = False
    blocksEnemySight = False
    repeatMainTexture = False
    ceilingTexture = ""
    floorTexture = ""
    mainTexture = ""
    geometryType = ""
    ceilingRepeatTexture = ""
    floorRepeatTexture = ""
    ceilingRepeats= ""
    floorRepeats = ""
    ceilingHeight = ""
    floorHeight = ""


def writeByte( file, value ):
    file.write(bytearray([int(value)]))


def writeRawByte( file, value ):
    file.write(bytearray([value]))


def writeAsFixedPoint(file, value):

    valueAsInt = int( float( value ) * float(0xFFFF) )
    writeByte(file, ( valueAsInt & 0x000000FF) >> 0 )
    writeByte(file, ( valueAsInt & 0x0000FF00) >> 8)
    writeByte(file, ( valueAsInt & 0x00FF0000) >> 16)
    writeByte(file, ( valueAsInt & 0xFF000000) >> 24)

def dumpProps(path):
    print "writing to " + str(path)
    f = open(str(path), "w")

    for prop in tilesProperties:
        prp = tilesProperties[prop]
        if prp:
          #  print("prop[" + str(prop) + "]= " + str(tilesProperties[prop]))
            writeRawByte(f, prop)
            writeByte(f, '1' if prp.needsAlpha else '0' )
            writeByte(f, '1' if prp.blocksVisibility else '0' )
            writeByte(f, '1' if prp.blocksMovement else '0' )
            writeByte(f, '1' if prp.blocksEnemySight else '0' )
            writeByte(f, '1' if prp.repeatMainTexture else '0' )
            writeByte(f, textureIndices[prp.ceilingTexture.replace(".img","")] )
            writeByte(f, textureIndices[prp.floorTexture.replace(".img", "") ] )
            writeByte(f, textureIndices[prp.mainTexture.replace(".img", "") ] )
            writeByte(f, geometryIndex[ prp.geometryType ] )
            writeByte(f, textureIndices[
                          prp.ceilingRepeatTexture.replace(".img", "") ] )
            writeByte(f, textureIndices[
                prp.floorRepeatTexture.replace(".img", "") ] )
            writeByte(f, prp.ceilingRepeats )
            writeByte(f, prp.floorRepeats )
            writeAsFixedPoint( f, prp.ceilingHeight )
            writeAsFixedPoint(f, prp.floorHeight )


def parseLine(line):
   # print("{" + str(line) + "}")

    prp = Properties()

    prp.needsAlpha = True if line[1] == '1' else False
    prp.blocksVisibility = True if line[2] == '1' else False
    prp.blocksMovement = True if line[3] == '1' else False
    prp.blocksEnemySight = True if line[4] == '1' else False
    prp.repeatMainTexture = True if line[5] == '1' else False
    prp.ceilingTexture = line[6]
    prp.floorTexture = line[7]
    prp.mainTexture = line[8]
    prp.geometryType = line[9]
    prp.ceilingRepeatTexture = line[10]
    prp.floorRepeatTexture = line[11]
    prp.ceilingRepeats= line[12]
    prp.floorRepeats = line[13]
    prp.ceilingHeight = line[14]
    prp.floorHeight = line[15]

    return prp


def getTextureList(path):
    print( "get texture list: " + str(path) )

    f = open(str(path))

    for line in f:
        textureName = str(line.replace("\n", "")).replace(".img", "")
        #print("{" + textureName + "}")
        textureIndex = len(textureList)
        textureList.append(textureName)
        textureIndices[textureName] = textureIndex

    f.close()


def getProperties(path):
    print( "get properties: " + str(path) )

    f = open(str(path))

    for line in f:
        propertiesLine = str(line.replace("\n", ""))
        tokens = propertiesLine.split(" ")

        if (tokens[0] in tilesProperties.keys()):
            print "already had " + tokens[0]
            sys.exit(1)

        tilesProperties[tokens[0]] = parseLine(tokens)
        #print "got tile for " + str(tokens[0])
    f.close()


def cleanup(path):
    print( "cleanup against map: " + str(path) )
    tokensList = []
    tokensNotFound = []
    f = open(str(path))

    for line in f:
        mapLine = str(line.replace("\n", ""))

        #all the tiles present in a line
        for tile in mapLine:
            if tile not in tokensList:
                tokensList.append(tile)

        #remove all known tiles from the "tiles in the line" list
        for prop in tilesProperties:
            mapLine = mapLine.replace(prop, "")

        #if we got something left, it is not known
        if len(mapLine) > 0:
            print("Tile not found in the properties: " + str(mapLine))
            sys.exit(1)


    for prop in tilesProperties:
        if prop not in tokensList:
            tokensNotFound.append(prop)

    for prop in tokensNotFound:
        print("Property to remove: " + str(prop) )
        tilesProperties[prop] = None


    if len(tokensNotFound ) > 0:
        sys.exit(1)

    f.close()

def compileMap( sourcePath, textureListPath, mapPath, outputPath):
    global textureList
    global textureIndices
    global tilesProperties

    textureList = []
    textureIndices = {"null" : 0xFF}
    tilesProperties = {}
    getTextureList(textureListPath)
    getProperties(sourcePath)
    cleanup(mapPath)
    dumpProps(outputPath)
    print "--done--"

if __name__ == "__main__":
    if len(sys.argv) == 1:
        for i in range(0, 24):
            iAsString = str(i)
            compileMap("src/tiles" + iAsString + ".prp", "assets/tiles" +
                       iAsString + ".lst", "assets/map" + iAsString + ".txt",
                       "assets/props" + iAsString + ".bin")
    else:
        if len(sys.argv) == 5:
            compileMap( sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
        else:
            print( "parms [properties] [textures] [map] [dst]" )

