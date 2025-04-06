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

/* RandomDomainWrapper.c++
 *
 * \paragraph{:Purpose:}
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"

class RandomActionSpec;

class RandomDomainWrapper : public CommandListener
{
private:
    RandomActionSpec *rd;
};

#endif
#include "RandomDomainWrapper.h++"
#include "RandomActionSpec.h++"
#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "Utils.h++"
#include "RandomActionSpec.h++"
#include "RandomReward.h++"

#include <sstream>
#include <cstdlib>
using namespace std;

PUBLIC RandomDomainWrapper::RandomDomainWrapper()
    : rd(NULL)
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("randomActionSpec", this, 5);
    reg->setFunction("randomReward", this, 2);
    reg->setFunction("randomSeed", this, 1);
}

PUBLIC RandomDomainWrapper::~RandomDomainWrapper()
{
    if (rd) {
        delete rd;
        rd = NULL;
    }
    
    Registry *reg = Registry::getInstance();
    reg->unregister("randomActionSpec", this);
    reg->unregister("randomReward", this);
    reg->unregister("randomSeed", this);
}

PUBLIC void RandomDomainWrapper::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    CommandInterpreter *ci = CommandInterpreter::getInstance();
    DomainSpecification *domSpec = ci->getDomSpec();
    if (command == "randomActionSpec") {
        rd = new RandomActionSpec(*domSpec, atoi(parameters[0].c_str()), atoi(parameters[1].c_str()),
                     atof(parameters[2].c_str()), atof(parameters[3].c_str()), atof(parameters[4].c_str()));
    } else if (command == "randomReward") {
        RandomReward *rr = new RandomReward(*domSpec, atoi(parameters[0].c_str()), atoi(parameters[1].c_str()));
        delete rr;
    } else if (command == "randomSeed") {
        srandom(atoi(parameters[0].c_str()));
    }
}

PUBLIC string RandomDomainWrapper::shortHelp(const string &command) const
{
    if (command == "randomActionSpec")
        return shortHelpLine(command + "(p,a,r,i,u)", "Produce a random action specification.");
    if (command == "randomReward")
        return shortHelpLine(command + "(r, u)", "Produce a random reward specification.");
    if (command == "randomSeed")
        return shortHelpLine(command + "(n)", "Sets the random number generator seed.");
    return "";
}

PUBLIC string RandomDomainWrapper::longHelp(const string &command) const
{
    if (command == "randomActionSpec") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "p: number of propositions.\n"
            "a: number of actions.\n"
            "r: proportion (0..1) of states reachable from the start state.\n"
            "i: influence - probability of a variable being used at any given point in the decision trees.\n"
            "u: uncertainty - probability of non-0/1 values.");
    } else if (command == "randomReward") {
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "r: number of attainable reward functions.\n"
            "u: number of unattainable reward functions.");
    }
    return shortHelp(command) + "\n";
}

PUBLIC bool RandomDomainWrapper::commandIteration(const string &command)
{
    if (command == "randomActionSpec") {
        return (*rd)();
    }
    return true;
}
