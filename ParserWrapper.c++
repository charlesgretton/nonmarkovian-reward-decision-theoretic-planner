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

/* ParserWrapper.c++
 *
 * \paragraph{:purpose:}
 *
 * Provides a command line interface to the domain parser.
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"
#include "Algorithm.h++"

using namespace MDP;

class ParserWrapper : public CommandListener
{
};

#endif
#include "ParserWrapper.h++"
#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "Utils.h++"

#include <cerrno>
#include <cstdlib>
#include <cstdio>

using namespace std;

/*Bison parser compatibility declarations.*/
extern int yyparse();

extern bool requireStartStateProposition;
extern void lexerSetInput(FILE *file);

/*Temporary (while parse) pointer to CUDD garbage collection.*/
Cudd *mgr;

/*Temporary (while parse) specification of problem domain.*/
DomainSpecification  *domSpec;

PUBLIC ParserWrapper::ParserWrapper()
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("loadWorld", this, 1);
    reg->setFunction("parse", this, 0);
    reg->setFunction("startStateRequired", this, 1);
}

PUBLIC ParserWrapper::~ParserWrapper()
{
    Registry *reg = Registry::getInstance();
    reg->unregister("loadWorld", this);
    reg->unregister("parse", this);
    reg->unregister("startStateRequired", this);
}

extern int yylineno;

void ParserWrapper::commandIssued(const string &command, const vector<string> &parameters)
{
    // stuff done for both commands
    CommandInterpreter *ci = CommandInterpreter::getInstance();
    domSpec = ci->getDomSpec();
    mgr = &domSpec->getActionSpecification()->getManager();

    if (command == "loadWorld") {
        yylineno = 0;
        FILE *inputFile = fopen(parameters[0].c_str(), "r");
        if (inputFile == NULL) {
            cerr << "Error opening input file: \"" + parameters[0] + "\": "
                + Utils::errorToString(errno) + "\n";
        } else {
            lexerSetInput(inputFile);
            yyparse();
        }
    } else if (command == "parse") {
        yylineno = 0;
        yyparse();
    } else if(command == "startStateRequired") {
        if("true" == parameters[0] || atoi(parameters[0].c_str()))
            requireStartStateProposition = true;
        else
            requireStartStateProposition = false;
    }
}

string ParserWrapper::shortHelp(const string &command) const
{
    if (command == "loadWorld")
        return shortHelpLine(command + "(file)", "Load a domain specification from specified file.");
    else if (command == "parse")
        return shortHelpLine(command, "Read a domain specification from standard input.");
    else if(command == "startStateRequired")
        return shortHelpLine(command, string("Set requirement for start state proposition (PLTL only)."));
    return "";
}

string ParserWrapper::longHelp(const string &command) const
{
    string result = shortHelp(command) + "\n" + "";

     if(command == "startStateRequired")
        result += Utils::wordWrapString(helpIndentSize, "\t arg1 :: '1' if a start state proposition is required or "
                                        "'0' if not.\n\n"
                                        "A call informs NMRDPP that the PLTL domain specification, during state space expansion, "
                                        "requires a start state proposition. NMRDPP must be notified of the need"
                                        " for such a proposition before the domain is loaded/parsed.\n");
    
    return result;
}
