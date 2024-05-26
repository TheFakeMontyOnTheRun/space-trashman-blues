#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

#include "xml.h"

std::string to_string(const RGB &c) {
    std::stringstream ss;

    ss << "fill:#";
    ss << std::setw(2) << std::setfill('0') << std::hex << c.r;
    ss << std::setw(2) << std::setfill('0') << std::hex << c.g;
    ss << std::setw(2) << std::setfill('0') << std::hex << c.b;
    return ss.str();
}

std::string to_string(const Vec &v) {
    std::stringstream ss;
    ss << static_cast<int>(v.x);
    ss << ",";
    ss << static_cast<int>(v.y);
    ss << " ";
    return ss.str();
}

std::string to_string(const Graphic &g) {
    std::stringstream ss;
    ss << "<path " << "\nstyle=\"";
    ss << to_string(g.colour);
    ss << "\"\nd=\"M ";
    for (const auto &v: g.points) {
        ss << to_string(v);
    }
    ss << "Z\"\n />\n";

    return ss.str();
}

RGB handleColour(const std::string &colour) {
    std::stringstream ss;
    RGB toReturn;

    ss << colour.substr(0, 2);
    ss >> std::hex >> toReturn.r;
    ss.clear();

    ss << colour.substr(2, 2);
    ss >> std::hex >> toReturn.g;
    ss.clear();

    ss << colour.substr(4, 2);
    ss >> std::hex >> toReturn.b;
    ss.clear();


    return toReturn;
}

 Graphic parsePath(const char *pathStr, float parentTransformX, float parentTransformY) {
    std::string path = pathStr;

    Graphic toReturn;
    bool absolute = false;
    std::stringstream ss(path);
    std::vector <std::string> tokens;
    char cmd;
    float lastX, lastY;
    std::string s;
    int p, end;
    lastX = lastY = 0;

    while (getline(ss, s, ' ')) {
        tokens.push_back(s);
    }

    end = tokens.size();

    p = 0;

    while (p < end) {
        Vec v;

        if (tokens[p] == "Z") {
            absolute = true;
            ++p;
            continue;
        }

        if (tokens[p] == "z") {
            absolute = false;
            ++p;
            continue;
        }

        if (tokens[p] == "M") {
            absolute = true;
            ++p;
            continue;
        }

        if (tokens[p] == "H") {
            absolute = true;
            ++p;
            continue;
        }

        if (tokens[p] == "V") {
            absolute = true;
            ++p;
            continue;
        }

        if (tokens[p] == "v") {
            absolute = false;
            ++p;
            continue;
        }

        if (tokens[p] == "h") {
            absolute = false;
            ++p;
            continue;
        }

        if (tokens[p] == "m") {
            absolute = false;
            ++p;
            continue;
        }

        if (tokens[p] == "C") {
            absolute = true;
            ++p;
            continue;
        }

        if (tokens[p] == "c") {
            absolute = false;
            ++p;
            continue;
        }

        if (tokens[p] == "L") {
            absolute = true;
            ++p;
            continue;
        }

        if (tokens[p] == "l") {
            absolute = false;
            ++p;
            continue;
        }
        std::string token = tokens[p];
        auto commaPos = token.find(",");

        if (commaPos == std::string::npos) {
            v.x = std::atof(tokens[p++].c_str())  + parentTransformX;
            v.y = std::atof(tokens[p++].c_str())  + parentTransformY;
        } else {
            std::string part1 = token.substr(0, commaPos);
            std::string part2 = token.substr(commaPos + 1);
            v.x = std::atof(part1.c_str())  + parentTransformX;
            v.y = std::atof(part2.c_str())  + parentTransformY;
            p++;
        }

        if (!absolute) {
            v.x += lastX;
            v.y += lastY;
        }

        toReturn.points.push_back(v);

        lastX = v.x;
        lastY = v.y;
    }

    return toReturn;
}

RGB parseStyle(const char* styleStr) {
    std::string style = styleStr;
    int pos = 0;
    int end = style.length();
    while (pos <= end) {
        auto fillPos = style.find("fill:");
        if (fillPos != std::string::npos) {
            auto colour = style.substr(fillPos + 6, 6);
            //tmp;
            return handleColour(colour);
        }
    }
    return {};
}
