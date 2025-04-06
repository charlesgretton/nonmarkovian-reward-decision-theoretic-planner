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

/* ActionADD.c++
 *
 * \paragraph{:Purpose:}
 *
 * Produces an ADD that represents the probabilities of some action
 * having a certain effect.  The ADD will contain both primed and
 * unprimed variables, and can intuitively be thought of as answering
 * the question, "what is the probability of these variables being in
 * this state before the action, and in this other state after the
 * action".  This is the combination of the dual-action diagrams for
 * an action.
 */

#include "common.h++"
#ifdef HEADER

#include "ExtADD.h++"
#include "actionSpecification.h++"

using namespace MDP;

class ActionADD
{
private:
    DomainSpecification& domSpec;
    ActionSpecification& actionSpec;
    Cudd& mgr;

    ExtADD add;
};

#endif
#include "ActionADD.h++"
#include "domainSpecification.h++"
#include "Diagram.h++"

PUBLIC ActionADD::ActionADD(DomainSpecification& domSpec, ActionSpecification::CPTS* cpts, const ExtADD& constraint)
    : domSpec(domSpec),
      actionSpec(*domSpec.getActionSpecification()),
      mgr(actionSpec.getManager())
{
    add = mgr.addOne();

    DomainSpecification::PropositionVector props = domSpec.getPropositions();
    DomainSpecification::PropositionVector::const_iterator j;
    
    for (j = props.begin(); j != props.end(); ++j) {
        ExtADD var = mgr.addVar(domSpec.getProposition(*j));
        ExtADD varPrimed = var.prime(props.size());

        ExtADD dualAction;
        if (cpts->count(*j) > 0) {
            dualAction = (varPrimed & (*cpts)[*j])
                + ((~varPrimed) & (mgr.addOne() - (*cpts)[*j]));
        } else {
            dualAction = (var & varPrimed) | ((~var) & (~varPrimed));
        }
            
        add *= dualAction;

        Diagram act;
        act.setNodeNames(props);
        
#if 0
        act.setADD(add);
        act.setTitle("before constraining");
        Utils::displayDotString(act.toDotString());
#endif

        if (constraint != mgr.addOne()) {
            add = add.constrain(constraint, props.size());

#if 0
            vector<int> unusedVariables = add.variablesUsed(props.size(), true);
            ExtADD tmp = constraint.sumOver(unusedVariables);
            tmp = tmp.toZeroOne();

            if (tmp != constraint) {
                act.setADD(add);
                act.setTitle("add");
                Utils::displayDotString(act.toDotString());
            
                act.setADD(constraint);
                act.setTitle("constraint before filtering");
                Utils::displayDotString(act.toDotString());
            
                act.setADD(tmp);
                act.setTitle("constraint after filtering");
                Utils::displayDotString(act.toDotString());
            }
            add *= tmp;
            
            //add *= constraint;
#endif
        }

#if 0
        act.setADD(add);
        act.setTitle("after constraining");
        Utils::displayDotString(act.toDotString());
#endif
    }
}

/* The ADD itself */
PUBLIC ADD ActionADD::getADD()
{
    return add;
}
