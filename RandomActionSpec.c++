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

/* RandomActionSpec.c++
 */

#include "common.h++"
#ifdef HEADER

#include "domainSpecification.h++"
#include "Algorithm.h++"
#include "actionSpecification.h++"
#include "Random.h++"

using namespace Formula;
using namespace MDP;

class RandomActionSpec : public Algorithm, private Random
{
private:
    DomainSpecification& domSpec;
    ActionSpecification& actionSpec;
    Cudd& mgr;
    vector<string> propositions;

    int num_props;
    int num_actions;
    double proportion_reachable;
    double influence;
    double uncertainty;
};

#endif
#include "RandomActionSpec.h++"
#include "ADDstateCounter.h++"
#include "AutomaticConstraintGenerator.h++"
#include "Utils.h++"
#include <cmath>
using namespace std;

PUBLIC RandomActionSpec::RandomActionSpec(DomainSpecification& domSpec, int num_props, int num_actions,
                                  double proportion_reachable, double influence, double uncertainty)
    : domSpec(domSpec),
      actionSpec(*domSpec.getActionSpecification()),
      mgr(actionSpec.getManager()),
      num_props(num_props),
      num_actions(num_actions),
      proportion_reachable(proportion_reachable),
      influence(influence),
      uncertainty(uncertainty)
{
    for (int i = 0; i < num_props; i++) {
        string propname = propName(i);
        domSpec.addProposition(propname);
        mgr.addVar(domSpec.getProposition(propname));
        propositions.push_back(propname);
    }

    for (int i = 0; i < num_actions; i++) {
        string name = "do" + Utils::intToString(i);
        
        map<string, ADD> *behaviour = new MDP::ActionSpecification::CPTS;
        domSpec.addSpecComponent(name, behaviour);
        
        // we want to make sure that every action has at least one bit
        // of behaviour associated with it, and that every variable is
        // referenced by at least one action.
        int vars_per_action = int(ceil(double(num_props) / num_actions));
        for (int j = vars_per_action * i; j < vars_per_action * (i + 1) && j < num_props; j++) {
            (*behaviour)[propName(j)] = randomADD(propositions);
        }
    }
}

PUBLIC RandomActionSpec::~RandomActionSpec()
{
}

PROTECTED proposition RandomActionSpec::propName(int num)
{
    assert(num >= 0 && num < num_props);
    int pdig = int(ceil(log(double(num+1))/log(26.0))); // number of digits in the proposition name
    
    if (num == 0)
        pdig = 1;
    string propname(pdig, '#');
    for (int j = 0; j < pdig; j++) {
        int idx = int(num / (pow(26.0, (pdig - j - 1)))) % 26;
        assert(idx >= 0 && idx <= 25);
        propname[j] = 'a' + idx;
    }

    return propname;
}

PROTECTED string RandomActionSpec::pickRandomProp()
{
    return propName(randomInt(0, num_props - 1));
}

PROTECTED string RandomActionSpec::pickRandomAction()
{
    return "do" + Utils::intToString(randomInt(0, num_actions - 1));
}

PROTECTED ADD RandomActionSpec::randomADD(vector<string> propositions)
{
    if (propositions.empty()) {
        // leaf
        if (randomProb() > uncertainty)
            return mgr.constant(randomInt(0, 1));
        else
            return mgr.constant(randomProb());
    } else {
        int idx = randomInt(0, propositions.size() - 1);
        string varName = propositions[idx];
        propositions.erase(propositions.begin() + idx);
        
        if (randomProb() < influence) {
            ADD var = mgr.addVar(domSpec.getProposition(varName));
            ADD t = randomADD(propositions);
            ADD f = randomADD(propositions);
            return (t & var) + (f & ~var);
        } else {
            return randomADD(propositions);
        }
    }
}

PRIVATE void RandomActionSpec::addBehaviour()
{
    int action_n;
    string action, variable;
    map<string, ADD> *behaviour = NULL;
    
    while (true) {
        action_n = randomInt(0, num_actions - 1);
        action = "do" + Utils::intToString(action_n);
        variable = pickRandomProp();

        for (ActionSpecification::caIterator i = actionSpec.begin(); i != actionSpec.end(); ++i) {
            if (i->first == action) {
                for (ActionSpecification::CPTS::const_iterator j = i->second->begin();
                     j != i->second->end(); ++j) {
                    if (j->first == variable) {
                        continue;
                    }
                }
                behaviour = i->second;
                break;
            }
        }
        break;
    }

    if (NULL != behaviour)
        (*behaviour)[variable] = randomADD(propositions);
}

/* Do an iteration of the algorithm - return true if we are done
 */
PUBLIC bool RandomActionSpec::operator()()
{
    addBehaviour();
    
    // count the number of reachable states, and say we are done if it
    // is at least as many as we wanted
    AutomaticConstraintGenerator acg(domSpec);
    while (!acg());
    double reachable_states = ADDstateCounter(acg.getConstraint(), num_props).getCount();
    return reachable_states >= proportion_reachable * pow(2.0, num_props);
}
