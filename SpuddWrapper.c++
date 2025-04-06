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

/* SpuddWrapper.c++
 *
 * \paragraph{:Purpose:}
 *
 * Provides the command line bindings for the Spudd algorithm.  This
 * is separate from the Spudd class in order to not tie the Spudd
 * class to a command line interface.
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"

#include "Spudd.h++"
using namespace MDP;

class SpuddWrapper : public CommandListener
{
public:
    friend class StructuredSolutionWrapper;
private:
    Spudd *spudd;
    bool monitorChanges;
    string lastSpuddPolicy;
    string changeHistory;
};

#endif
#include "SpuddWrapper.h++"
#include "Spudd.h++"
#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "Utils.h++"

#include <sstream>
#include <cstdlib>
using namespace std;

PUBLIC SpuddWrapper::SpuddWrapper()
    :spudd(0)
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("spudd", this, 2);
    reg->setFunction("spuddPolicyToDot", this, 0);
    reg->setFunction("spuddValueToDot", this, 0);
    reg->setFunction("spuddDeltaHistory", this, 0);
    reg->setFunction("spuddChangeHistory", this, 0);
    reg->setFunction("spuddValueLeaves", this, 0);
    reg->setFunction("spuddValueDensity", this, 0);
    reg->setFunction("spuddValueNodes", this, 0);
    reg->setFunction("spuddValuePaths", this, 0);
    reg->setFunction("clear", this, 0);
}

PUBLIC SpuddWrapper::~SpuddWrapper()
{
    if (spudd != NULL) {
        delete spudd;
        spudd = NULL;
    }
        
    Registry *reg = Registry::getInstance();
    reg->unregister("spudd", this);
    reg->unregister("spuddPolicyToDot", this);
    reg->unregister("spuddValueToDot", this);
    reg->unregister("spuddDeltaHistory", this);
    reg->unregister("spuddChangeHistory", this);
    reg->unregister("spuddValueLeaves", this);
    reg->unregister("spuddValueDensity", this);
    reg->unregister("spuddValueNodes", this);
    reg->unregister("spuddValuePaths", this);
    reg->unregister("clear", this);
}

bool SpuddWrapper::commandIteration(const string &command)
{
    if (command == "spudd") {
        bool done = (*spudd)();
        if (monitorChanges) {
            string tmp = spudd->getPolicy().toDotString();
            if (tmp != lastSpuddPolicy) {
                changeHistory += '1';
                lastSpuddPolicy = tmp;
            } else {
                changeHistory += '0';
            }
        }
        return done;
    } else {
        return true;
    }
}

void SpuddWrapper::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "spudd") {
        if (spudd != NULL) {
            delete spudd;
            spudd = NULL;
        }

        bool hideAssumptions = Registry::getInstance()->getBool("hideAssumptions", false);
        monitorChanges = Registry::getInstance()->getBool("monitorSpuddPolicyChanges", false);
        
        CommandInterpreter *ci = CommandInterpreter::getInstance();
        double discount = atof(parameters[0].c_str());
        double epsilon = atof(parameters[1].c_str());
        spudd = new Spudd(*ci->getDomSpec(), ci->getAutomaticConstraint(), discount, epsilon, hideAssumptions);
    } else if (command == "clear") {
        if (spudd != NULL) {
            delete spudd;
            spudd = NULL;
        }
    } else {
        if (spudd == NULL) {
            cerr << command << ": spudd algorithm has not been run\n";
            return;
        }
        if (command == "spuddPolicyToDot")
            commandResult = spudd->getPolicy().toDotString();
        if (command == "spuddValueToDot")
            commandResult = spudd->getOptimalValue().toDotString();
        if (command == "spuddDeltaHistory") {
            vector<double> history = spudd->getDeltaHistory();
            ostringstream result;
            bool first = true;
            for (vector<double>::const_iterator i = history.begin(); i != history.end(); ++i) {
                if (!first)
                    result << ',';
                first = false;
                result << *i;
            }
            commandResult = result.str();
        }
        if (command == "spuddChangeHistory") {
            commandResult = changeHistory;
        }
        if (command == "spuddValueLeaves") {
            commandResult = Utils::intToString(spudd->getOptValSize());
        }
        if (command == "spuddValueDensity") {
            commandResult = Utils::doubleToString(spudd->getOptValDensity());
        }
        if (command == "spuddValueNodes") {
            commandResult = Utils::doubleToString(spudd->getOptValNodes(), 0);
        }
        if (command == "spuddValuePaths") {
            commandResult = Utils::doubleToString(spudd->getOptValPaths(), 0);
        }
    }
}

string SpuddWrapper::shortHelp(const string &command) const
{
    if (command == "spudd")
        return shortHelpLine(command + "(d,e)", "SPUDD algorithm with `d' discount, `e' epsilon");
    if (command == "spuddPolicyToDot")
        return shortHelpLine(command, "Return the current policy as a dot graph.");
    if (command == "spuddValueToDot")
        return shortHelpLine(command, "Return the current expected value graph as a dot graph.");
    if (command == "spuddDeltaHistory")
        return shortHelpLine(command, "Record of the convergence of the solution.");
    if (command == "spuddChangeHistory")
        return shortHelpLine(command, "0/1 record of changes in policy for each iteration.");
    if (command == "spuddValueLeaves")
        return shortHelpLine(command, "The number of leaves in the optimal value ADD.");
    if (command == "spuddValueDensity")
        return shortHelpLine(command, "The density of the optimal value ADD.");
    if (command == "spuddValueNodes")
        return shortHelpLine(command, "The number of nodes in the optimal value ADD.");
    if (command == "spuddValuePaths")
        return shortHelpLine(command, "The number of paths in the optimal value ADD.");
    return "";
}

string SpuddWrapper::longHelp(const string &command) const
{
    if (command == "spudd") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "This command takes two parameters:  \n"
            "  arg1 : discount factor\n"
            "  arg2 : epsilon\n"
            "The domain specification should be Markovian before calling this.  "
            "See the PLTLvarExpand command for how to make the domain specification Markovian.  ");
    }
    if (command == "spuddDeltaHistory") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "Returns a comma separated list of values that are the maximum absolute difference "
            "between the value function at each iteration.");
    }
    if (command == "spuddChangeHistory") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "If monitorSpuddPolicyChanges was set to true when Spudd was previously run, "
            "then this function will return a string of 0s and 1s, where the 1s represent "
            "iterations in which the policy changed.  Note that using this feature will "
            "slow down the execution of the SPUDD algorithm by about a factor of 2.");
    }
    return shortHelp(command) + "\n";
}
