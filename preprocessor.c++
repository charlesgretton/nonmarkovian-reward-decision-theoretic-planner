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

extern "C" {
#include <unistd.h>
}

#include <cstdio>

#include <iostream>
#include <string>
#include <map>
#include <cstring>
using namespace std;

#include "preprocessor.h++"

static bool getLine(string& result)
{
    int ch;
    result = "";//.clear();
    while (true) {
        ch = fgetc(stdin);
        if (ch <= 0 || ch >= 256)
            return false;
        if (ch == '\n')
            return true;
        if (ch == '\r')
            continue;
        result += ch;
    }
}

#if 0
/*The following \method{parseDefineOld} used to be called
 *\method{parseDefine} but this implementation was since deprecated in
 *favour for the \method{parseDefine} below.*/
static void parseDefineOld(const string& line, string& lhs, string& rhs)
{
    int i = 7; // first character after #define
    int l = line.size();

    // skip whitespace
    while ((line[i] == ' ' || line[i] == '\t') && i < l)
        i++;

    // read lhs
    lhs = "";//.clear();
    while (line[i] != ' ' && line[i] != '\t' && i < l) {
        lhs += line[i];
        i++;
    }

    // skip whitespace
    while ((line[i] == ' ' || line[i] == '\t') && i < l)
        i++;

    // read rhs
    rhs = ""; //.clear();
    while (line[i] != ' ' && line[i] != '\t' && i < l) {
        rhs += line[i];
        i++;
    }
}
#endif

static void parseDefine(const string& line, string& lhs, string& rhs)
{
    int i = 7; // first character after #define
    int l = line.size();

    // skip whitespace
    while ((line[i] == ' ' || line[i] == '\t') && i < l)
        i++;

    // read lhs
    lhs = "";//.clear();
    while (line[i] != ' ' && line[i] != '\t' && i < l) {
        lhs += line[i];
        i++;
    }

    // skip whitespace
    while ((line[i] == ' ' || line[i] == '\t') && i < l)
        i++;

    // read rhs
    rhs = ""; //.clear();
    while (i < l) {
        rhs += line[i];
        i++;
    }
}

static void outputPreprocessedLine(const string& line, map<string, string>& defines)
{
    string result;
    int l = line.size();
    string word;

    for (int i = 0; i < l; i++) {
        switch (line[i]) {
            case ' ':
            case '(':
            case ')':
                if (defines.count(word) > 0)
                    cout << defines[word];
                else
                    cout << word;
                cout << line[i];
                word = "";//.clear();
                break;
            default:
                word += line[i];
        }
    }
    if (defines.count(word) > 0)
        cout << defines[word];
    else
        cout << word;
    word = "";//.clear();
}

void preprocess()
{
    string line;

    map<string, string> defines;
    string lhs, rhs;
    
    while (getLine(line)) {
        if (!strncmp(line.c_str(), "#define", 7)) {
            parseDefine(line, lhs, rhs);
            defines[lhs] = rhs;
            outputPreprocessedLine("", defines);
        }else if (!strncmp(line.c_str(), "--", 2)) {
            outputPreprocessedLine("", defines);
            ;
        }else if (!strncmp(line.c_str(), "//", 2)) {
            outputPreprocessedLine("", defines);
            ;
        }else if (!strncmp(line.c_str(), "%", 1)) {
            outputPreprocessedLine("", defines);
            ;
        }else {
            outputPreprocessedLine(line, defines);
        }
        cout << endl;
    }
}

FILE *preprocessFrom(FILE *from)
{
    int output[2];
    int pid;

    pipe(output);

    pid = fork();

    if (pid == 0) {
        close(0);
        dup(fileno(from));
        close(1);
        dup(output[1]);

        close(output[0]);
        close(output[1]);

        preprocess();
        exit(0);
    }

    close(output[1]);
    return fdopen(output[0],  "r");
}

