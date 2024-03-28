#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

struct Vec {
    int x, y;
};

struct RGB {
    int r, g, b;
};

struct Graphic {
    std::vector <Vec> points;
    RGB colour;
};

std::string to_string(const RGB &c) {
    std::stringstream ss;

    ss << c.r;
    ss << ", ";
    ss << c.g;
    ss << ", ";
    ss << c.b;
    ss << ",\n";
    return ss.str();
}

std::string to_string(const Vec &v) {
    std::stringstream ss;
    ss << static_cast<int>((v.x * 127.0f) / 800.0f);
    ss << ", ";
    ss << static_cast<int>((v.y * 127.0f) / 480.0f);
    ss << ",\n";
    return ss.str();
}

std::string to_string(const Graphic &g) {
    std::stringstream ss;
    ss << g.points.size() << ",\n";
    ss << "#ifndef MONOCHROME_VECTORS\n";
    ss << to_string(g.colour);
    ss << "#endif\n";
    for (const auto &v: g.points) {
        ss << to_string(v);
    }

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

Graphic parsePath(const std::string &path) {
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
            v.x = std::atof(tokens[p++].c_str());
            v.y = std::atof(tokens[p++].c_str());
        } else {
            std::string part1 = token.substr(0, commaPos);
            std::string part2 = token.substr(commaPos + 1);
            v.x = std::atof(part1.c_str());
            v.y = std::atof(part2.c_str());
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

RGB handleStyle(const std::string &style) {

    int pos = 0;
    int end = style.length();
    while (pos <= end) {
        auto fillPos = style.find("fill:");
        if (fillPos == 0) {
            auto colour = style.substr(6);
            //tmp;
            return handleColour(colour);
        }
    }

    return {};
}

void handlePathNode(std::ifstream &svg) {

    RGB colour;
    bool inPath = false;
    std::stringstream currentPath;

    while (svg.good()) {
        std::string line;

        std::getline(svg, line);

        if (line.find("/>") != std::string::npos) {
            return;
        }


        auto style = line.find("style=");
        if (style != std::string::npos) {
            line = line.substr(style + 7);
            colour = handleStyle(line.substr(0, line.length() - 1));
            continue;
        }

        if (!inPath) {

            auto pathStart = line.find("d=");
            if (pathStart == 0 || (pathStart != std::string::npos && line[pathStart - 1] == ' ')) {
                line = line.substr(pathStart + 3);
                inPath = true;
                currentPath.clear();

                auto pathEnd = line.find("\"");

                if (pathEnd != std::string::npos) {
                    inPath = false;
                    line = line.substr(0, pathEnd - 1);
                    Graphic g;
                    g = parsePath(line);
                    g.colour = colour;

                    std::cout << to_string(g);

                } else {
                    currentPath << line;
                }
            }
        } else {
            auto pathEnd = line.find("\"");

            if (pathEnd != std::string::npos) {
                inPath = false;
                currentPath << line.substr(0, pathEnd - 1);
                Graphic g;
                g = parsePath(currentPath.str());
                g.colour = colour;
                std::cout << to_string(g);
            } else {
                currentPath << line;
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cout << "USAGE: distiller [svg]" << std::endl;
        return 0;
    }

    std::string filename = argv[1];

    auto extensionPos = filename.rfind(".svg");
    if ( extensionPos > filename.length() ) {
        std::cout << "/* " << filename << " is not an SVG file! */" << std::endl;
        return 0;
    }

    filename = filename.substr(0, extensionPos);

    for (auto& c : filename) {
        if ( c == '.' || c == '-' || c == '/' ) {
            c = '_';
        }
    }

    std::ifstream svg(argv[1]);

    std::cout << "static const uint8_t " << filename << "[] = {" << std::endl;

    while (svg.good()) {
        std::string line;

        std::getline(svg, line);

        auto pathStart = line.find("<path");
        if (pathStart != std::string::npos) {
            handlePathNode(svg);
        }
    }

    std::cout << "0};" << std::endl;

    return 0;
}