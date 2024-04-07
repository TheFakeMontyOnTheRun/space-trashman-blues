#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>

#include "xml.h"

std::vector <Graphic> triangulate(Graphic g);


struct ord {
    float v;
    int i;
};


void order(float *xv, int *xi, int length) {

    ord _xi[length];

    for (int f = 0; f < length; ++f) {
        _xi[f] = ord();
        _xi[f].i = xi[f];
        _xi[f].v = xv[f];
    }

    int size = length;
    int menori = 0;
    float menorv = xv[menori];
    ord tmp;

    for (int c = 0; c < size; ++c) {

        menori = c;
        menorv = _xi[menori].v;

        for (int d = c + 1; d < size; ++d) {

            if (menorv > _xi[d].v) {

                menorv = _xi[d].v;
                menori = d;

            }

        }

        tmp = _xi[c];
        _xi[c] = _xi[menori];
        _xi[menori] = tmp;
    }

    for (int e = 0; e < size; ++e) {
        xv[e] = _xi[e].v;
        xi[e] = _xi[e].i;
    }
}

Graphic CullXNoMoreThan(float aSpan, Graphic aSrc) {
    Graphic pa;
    pa.colour = aSrc.colour;

    int qtd = aSrc.points.size();

    if (qtd == 0) {
        return pa;
    }

    bool lastinside = (aSrc.points[qtd - 1].x < aSpan);
    float DX;
    int prev;
    float a2, b1, b2;
    for (int c = 0; c < qtd; c++) {

        if (aSrc.points[c].x < aSpan) {

            if (!lastinside) {

                prev = c - 1;
                while (prev < 0) {
                    prev += qtd;
                }
                prev = prev % qtd;

                a2 = aSrc.points[c].x - aSrc.points[prev].x;
                b1 = aSrc.points[c].x - aSpan;
                b2 = aSrc.points[c].y - aSrc.points[prev].y;
                DX = aSrc.points[c].y - ((b1 * b2) / a2);
                Vec spanVec = {.x = aSpan, .y = DX};
                pa.points.push_back(spanVec);
                lastinside = true;
            }

            Vec tmp = {.x = aSrc.points[c].x, .y = aSrc.points[c].y};
            pa.points.push_back(tmp);
        } else {
            if (lastinside) {
                prev = c - 1;

                while (prev < 0) {
                    prev += qtd;
                }
                prev = prev % qtd;

                a2 = aSrc.points[c].x - aSrc.points[prev].x;
                b1 = aSrc.points[c].x - aSpan;
                b2 = aSrc.points[c].y - aSrc.points[prev].y;
                DX = aSrc.points[c].y - ((b1 * b2) / a2);

                Vec tmp = {.x = (int) aSpan, .y = (int) DX};
                pa.points.push_back(tmp);

                lastinside = false;
            }
        }
    }
    return pa;

}

Graphic CullXNoLessThan(float aSpan, Graphic aSrc) {
    Graphic pa;
    pa.colour = aSrc.colour;

    int qtd = aSrc.points.size();
    if (qtd == 0) {
        return pa;
    }
    bool lastinside = (aSrc.points[qtd - 1].x > aSpan);
    float DX;
    int prev;
    float a2, b1, b2;
    for (int c = 0; c < qtd; c++) {
        if (aSrc.points[c].x > aSpan) {
            if (!lastinside) {

                prev = c - 1;
                while (prev < 0) {
                    prev += qtd;
                }
                prev = prev % qtd;

                a2 = aSrc.points[c].x - aSrc.points[prev].x;
                b1 = aSrc.points[c].x - aSpan;
                b2 = aSrc.points[c].y - aSrc.points[prev].y;
                DX = aSrc.points[c].y - ((b1 * b2) / a2);

                Vec tmp = {.x = (int) aSpan, .y = (int) DX};
                pa.points.push_back(tmp);

                lastinside = true;
            }

            Vec tmp = {.x = aSrc.points[c].x, .y = aSrc.points[c].y};
            pa.points.push_back(tmp);


        } else {
            if (lastinside) {
                prev = c - 1;

                while (prev < 0) {
                    prev += qtd;
                }
                prev = prev % qtd;

                a2 = aSrc.points[c].x - aSrc.points[prev].x;
                b1 = aSrc.points[c].x - aSpan;
                b2 = aSrc.points[c].y - aSrc.points[prev].y;
                DX = aSrc.points[c].y - ((b1 * b2) / a2);

                Vec tmp = {.x = (int) aSpan, .y = (int) DX};
                pa.points.push_back(tmp);

                lastinside = false;
            }
        }
    }
    return pa;

}

// -----------------------------------------------------------------------------
/**
 *
 * @param proc2
 * @return
 */
std::vector <Graphic> decomposeMonotone(Graphic proc) {
    Graphic polygon = proc;

    float v1x;
    float v1y;
    float v2x;
    float v2y;
    float v3x;
    float v3y;
    float vx;
    float vy;
    int c = 0;
    std::vector <Graphic> toReturn;

    while (polygon.points.size() > 4) {
        Graphic p;
        p.colour = polygon.colour;
        c = 0;
        auto v = polygon.points[0];
        v1x = v.x;
        v1y = v.y;
        vx = v1x;
        vy = v1y;

        while (c < polygon.points.size()) {
            v = polygon.points[c];
            vx = v.x;
            vy = v.y;

            if (vx != v1x) {
                break;
            }

            v1x = vx;
            v1y = vy;

            c++;
        }

        Vec tmp = {.x = (int) v1x, .y = (int) v1y};
        p.points.push_back(tmp);


        while (c < polygon.points.size()) {
            v = polygon.points[c];
            vx = v.x;
            vy = v.y;

            if (vx != v1x) {
                break;
            }
            c++;
        }

        v2x = vx;
        v2y = vy;
        v3x = vx;
        v3y = vy;
        {
            Vec tmp = {.x = (int) v2x, .y = (int) v2y};
            p.points.push_back(tmp);
        }
        while (c < polygon.points.size()) {
            v = polygon.points[c];
            vx = v.x;
            vy = v.y;

            if (vx != v2x) {
                break;
            }

            v3x = vx;
            v3y = vy;

            c++;
        }

        {
            Vec tmp = {.x = (int) v3x, .y = (int) v3y};
            p.points.push_back(tmp);
        }
        {
            Vec tmp = {.x = (int) vx, .y = (int) vy};
            p.points.push_back(tmp);
        }

        toReturn.push_back(p);

        p.points.clear();
        p.colour = polygon.colour;

        while (c < polygon.points.size()) {
            v = polygon.points[c];
            vx = v.x;
            vy = v.y;

            {
                Vec tmp = {.x = (int) vx, .y = (int) vy};
                p.points.push_back(tmp);

            }

            c++;
        }

        polygon = p;
    }

    toReturn.push_back(polygon);

    return toReturn;
}

std::vector <Graphic> splitIntoMonotones(std::vector <Graphic> polytonics) {

    std::vector <Graphic> candidate;
    std::vector <Graphic> pols;

    for (Graphic c: polytonics) {
        pols.push_back(c);
    }

    Graphic proc;
    Graphic proc1;
    Graphic proc2;
    std::vector <Graphic> proc3;
    Graphic proc4;
    std::vector <Graphic> proc5;
    Graphic proc6;
    int c;
    int f;
    int bigger = 0;

    for (int d = 0; d < pols.size(); ++d) {

        proc = pols[d];

        if (proc.points.size() == 0) {
            //puts("// *WARNING*: image contains degenerated polygon!");
            continue;
        }

        if (proc.points.size() > bigger)
            bigger = proc.points.size();

        if (proc.points.size() >= 35) {
           // puts("// *WARNING*: as of this beta version, the number of vertices you're pushing is not supported");
        }

        float xv[proc.points.size()];
        int xi[proc.points.size()];

        for (int e = 0; e < proc.points.size(); e++) {
            xv[e] = proc.points[e].x;
            xi[e] = e;
        }

        order(xv, xi, proc.points.size());

        for (int h = 0; h < proc.points.size(); h++) {
            c = xi[h];
            f = xi[(h + 1) % proc.points.size()];

            proc1 = CullXNoLessThan(proc.points[c].x, proc);
            proc2 = CullXNoMoreThan(proc.points[f].x, proc1);
            proc3 = decomposeMonotone(proc2);

            for (int i = 0; i < proc3.size(); ++i) {
                proc4 = proc3[i];

                proc5 = triangulate(proc4);

                for (int j = 0; j < proc5.size(); ++j) {
                    proc6 = proc5[j];
                    candidate.push_back(proc6);
                }
            }
        }
    }

    //printf("bigger polygon has %d \n", bigger);

    return candidate;
}


std::vector <Graphic> triangulate(Graphic proc2) {
    std::vector <Graphic> toReturn;
    Graphic pol;

    if (proc2.points.size() <= 3 ) {
        return toReturn;
    }

    for (int c = 0; c < proc2.points.size() - 2; ++c) {
        pol.points.clear();
        pol.colour = proc2.colour;
        pol.points.push_back(proc2.points[0]);
        pol.points.push_back(proc2.points[c + 1]);
        pol.points.push_back(proc2.points[c + 2]);
        toReturn.push_back(pol);
    }
    return toReturn;
}

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

 Graphic parsePath(const char *pathStr) {
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

RGB parseStyle(const char* styleStr) {
    std::string style = styleStr;
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

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cout << "USAGE: trigger [svg]" << std::endl;
        return 0;
    }

    std::string filename = argv[1];

    if (filename.rfind(".svg") > filename.length() ) {
        std::cout << "/* " << filename << " is not an SVG file! */" << std::endl;
        return 0;
    }

    std::cout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n<svg\n";
    std::cout << "width=\"800\"\n"
                 "   height=\"480\"\n"
                 "   version=\"1.1\"\n"
                 "   id=\"svg1\"\n"
                 "   xmlns=\"http://www.w3.org/2000/svg\"\n"
                 "   xmlns:svg=\"http://www.w3.org/2000/svg\"";
    std::cout << ">\n" << std::endl;

    openXML(filename.c_str(), true);

    std::cout << "\n</svg>" << std::endl;

    return 0;
}
