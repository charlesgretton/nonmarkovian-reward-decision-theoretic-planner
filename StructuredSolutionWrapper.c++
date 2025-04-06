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

/* StructuredSolutionWrapper.c++
 *
 * \paragraph{:Purpose:}
 *
 * An interface to amalgamate the features of the lower level
 * interfaces. This module contains the implementation of
 * \class{StructuredSolutionWrapper} which makes use of the lower
 * level wrappers. Interaction through wrappers distinguishes this
 * interface module form its counterpart the
 * \module{StateBasedSolutionWrapper}.
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"
class Registry;

class StructuredSolutionWrapper : public CommandListener
{
private:
    Registry *reg;
};

#endif
#include "StructuredSolutionWrapper.h++"
#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "Utils.h++"

#include<string>

using namespace std;

PUBLIC StructuredSolutionWrapper::StructuredSolutionWrapper()
    : reg(Registry::getInstance())
{
    reg->setFunction("preprocess", this, 1);
    reg->setFunction("preprocess", this, 2);
}

PUBLIC StructuredSolutionWrapper::~StructuredSolutionWrapper()
{       
    reg->unregister("preprocess", this);
    reg->unregister("preprocess", this);
}

void StructuredSolutionWrapper::commandIssued(const string &command, const vector<string> &parameters)
{    
    commandResult = "";
    acceptLastCommand = false;
    
    if (command == "preprocess") {
        if (parameters[0] == "pltlstruct" || parameters[0] == "spltl")
        {
            commandResult += reg->evaluate("PLTLvarExpand", vector<string>());
            
            /*If a second command was passed, then we will probably
              have to apply the space exploration to generate a
              constraint.*/
            if (2 == parameters.size() && "autoConstrain" == parameters[1])
                commandResult += reg->evaluate("autoConstrain", vector<string>());
            acceptLastCommand = true;
        }
    } else if (command == "explore") {
        commandResult += reg->evaluate("autoConstrain", vector<string>());
        acceptLastCommand = true;
    }
}

string StructuredSolutionWrapper::shortHelp(const string &command) const
{
    string result = "";
    
    if(command == "preprocess")
         result = shortHelpLine(command, "Generate temporal variables and their dynamics for\n")
            + shortHelpLine("", "use by a structured algorithm.");
    
    return result;
}

string StructuredSolutionWrapper::longHelp(const string &command) const
{
    string result = shortHelp(command) + "\n" +"";
    if(command == "preprocess")
        result +=
            Utils::wordWrapString(helpIndentSize,
                                  "\t arg1 :: Abbreviation \"spltl\".\n\n"
                                  "Executing this command with the argument \"pltlstruct\", or the "
                                  "abbreviation \"spltl\", causes NMRDPP to compute the set of temporal "
                                  "variables as well as the ADDs for their dynamics and the reward. This "
                                  "essentially translates the NMRDP into a corresponding MDP (XMDP). If "
                                  "you would like finer grained control over this process see the "
                                  "commands \"PLTLvarExpand\" and \"autoConstrain\" respectively. "
                                  "\n\n"
                                  "Furthermore, if you would also like the preprocessing phase to "
                                  "execute automatic constraint generation then pass, as a second "
                                  "argument to this command, the string \"autoConstrain\".");
    
    return result;
}
