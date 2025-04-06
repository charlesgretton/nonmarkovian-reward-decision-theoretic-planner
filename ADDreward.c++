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

/* ADDreward.cpp
 *
 * \paragraph{:purpose:}
 *
 * Generates an ADD representing the reward as specified in the domain
 * specification (which stores it as a set of formulas).  The reward
 * specification must be non-temporal before this class is used.
 */

#include "common.h++"
#ifdef HEADER

#include "formulaVisitation.h++"
#include "formula.h++"
#include "rewardSpecification.h++"
#include "SpecificationTypes.h++"
#include "domainSpecification.h++"
#include "ExtADD.h++"
#include "Diagram.h++"

using namespace Formula;
using namespace MDP;

class ADDreward : public PCvisitor
{
public:
    virtual ~ADDreward()
    {
    }
    
private:
    DomainSpecification& domSpec;
    ActionSpecification& actSpec;
    Cudd& mgr;
    
    ExtADD rhs;
    bool haveValidRHS;
    ExtADD value;
    ExtADD constraint;
};

#endif
#include "ADDreward.h++"

/* Produces an ADD representing the reward stored in `domSpec'
 */
PUBLIC ADDreward::ADDreward(DomainSpecification& domSpec)
    : domSpec(domSpec),
      actSpec(*domSpec.getActionSpecification()),
      mgr(actSpec.getManager()),
      rhs(mgr.addZero()),
      value(mgr.addZero()),
      constraint(mgr.addOne())
{
    RewardSpecification *rwdspec = domSpec.getRewardSpecification();
    RewardSpecification::rIterator i;
    for (i = rwdspec->begin(); i != rwdspec->end(); ++i) {
        formula *form = i->second.form;
        haveValidRHS = false;
        form->accept(this);
        if (haveValidRHS) {
            if (i->second.value == 0.0) {
                // rewards with a value of 0.0 are constraints

                constraint &= rhs;
            } else {
                //rhs.display(domSpec.getPropositions(), vector<string> (), "partial reward");
                rhs &= constraint;
                rhs *= mgr.constant(i->second.value);
                if (rwdspec->begin() == i)
                    value = rhs;
                else
                    value += rhs;
            }
        } else {
            cerr << "No valid reward formula found\n";
        }
    }
}

/* Get the reward ADD
 */
PUBLIC ExtADD ADDreward::getReward()
{
    return value;
}

/* Get the ADD for the constraints
 */
PUBLIC ExtADD ADDreward::getConstraint()
{
    return constraint;
}

/* Get the reward diagram
 */
PUBLIC Diagram ADDreward::getRewardDiagram()
{
    Diagram result;
    result.setNodeNames(domSpec.getPropositions());
    result.addPage(value, "Reward specification");
    return result;
}

/* Get the diagram for the constraints
 */
PUBLIC Diagram ADDreward::getConstraintDiagram()
{
    Diagram result;
    result.setNodeNames(domSpec.getPropositions());
    result.addPage(constraint, "Constraint specification");
    return result;
}

void ADDreward::visit(const conj& f)
{
    accept_(f[0]);
    ADD lhs = rhs;
    accept_(f[1]);
    rhs = lhs & rhs;
}

void ADDreward::visit(const disj& f)
{
    accept_(f[0]);
    ADD lhs = rhs;
    accept_(f[1]);
    rhs = lhs | rhs;
}

void ADDreward::visit(const literal& f)
{
    rhs = mgr.addVar(domSpec.getProposition(f.getId()));

    // we set this here, since every valid non-empty formula must have
    // at least one literal
    haveValidRHS = true;
}

void ADDreward::visit(const assLiteral& f)
{
}

void ADDreward::visit(const iff& f)
{
    accept_(f[0]);
    ADD lhs = rhs;
    accept_(f[1]);
    rhs = lhs.Xnor(rhs);
}

void ADDreward::visit(const imp& f)
{
    accept_(f[0]);
    ADD lhs = rhs;
    accept_(f[1]);
    rhs = (~lhs) | rhs;
}

void ADDreward::visit(const lnot& f)
{
    accept_(f[0]);
    rhs = ~rhs;
}
