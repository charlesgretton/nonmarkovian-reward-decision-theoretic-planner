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

/* AutomaticConstraintGeneratorWrapper.c++
 *
 * \paragraph{:Purpose:}
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"

#include "AutomaticConstraintGenerator.h++"
using namespace MDP;

class AutomaticConstraintGeneratorWrapper : public CommandListener
{
public:
    friend class StructuredSolutionWrapper;
private:
    AutomaticConstraintGenerator *acg;
    bool monitorChanges;
    string lastAutomaticConstraintGeneratorPolicy;
    string changeHistory;
};

#endif
#include "AutomaticConstraintGeneratorWrapper.h++"
#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "Utils.h++"
#include "Diagram.h++"

PUBLIC AutomaticConstraintGeneratorWrapper::AutomaticConstraintGeneratorWrapper()
    :acg(0)
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("autoConstrain", this, 0);
    reg->setFunction("autoConstraintToDot", this, 0);
    reg->setFunction("clear", this, 0);
}

PUBLIC AutomaticConstraintGeneratorWrapper::~AutomaticConstraintGeneratorWrapper()
{
    if (acg != NULL) {
        delete acg;
        acg = NULL;
    }
        
    Registry *reg = Registry::getInstance();
    reg->unregister("autoConstrain", this);
    reg->unregister("autoConstraintToDot", this);
    reg->unregister("clear", this);
}

bool AutomaticConstraintGeneratorWrapper::commandIteration(const string &command)
{
    if (command == "autoConstrain") {
        bool done = (*acg)();
        if (done) {
            CommandInterpreter *ci = CommandInterpreter::getInstance();
            ci->setAutomaticConstraint(acg->getConstraint());
        }
        return done;
    }
    return true;
}

void AutomaticConstraintGeneratorWrapper::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "autoConstrain") {
        if (acg != NULL) {
            delete acg;
            acg = NULL;
        }

        CommandInterpreter *ci = CommandInterpreter::getInstance();
        acg = new AutomaticConstraintGenerator(*ci->getDomSpec());
    } else if (command == "clear") {
        if (acg != NULL) {
            delete acg;
            acg = NULL;
        }
    } else {
        if (NULL == acg) {
            cerr << "autoConstrain must be run first\n";
        } else {
            if (command == "autoConstraintToDot") {
                CommandInterpreter *ci = CommandInterpreter::getInstance();
                Diagram result;
                result.setNodeNames(ci->getDomSpec()->getPropositions());
                result.addPage(acg->getConstraint(), "Automatic constraint");
                commandResult = result.toDotString();
            }
        }
    }
}

string AutomaticConstraintGeneratorWrapper::shortHelp(const string &command) const
{
    if (command == "autoConstrain")
        return shortHelpLine(command, "Produce a constraint by exploring the reachable state space.");
    if (command == "autoConstraintToDot")
        return shortHelpLine(command, "Dot representation of the automatically generated constraint.");
    return "";
}

string AutomaticConstraintGeneratorWrapper::longHelp(const string &command) const
{
    string result = shortHelp(command) + "\n" + "";
    
    if (command == "autoConstrain")
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "This command will only work with structured methods.  It requires a start state.");
    return result;
}
