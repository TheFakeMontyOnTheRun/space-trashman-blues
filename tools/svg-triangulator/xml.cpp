#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cstring>

#include <vector>

#include <iostream>

#include "xml.h"

static void handlePath(xmlNode* currentNode, bool triangulate) {
    Graphic g = parsePath((const char *) xmlGetProp(currentNode, (const xmlChar *) "d"));
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

static void handleGroup(xmlNode *node, bool triangulate) {
    xmlNode *currentNode = NULL;
    if (xmlGetProp(node, (const xmlChar *) "id") != NULL) {
        std::cout << "<g id=\"" << ((const char *) xmlGetProp(node, (const xmlChar *) "id")) << ">\n";
    } else {
        std::cout << "<g>\n";
    }

    if (node->children) {
        for (currentNode = node->children; currentNode; currentNode = currentNode->next) {
            if (currentNode->type == XML_ELEMENT_NODE) {

                if (!std::strcmp((const char *) currentNode->name, "path")) {
                    handlePath(currentNode, triangulate);
                }

                if (!std::strcmp((const char *) currentNode->name, "g")) {
                    handleGroup(currentNode, triangulate);
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

    handleGroup(root, triangulate);

    xmlFreeDoc(doc);
}