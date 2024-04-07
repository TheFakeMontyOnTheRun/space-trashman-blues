#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cstring>

#include <vector>

#include <iostream>

#include "xml.h"

static void findPath(xmlNode *node, bool triangulate) {
    xmlNode *currentNode = NULL;

    for (currentNode = node; currentNode; currentNode = currentNode->next) {
        if (currentNode->type == XML_ELEMENT_NODE) {
            if (!std::strcmp((const char *) currentNode->name, "path")) {
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
        }
        findPath(currentNode->children, triangulate);
    }
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

    findPath(root, triangulate);

    xmlFreeDoc(doc);
}