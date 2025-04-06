// Copyright (C) 2002, 2003
// Charles Gretton and David Price
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/* Utils.c++
 *
 * \paragraph{:purpose:}
 *
 * Some simple helper functions that don't really belong to any
 * particular class.
 */

#include "common.h++"
#ifdef HEADER

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
//#include <cstdio>

#include<cstdio>
#include <string>
using namespace std;

class Utils
{
};

#endif
#include "Utils.h++"

PUBLIC STATIC string Utils::intToString(int num)
{
    char buf[15];
    sprintf(buf, "%d", num);
    return buf;
}

PUBLIC STATIC string Utils::doubleToString(double num, int nDecimalPlaces DEFVAL(5))
{
    char buf[20];
    char fmt[20];
    sprintf(fmt, "%%.%df", nDecimalPlaces);
    sprintf(buf, fmt, num);
    return buf;
}

/* Add newlines in appropriate places in order to word wrap the input
 * string.  Also indents by specified number of spaces.  Newlines are
 * allowed in the input string, and force a line break in that
 * possition
 */
PUBLIC STATIC string Utils::wordWrapString(int indentSize, const string& input)
{
    const static int maxLineLength = 75;
    string result = string(indentSize, ' ');
    string::const_iterator i;
    int charsOnLine = indentSize;
    string currentWord;
    for (i = input.begin(); i != input.end(); ++i) {
        if (*i == ' ') {
            if (charsOnLine + int(currentWord.length()) > maxLineLength) {
                result += "\n" + string(indentSize, ' ');
                charsOnLine = indentSize;
            }
            if (charsOnLine > indentSize) {
                result += " ";
                charsOnLine++;
            }
            result += currentWord;
            charsOnLine += currentWord.length();
            currentWord = "";
        } else if (*i == '\n') {
            if (charsOnLine > indentSize)
                result += " ";
            result += currentWord;
            currentWord = "";
            result += "\n" + string(indentSize, ' ');
            charsOnLine = indentSize;
        } else {
            currentWord += *i;
        }
    }
    if (charsOnLine > indentSize) {
        result += " ";
        charsOnLine++;
    }
    result += currentWord + "\n";
    
    return result;
}

PUBLIC STATIC void Utils::displayDotString(const string& dotString)
{
    ofstream dotfile;
    dotfile.open("tmp.dot");
    if (dotfile.is_open()) {
        dotfile << dotString;
        dotfile.close();
        
        system("dot -Tps tmp.dot -o tmp.ps && gv tmp.ps");
    }
}

PUBLIC STATIC void Utils::dotToPS(const string& dotString, const string& filename)
{
    std::ofstream dotfile;
    dotfile.open("tmp.dot");
    if (dotfile.is_open()) {
        dotfile << dotString;
        dotfile.close();
        
        system(("dot -Tps tmp.dot -o \"" + filename + "\"").c_str());
    }
}

PUBLIC STATIC void Utils::dotToEPS(const string& dotString, const string& filename)
{
    ofstream dotfile;
    dotfile.open("tmp.dot");
    if (dotfile.is_open()) {
        dotfile << dotString;
        dotfile.close();
        
        //system(("dot -Tfig tmp.dot -o tmp.fig && fig2dev -Leps tmp.fig \"" + filename + "\"").c_str());
        system(("dot -Tps tmp.dot -o tmp.ps && ps2epsi tmp.ps \"" + filename + "\"").c_str());
    }
}

PUBLIC STATIC string Utils::errorToString(int errnum)
{
    return strerror(errnum);
}

