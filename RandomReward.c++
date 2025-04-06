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

/* RandomReward.c++
 */

#include "common.h++"
#ifdef HEADER

#include "domainSpecification.h++"
#include "actionSpecification.h++"
#include "Random.h++"
#include "ExtADD.h++"

using namespace Formula;
using namespace MDP;

class RandomReward : public Random
{
private:
    DomainSpecification& domSpec;
    ActionSpecification& actionSpec;
    Cudd& mgr;
    vector<string> props;
    
};

#endif
#include "RandomReward.h++"
#include "ADDstateCounter.h++"
#include "AutomaticConstraintGenerator.h++"
#include "Utils.h++"
#include <cmath>
using namespace std;

PUBLIC RandomReward::RandomReward(DomainSpecification& domSpec, int numReachableFormulas, int numUnreachableFormulas)
    : domSpec(domSpec),
      actionSpec(*domSpec.getActionSpecification()),
      mgr(actionSpec.getManager()),
      props(domSpec.getPropositions())
{
    AutomaticConstraintGenerator acg(domSpec);
    while (!acg());
    ExtADD constraint = acg.getConstraint();

    for (int i = 0; i < numReachableFormulas; i++) {
        formula *form = getRandomReward(constraint, true);
        if (NULL == form) {
            cerr << "formula creation failed\n";
        } else {
            domSpec.addSpecComponent("r" + Utils::intToString(i), 1.0, form);
        }
    }
    
    for (int i = 0; i < numUnreachableFormulas; i++) {
        formula *form = getRandomReward(constraint, false);
        if (NULL == form) {
            cerr << "formula creation failed\n";
        } else {
            domSpec.addSpecComponent("u" + Utils::intToString(i), 1.0, form);
        }
    }
}

PRIVATE formula *RandomReward::getRandomSubReward(ExtADD add, bool reachable, bool side)
{
    double desiredValue = reachable ? 1.0 : 0.0;
    ExtADD child = side ? add.thenChild() : add.elseChild();

    formula *newform = new literal(props[add.getIndex()]);
    if (!side)
        newform = new lnot(newform);
    
    if (child.isConstant()) {
        if (child.value() == desiredValue) {
            return newform;
        } else {
            return NULL;
        }
    } else {
        bool newside = randomBool();
        formula *subform = getRandomSubReward(child, reachable, newside);
        if (NULL == subform)
            subform = getRandomSubReward(child, reachable, !newside);
        if (NULL == subform)
            return NULL;
        return new conj(newform, subform);
    }
}

/* Produce a random reward formula (a conjunction of variables,
 * consistent with the constraint `add') */
PRIVATE formula *RandomReward::getRandomReward(ExtADD add, bool reachable)
{
    if (add.isConstant()) {
        return NULL;
    }
    
    bool side = randomBool();
    formula *subform = getRandomSubReward(add, reachable, side);
    if (NULL == subform)
        subform = getRandomSubReward(add, reachable, !side);
    
    return subform;
}

PUBLIC RandomReward::~RandomReward()
{
}
