#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cstring>

#include <vector>

#include <iostream>

#include "xml.h"

static void handlePath(xmlNode* currentNode, bool triangulate, float parentTransformX, float parentTransformY) {
    Graphic g = parsePath((const char *) xmlGetProp(currentNode, (const xmlChar *) "d"), parentTransformX, parentTransformY);
    RGB colour = parseStyle((const char *) xmlGetProp(currentNode, (const xmlChar *) "style"));
    g.colour = colour;


    if (triangulate) {
        std::vector <Graphic> input;
        input.push_back(g);

        auto triangulated = splitIntoMonotones(input);
        for (const auto &t: triangulated) {
            std::cout << to_string(t);
        }

    } else {
        std::cout << to_string(g);
    }
}

static void handleGroup(xmlNode *node, bool triangulate, float parentTransformX, float parentTransformY) {
    xmlNode *currentNode = NULL;
    if (xmlGetProp(node, (const xmlChar *) "id") != NULL) {
        std::cout << "<g id=\"" << ((const char *) xmlGetProp(node, (const xmlChar *) "id")) << "\">\n";
    } else {
        std::cout << "<g>\n";
    }

    if (((const char *) xmlGetProp(node, (const xmlChar *) "transform")) != NULL) {
        std::string toParse = ((const char *) xmlGetProp(node, (const xmlChar *) "transform"));

        if (toParse.find("matrix") != std::string::npos) {
            std::cout << "\n\nmatrix transforms are not currently supported\n\n" << std::endl;
            exit(-1);
        }

        if (toParse.find("translate") != std::string::npos) {
            auto parmBegin = toParse.find("translate");


            toParse = toParse.substr(parmBegin + 10);

            auto parmEnd = toParse.find(",");

            parentTransformX += atof(toParse.substr(0, parmEnd).c_str());
            toParse = toParse.substr(parmEnd + 1);
            parmEnd = toParse.find(")");
            parentTransformY += atof(toParse.substr(0, parmEnd).c_str());
        }
    }


    if (node->children) {
        for (currentNode = node->children; currentNode; currentNode = currentNode->next) {
            if (currentNode->type == XML_ELEMENT_NODE) {

                if (!std::strcmp((const char *) currentNode->name, "path")) {
                    handlePath(currentNode, triangulate, parentTransformX, parentTransformY);
                }

                if (!std::strcmp((const char *) currentNode->name, "g")) {
                    handleGroup(currentNode, triangulate, parentTransformX, parentTransformY);
                }

            }
        }
    }

    std::cout << "</g>" << std::endl;
}

void openXML(const char *filename, bool triangulate) {
    xmlDoc *doc = NULL;
    xmlNode *root = NULL;

    LIBXML_TEST_VERSION

    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL) {
        printf("error: could not parse file %s\n", filename);
    }

    root = xmlDocGetRootElement(doc);

    handleGroup(root, triangulate, 0, 0);

    xmlFreeDoc(doc);
}