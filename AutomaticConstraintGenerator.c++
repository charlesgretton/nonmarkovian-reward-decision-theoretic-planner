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

/* AutomaticConstraintGenerator.c++
 *
 * \paragraph{:Purpose:}
 *
 * Generates a constraint ADD suitable for use by algorithms such as
 * Spudd.  This constraint makes simplifies both the resulting optimal
 * policy, and the production of that policy by reducing the size of
 * the search space.
 */

#include "common.h++"
#ifdef HEADER

#include "ExtADD.h++"
#include "domainSpecification.h++"
#include "ActionADDmap.h++"
#include "Algorithm.h++"

using namespace Formula;
using namespace MDP;

class AutomaticConstraintGenerator : public Algorithm
{
public:
    ExtADD getConstraint() const
    {
        return constraint;
    }
    
private:
    DomainSpecification& domSpec;
    ActionSpecification& actionSpec;
    Cudd& mgr;
    
    ExtADD constraint;
    
    ActionADDmap combinedDualActionDiagrams;

    int numProps;

    DomainSpecification::PropositionVector props;
    map<action, ActionSpecification::CPTS *>::const_iterator i;
    ExtADD oldConstraint;
    ExtADD temp;
    vector<int> uncertain;
    vector<proposition>::const_iterator k;
};

#endif
#include "AutomaticConstraintGenerator.h++"

#include "Diagram.h++"
#include "Utils.h++"

PUBLIC AutomaticConstraintGenerator::AutomaticConstraintGenerator(DomainSpecification& domSpec)
    : domSpec(domSpec),
      actionSpec(*domSpec.getActionSpecification()),
      mgr(actionSpec.getManager()),
      combinedDualActionDiagrams(domSpec, mgr.addOne())
{
    props = domSpec.getPropositions();
    DomainSpecification::PropositionSet startState = domSpec.getStartStatePropositions();

    constraint = mgr.addOne();

    numProps = props.size();

    for (int j = 0; j != numProps; j++) {
        if (startState.count(props[j]) == 1) {
            constraint *= mgr.addVar(j);
        } else {
            constraint *= ~mgr.addVar(j);
        }
    }

    beginActionIteration();
}

PRIVATE void AutomaticConstraintGenerator::beginActionIteration()
{
    i = actionSpec.begin();
    oldConstraint = constraint;
    beginPropositionIteration();
}

PRIVATE void AutomaticConstraintGenerator::beginPropositionIteration()
{
    temp = oldConstraint;
    uncertain.clear();
    k = props.begin();
}

PUBLIC bool AutomaticConstraintGenerator::operator()()
{
    if (i == actionSpec.end()) {
        bool done = (constraint == oldConstraint);
        beginActionIteration();
        return done;
    } else {
        if (k == props.end()) {
            temp = temp.sumOver(uncertain);
        
            /* We sum over all unprimed variables */
            temp = temp.sumOver(temp.variablesUsed(numProps));
        
            temp = temp.unprimeRecursive(numProps);
        
            constraint |= temp.toZeroOne();

            i++;
            
            beginPropositionIteration();
        } else {
            ExtADD add;

            ExtADD var = mgr.addVar(domSpec.getProposition(*k));
            ExtADD varPrimed = var.prime(props.size());
            
            if ((*i->second).count(*k) > 0)
                add = (*i->second)[*k];
            else
                add = var;

            ExtADD dualAction = (varPrimed & add)
                + ((~varPrimed) & (mgr.addOne() - add));

            temp *= dualAction;

            /* If this action has an uncertain effect on this proposition,
             * (ie, it doesn't set it to either 0 or 1), then we must
             * not add this proposition to the constraint. */
            if (!temp.isZeroOne())
                uncertain.push_back(domSpec.getProposition(*k));
            k++;
        }
        return false;
    }
}
