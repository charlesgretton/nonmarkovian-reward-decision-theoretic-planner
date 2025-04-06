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

/* StructuredPLTLtranslatorWrapper.c++
 *
 * \paragraph{:Purpose:}
 *
 * Provides the command line bindings to the StructuredPLTLtranslator
 * class.  This is separate from that class in order to avoid tying
 * that class to the command line.
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"

#include "Algorithm.h++"
using namespace MDP;

class StructuredPLTLtranslatorWrapper : public CommandListener
{
private:
    Algorithm *algorithm;
};

#endif
#include "StructuredPLTLtranslatorWrapper.h++"
#include "StructuredPLTLtranslator.h++"
#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "Utils.h++"

#include <cstdlib>
using namespace std;

PUBLIC StructuredPLTLtranslatorWrapper::StructuredPLTLtranslatorWrapper()
    :algorithm(0)
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("PLTLvarExpand", this, 0);
    reg->setFunction("clear", this, 0);
}

PUBLIC StructuredPLTLtranslatorWrapper::~StructuredPLTLtranslatorWrapper()
{
    if (algorithm)
        delete algorithm;
    Registry *reg = Registry::getInstance();
    reg->unregister("PLTLvarExpand", this);
    reg->unregister("clear", this);
}

bool StructuredPLTLtranslatorWrapper::commandIteration(const string &command)
{
    if (command == "PLTLvarExpand")
        return (*algorithm)();
    else
        return true;
}

void StructuredPLTLtranslatorWrapper::commandIssued(const string &command, const vector<string> &parameters)
{
    if (algorithm) {
        delete algorithm;
        algorithm = 0;
    }

    if (command == "PLTLvarExpand") {
        CommandInterpreter *ci = CommandInterpreter::getInstance();
        algorithm = new StructuredPLTLtranslator(*ci->getDomSpec());
    }
}

PUBLIC void StructuredPLTLtranslatorWrapper::endCommand(const string& command)
{
    if (algorithm) {
        delete algorithm;
        algorithm = NULL;
    }
}

string StructuredPLTLtranslatorWrapper::shortHelp(const string &command) const
{
    if(command == "PLTLvarExpand")
        return shortHelpLine(command, "PLTL NMRDP to MDP translation for structured algorithms.");
    else
        return "";
}

string StructuredPLTLtranslatorWrapper::longHelp(const string &command) const
{
    string result = shortHelp(command) + "\n" + "";
    
    if(command == "PLTLvarExpand")
        return  shortHelp(command) + "\n"
            + Utils::wordWrapString(helpIndentSize,
                                    "Adds extra propositions to keep track of history.");
    return result;
}
