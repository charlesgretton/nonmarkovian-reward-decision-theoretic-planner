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

/* DomainInspector.c++
 *
 * \paragraph{:purpose:}
 *
 * Provides command line function for inspecting various components of
 * the current domain specification.
 */

#include "common.h++"
#ifdef HEADER

#include"CommandListener.h++"
#include "SpecificationTypes.h++"
#include "AlgorithmTypes.h++"

using namespace MDP;

class DomainInspector : public CommandListener
{
private:
    Algorithm *algorithm;

    CommandInterpreter& commandInterpreter;
    DomainSpecification* domSpec;
    ActionSpecification* actionSpec;
};

#endif
#include "CommandInterpreter.h++"
#include "domainSpecification.h++"
#include "actionSpecification.h++"
#include "Algorithm.h++"
#include "DomainInspector.h++"
#include "Registry.h++"
#include "ExtADD.h++"
#include "rewardSpecification.h++"
#include "Diagram.h++"
#include "Utils.h++"
#include "ADDreward.h++"
#include "ADDstateCounter.h++"
#include "AutomaticConstraintGenerator.h++"

#include <fstream>
#include <cstdlib>
#include <sstream>
using namespace std;

PUBLIC DomainInspector::DomainInspector()
    : commandInterpreter(*CommandInterpreter::getInstance()),
      domSpec(commandInterpreter.getDomSpec()),
      actionSpec(domSpec->getActionSpecification())
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("rewardSpecification", this, 0);
    reg->setFunction("actions", this, 0);
    reg->setFunction("propositions", this, 0);
    reg->setFunction("actionToDot", this, 2);
    reg->setFunction("displayDot", this, 1);
    reg->setFunction("dotToPS", this, 2);
    reg->setFunction("rewardToDot", this, 0);
    reg->setFunction("constraintToDot", this, 0);
    reg->setFunction("reachableStates", this, 0);
    reg->setFunction("actionSpec", this, 0);
    reg->setFunction("clear", this, 0);
}

PUBLIC DomainInspector::~DomainInspector()
{
    Registry *reg = Registry::getInstance();
    reg->unregister("rewardSpecification", this);
    reg->unregister("actions", this);
    reg->unregister("propositions", this);
    reg->unregister("actionToDot", this);
    reg->unregister("displayDot", this);
    reg->unregister("dotToPS", this);
    reg->unregister("rewardToDot", this);
    reg->unregister("constraintToDot", this);
    reg->unregister("reachableStates", this);
    reg->unregister("actionSpec", this);
    reg->unregister("clear", this);
}

void DomainInspector::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "rewardSpecification") {
        commandResult = domSpec->getRewardSpecification()->toString();
    } else if (command == "actions") {
        map<action, ActionSpecification::CPTS*>::iterator i;
        for (i = actionSpec->begin(); i != actionSpec->end(); ++i) {
            commandResult += i->first + "\n";
        }
    } else if (command == "propositions") {
        DomainSpecification::PropositionVector props = domSpec->getPropositions();
        DomainSpecification::PropositionVector::const_iterator i;
        for (i = props.begin(); i != props.end(); ++i) {
            commandResult += *i + "\n";
        }
    } else if (command == "actionToDot") {
        //ActionSpecification& actionSpec = *(domSpec->getActionSpecification());
        map<action, ActionSpecification::CPTS*>::iterator i;
        map<proposition, ADD>::iterator j;

        DomainSpecification::PropositionVector props = domSpec->getPropositions();
        if (find(props.begin(), props.end(), parameters[1]) == props.end()) {
            cerr << "No such proposition \"" + parameters[1] << "\"\n";
            return;
        }

        string description = "Effect of action `" + parameters[0] + "' on proposition `" + parameters[1] + "'";
        
        bool validAction = false;
        for (i = actionSpec->begin(); i != actionSpec->end(); ++i) {
            if (i->first == parameters[0]) {
                validAction = true;
                for (j = i->second->begin(); j != i->second->end(); ++j) {
                    if (j->first == parameters[1]) {
                        Diagram act;
                        act.setNodeNames(domSpec->getPropositions());
                        act.addPage(j->second, description);
                        commandResult = act.toDotString();
                        return;
                    }
                }
            }
        }
        if (validAction)
            cerr << description << " not defined\n";
        else
            cerr << "No such action \"" + parameters[0] << "\"\n";
    } else if (command == "displayDot") {
        string epsOutput = Registry::getInstance()->getString("epsOutput", "");
        if (epsOutput.size() > 0) {
            Utils::dotToEPS(parameters[0], epsOutput);
        } else {
            Utils::displayDotString(parameters[0]);
        }
    } else if (command == "dotToPS") {
        Utils::dotToPS(parameters[0], parameters[1]);
    } else if (command == "rewardToDot") {
        ADDreward rwd(*domSpec);
        commandResult = rwd.getRewardDiagram().toDotString();
    } else if (command == "constraintToDot") {
        ADDreward rwd(*domSpec);
        commandResult = rwd.getConstraintDiagram().toDotString();
    } else if (command == "reachableStates") {
        AutomaticConstraintGenerator acg(*domSpec);
        while (!acg());
        DomainSpecification::PropositionVector props = domSpec->getPropositions();
        double reachable_states = ADDstateCounter(acg.getConstraint(), props.size()).getCount();
        commandResult = Utils::doubleToString(reachable_states, 0);
    } else if (command == "actionSpec") {
        ostringstream result;
        
        map<action, ActionSpecification::CPTS*>::iterator i;
        for (i = actionSpec->begin(); i != actionSpec->end(); ++i) {
            result << "action " << i->first << endl;
            map<proposition, ADD> *cpts = i->second;
            map<proposition, ADD>::iterator j;
            for (j = cpts->begin(); j != cpts->end(); ++j) {
                result << "    " << j->first << "  ";
                ExtADD add = j->second;
                result << add.toTreeString(domSpec->getPropositions()) << endl;
            }
            result << "endaction\n\n";
        }
        commandResult = result.str();
    } else if (command == "clear") {
        domSpec = commandInterpreter.getDomSpec();
        actionSpec = domSpec->getActionSpecification();
    }
    
}

string DomainInspector::shortHelp(const string &command) const
{
    if (command == "rewardSpecification")
        return shortHelpLine(command, "Output the reward specification.");
    if (command == "actions")
        return shortHelpLine(command, "List actions for current domain specification.");
    if (command == "propositions")
        return shortHelpLine(command, "List propositions for current domain specification.");
    if (command == "actionToDot")
        return shortHelpLine(command + "(act, prop)", "Convert to dot, the behaviour of prop when act is applied.");
    if (command == "displayDot")
        return shortHelpLine(command + "(dotString)", "Pass dotString to dot, then call gv to display result.");
    if (command == "dotToPS")
        return shortHelpLine(command + "(dotString,outfile)", "Call dot to produce a PS file.");
    if (command == "rewardToDot")
        return shortHelpLine(command, "Produce a dot specification of the current Markovian reward.");
    if (command == "constraintToDot")
        return shortHelpLine(command, "Produce a dot specification of the current Markovian constraint.");
    if (command == "reachableStates")
        return shortHelpLine(command, "The number of states that can possibly be reached from the start state.");
    if (command == "actionSpec")
        return shortHelpLine(command, "Print the action specification in the same format used for input.");
    return "";
}

string DomainInspector::longHelp(const string &command) const
{
    if (command == "actionToDot") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "act - an action\n"
            "prop - a proposition\n"
            "Produces output in a format suitable for interpretation by dot which is part of the "
            "GraphViz package.");
    }
    if (command == "displayDot") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "Example usage: " + command + "(actionToDot(...))\n"
            "Note: if epsOutput is set, then output will be written there instead.");
    }
    return shortHelp(command) + "\n";
}
