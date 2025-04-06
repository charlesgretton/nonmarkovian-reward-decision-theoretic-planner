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

#include"rewardSpecification.h++"
#include"formulaSequenceTraversal.h++"
#include"RewardCalculation.h++"

using namespace MDP;
using namespace Formula;

FLTLrewardSpecification::FLTLrewardSpecification(const FLTLrewardSpecification& rs)
    :RewardSpecification(rs)
{}

FLTLrewardSpecification::FLTLrewardSpecification(const RewardSpecification& rs)
    :RewardSpecification(rs)
{}

RewardSpecification* FLTLrewardSpecification::expandsTo()const
{
    return new PLTLrewardSpecification(*this);
}

double FLTLrewardSpecification::sequenceTraversal
(const DomainSpecification::PropositionSet& propositionSet,
 const BuildFilter<formula>& buildFilter)
{
    FLTLprogression<DomainSpecification::PropositionSet const> progressor;
    return RewardSpecification::sequenceTraversal(progressor,
                                                  propositionSet,
                                                  buildFilter);/*Spec may change.*/
}

RewardSpecification* FLTLrewardSpecification::addSpecComponent(string str, double d, formula* f)
{
    /*If no expansion is required.*/
    if(true == expansionRequired
       && formula_treeCast<FLTLformula>(f))
        expansionRequired = false;
    
    return RewardSpecification::addSpecComponent(str, d, f);
}

bool FLTLrewardSpecification::isPossible()const
{
    /*We shall be checking for formula in the specification which equal
      $false$.*/
    RewardSpecification::FormulaEqual<assLiteral> formulaEqual;

    /*Find a false element in the specification.*/
    crIterator p = find_if(specificationContents.begin(), 
                           specificationContents.end(), 
                           bind2nd(formulaEqual, assLiteral(false)));

    /*If no $false$ elements were present in the specification.*/
    if(p == specificationContents.end())
        return true;
    else
        return false;
    
}

double FLTLrewardSpecification::getReward(bool rewarding, 
                                          RewardSpecification::rIterator &p,
                                          formula* build)const
{
    /*If the traversed formula contained a reward proposition
      ($dollars$).*/
    if(formula_treeCast<dollars>(p->second.form)
       /*and progression yielded $false$.*/
       && assLiteral(false) == (*build))
        throw RewardAbnormality(p);/*Reward was non-normal.*/

    return RewardSpecification::getReward(rewarding, p, build);
}

RewardSpecification* FLTLrewardSpecification::copy()const
{
    return new FLTLrewardSpecification(*this);
}

RewardCalculator* FLTLrewardSpecification::getRewardCalculator()const
{
    return new FLTLrewardCalculator;
}

