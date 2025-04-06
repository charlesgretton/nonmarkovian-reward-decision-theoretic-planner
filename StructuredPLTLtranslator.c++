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

/* StructuredPLTLtranslator.c++
 *
 * \paragraph{:Purpose:}
 *
 * Converts a PLTL domain specification into a Markovian domain
 * specification by adding extra propositions in order to store the
 * history necessary to answer all temporal expressions in the reward
 * specification.
 */

#include "common.h++"
#ifdef HEADER

#include "SpecificationTypes.h++"
#include "AlgorithmTypes.h++"
#include "formulaTypes.h++"
#include "rewardSpecification.h++"
#include "Algorithm.h++"
#include "ExtADD.h++"

#include"include/cuddObj.hh"
#include <string>

using namespace std;

using namespace Formula;
using namespace MDP;

class StructuredPLTLtranslator : public PLTLvisitor, public Algorithm
{
private:
    string rhs;
    formula *rhsFormula;
    ADD rhsADD;
    bool rhsValue;

    DomainSpecification& domSpec;
    ActionSpecification& actSpec;
    Cudd& mgr;
    RewardSpecification& rwdspec;
    
    // the next reward formula to process
    RewardSpecification::rIterator nextReward;
};

#endif

#include <set>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "StructuredPLTLtranslator.h++"
#include "formulaVisitation.h++"
#include "formula.h++"
#include "domainSpecification.h++"
#include "actionSpecification.h++"

PUBLIC StructuredPLTLtranslator::StructuredPLTLtranslator(DomainSpecification& domSpec)
    : domSpec(domSpec),
      actSpec(*domSpec.getActionSpecification()),
      mgr(actSpec.getManager()),
      rwdspec(*domSpec.getRewardSpecification()),
      nextReward(rwdspec.begin())
{
}

/* Perform an iteration of this algorithm */
PUBLIC bool StructuredPLTLtranslator::operator()()
{
    if (nextReward == rwdspec.end())
        return true;
    
    formula *form = nextReward->second.form;
    form->accept(this);
    delete form;
    nextReward->second.form = rhsFormula;

    nextReward++;
    
    return false;
}

/* Add behaviour for prop to all actions in the action specification
 */
PRIVATE void StructuredPLTLtranslator::addBehaviour(const proposition& prop,
                                                    ADD behaviour)
{
    ActionSpecification::ActionSpec::iterator i;
    for (i = actSpec.begin(); i != actSpec.end(); ++i) {
        ActionSpecification::CPTS& cpts = *(i->second);
        cpts[prop] = behaviour;
    }
}

PUBLIC void StructuredPLTLtranslator::visit(const conj& f)
{
    accept_(f[0]);
    string lhs = rhs;
    formula *lhsFormula = rhsFormula;
    ADD lhsADD = rhsADD;
    bool lhsValue = rhsValue;
    
    accept_(f[1]);
    rhs = "(" + lhs + " and " + rhs + ")";
    rhsFormula = new conj(lhsFormula, rhsFormula);
    rhsADD = lhsADD & rhsADD;
    rhsValue = lhsValue && rhsValue;
}

PUBLIC void StructuredPLTLtranslator::visit(const disj& f)
{
    accept_(f[0]);
    string lhs = rhs;
    formula *lhsFormula = rhsFormula;
    ADD lhsADD = rhsADD;
    bool lhsValue = rhsValue;
    
    accept_(f[1]);
    rhs = "(" + lhs + " or " + rhs + ")";
    rhsFormula = new disj(lhsFormula, rhsFormula);
    rhsADD = lhsADD | rhsADD;
    rhsValue = lhsValue || rhsValue;
}

PUBLIC void StructuredPLTLtranslator::visit(const literal& f)
{
    rhs = f.getId();
    rhsFormula = new literal(f);
    rhsADD = mgr.addVar(domSpec.getProposition(f.getId()));
    rhsValue = domSpec.getStartStatePropositions().count(rhs);
}

PUBLIC void StructuredPLTLtranslator::visit(const assLiteral& f)
{
    rhsFormula = new assLiteral(f);
    if (f.getAssignment()) {
        rhs = "tt";
        rhsADD = mgr.addOne();
        rhsValue = true;
    } else {
        rhs = "ff";
        rhsADD = mgr.addZero();
        rhsValue = false;
    }
}

PUBLIC void StructuredPLTLtranslator::visit(const iff& f)
{
    accept_(f[0]);
    string lhs = rhs;
    formula *lhsFormula = rhsFormula;
    ADD lhsADD = rhsADD;
    bool lhsValue = rhsValue;
    
    accept_(f[1]);
    rhs = "(" + lhs + " <-> " + rhs + ")";
    rhsFormula = new iff(lhsFormula, rhsFormula);
    rhsADD = lhsADD.Xnor(rhsADD);
    rhsValue = lhsValue == rhsValue;
}

PUBLIC void StructuredPLTLtranslator::visit(const imp& f)
{
    accept_(f[0]);
    string lhs = rhs;
    formula *lhsFormula = rhsFormula;
    ADD lhsADD = rhsADD;
    bool lhsValue = rhsValue;
    
    accept_(f[1]);
    rhs = "(" + lhs + " -> " + rhs + ")";
    rhsFormula = new imp(lhsFormula, rhsFormula);
    rhsADD = (~lhsADD) | rhsADD;
    rhsValue = rhsValue || !lhsValue;
}

PUBLIC void StructuredPLTLtranslator::visit(const lnot& f)
{
    accept_(f[0]);
    rhs = "~" + rhs;
    rhsFormula = new lnot(rhsFormula);
    rhsADD = ~rhsADD;
    rhsValue = !rhsValue;
}

PUBLIC void StructuredPLTLtranslator::visit(const startStateProposition& f)
{
    
}
    
PUBLIC void StructuredPLTLtranslator::visit(const prv& f)
{
    accept_(f[0]);
    rhs = "(prv " + rhs + ")";
    domSpec.addProposition(rhs);
    rhsFormula = new literal(rhs);
    addBehaviour(rhs, rhsADD);
    rhsADD = mgr.addVar(domSpec.getProposition(rhs));
    rhsValue = false;
    if (rhsValue)
        domSpec.addToStartState(rhs);
}

PUBLIC void StructuredPLTLtranslator::visit(const snc& f)
{
    accept_(f[0]);
    string lhs = rhs;
    formula *lhsFormula = rhsFormula;
    ADD lhsADD = rhsADD;
    
    accept_(f[1]);
    rhs = "(prv (" + lhs + " snc " + rhs + "))";
    domSpec.addProposition(rhs);
    // a S b: b | (a & prev(a S b))
    rhsFormula = new disj(rhsFormula,
                          new conj(lhsFormula, new literal(rhs)));
    rhsADD = rhsADD | (lhsADD & mgr.addVar(domSpec.getProposition(rhs)));
    addBehaviour(rhs, rhsADD);

    if (rhsValue)
        domSpec.addToStartState(rhs);
}

PUBLIC void StructuredPLTLtranslator::visit(const pbx& f)
{
    accept_(f[0]);
    rhs = "(prv pbx " + rhs + ")";
    domSpec.addProposition(rhs);
    // pbx a: a & prv(pbx a)
    rhsFormula = new conj(rhsFormula, new literal(rhs));
    rhsADD = rhsADD & mgr.addVar(domSpec.getProposition(rhs));
    addBehaviour(rhs, rhsADD);
    // rhsValue remains unchanged
    if (rhsValue)
        domSpec.addToStartState(rhs);
}

PUBLIC void StructuredPLTLtranslator::visit(const pdi& f)
{
    accept_(f[0]);

    rhs = "(prv pdi " + rhs + ")";
    domSpec.addProposition(rhs);
    // pbx a: a & prv(pbx a)
    rhsFormula = new disj(rhsFormula, new literal(rhs));
    rhsADD = rhsADD | mgr.addVar(domSpec.getProposition(rhs));
    addBehaviour(rhs, rhsADD);
    // rhsValue remains unchanged
    if (rhsValue)
        domSpec.addToStartState(rhs);
}

PUBLIC void StructuredPLTLtranslator::visit(const prvConj& f)
{
    int depth = f.getDepth();
    string newProp;
    accept_(f[0]);
    ADD propBehaviour = rhsADD;
    
    for (int i = 2; i <= depth; i++) {
        if (i == 2)
            newProp = "(prv " + rhs + ")";
        else
            newProp = "(prv and <" + Utils::intToString(i) + " " + rhs + ")";
        domSpec.addProposition(newProp);
        addBehaviour(newProp, rhsADD);
        rhsADD = mgr.addVar(domSpec.getProposition(newProp)) & propBehaviour;
    }
    rhsFormula = new literal(newProp);
    rhs = newProp;
    rhsValue = false;
}

PUBLIC void StructuredPLTLtranslator::visit(const prvDisj& f)
{
    int depth = f.getDepth();
    string newProp;
    accept_(f[0]);
    ADD propBehaviour = rhsADD;
    
    for (int i = 2; i <= depth; i++) {
        if (i == 2)
            newProp = "(prv " + rhs + ")";
        else
            newProp = "(prv or <" + Utils::intToString(i) + " " + rhs + ")";
        domSpec.addProposition(newProp);
        addBehaviour(newProp, rhsADD);
        rhsADD = mgr.addVar(domSpec.getProposition(newProp)) | propBehaviour;
    }
    rhsFormula = new literal(newProp);
    rhs = newProp;
    // rhsValue remains unchanged
    rhsValue = false;
}

PUBLIC void StructuredPLTLtranslator::visit(const prvNest& f)
{
    int depth = f.getDepth();
    string newProp;
    accept_(f[0]);

    if (depth > 0) {
        for (int i = 1; i <= depth; i++) {
            if (i == 1)
                newProp = "(prv " + rhs + ")";
            else
                newProp = "(prv^" + Utils::intToString(i) + " " + rhs + ")";
            domSpec.addProposition(newProp);
            addBehaviour(newProp, rhsADD);
            rhsADD = mgr.addVar(domSpec.getProposition(newProp));
        }
        rhsFormula = new literal(newProp);
        rhs = newProp;
    }
    if (1 == depth)
        rhsValue = rhsValue;
    else
        rhsValue = false;
    if (rhsValue)
        domSpec.addToStartState(rhs);
}
