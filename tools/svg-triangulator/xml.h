#ifndef XML_H
#define XML_H


struct Vec {
    float x, y;
};

struct RGB {
    int r, g, b;
};

struct Graphic {
    std::vector <Vec> points;
    RGB colour;
};

void openXML(const char* filename, bool triangulate);

std::vector <Graphic> splitIntoMonotones(std::vector <Graphic> polytonics);

struct Graphic parsePath(const char* pathStr, float parentTransformX, float parentTransformY);

struct RGB parseStyle(const char* styleStr);

std::string to_string(const RGB &c);

std::string to_string(const Vec &v);

std::string to_string(const Graphic &g);

RGB handleColour(const std::string &colour);

std::string to_string(const RGB &c);

std::string to_string(const Vec &v);

std::string to_string(const Graphic &g);

RGB handleColour(const std::string &colour);

Graphic parsePath(const char *pathStr, float parentTransformX, float parentTransformY);

RGB parseStyle(const char* styleStr);
#endif