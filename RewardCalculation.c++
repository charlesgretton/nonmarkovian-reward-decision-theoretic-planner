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

#include"RewardCalculation.h++"

#include"rewardSpecification.h++"
#include"EntailmentFilter.h++"

using namespace MDP;

double FLTLrewardCalculator::operator()
    (pair<RewardSpecification**, RewardSpecification*> rewardSpecs,
     const DomainSpecification::PropositionSet& propositions,
     BuildFilter<formula>* buildFilter)
{
    RewardSpecification* rewardSpec = *rewardSpecs.first;
    
    try{
        return rewardSpec->sequenceTraversal(propositions);
    }catch(RewardSpecification::RewardAbnormality ex)
    {
        cerr<<ex.getMessage();
        assert(0);
    }
}

double PLTLrewardCalculator::operator()
    (pair<RewardSpecification**, RewardSpecification*> rewardSpecs,
     const DomainSpecification::PropositionSet& propositions,
     BuildFilter<formula>* buildFilter)
{
    /*Reward associated with the caller states predecessor.*/
    RewardSpecification* predecessorRewardSpec = *rewardSpecs.first;

    /*Set of atoms.*/
    RewardSpecification* labelSet = rewardSpecs.second;

    /*Copy of the set of atoms.*/
    RewardSpecification* labelSetCopy = rewardSpecs.second->copy();

    BuildFilter<formula>* entailmentFilter = buildFilter;
    
    /*If the state did not have a predecessor and we have not been provided
      with an entailment filter.*/
    if(0 == predecessorRewardSpec && 0 == entailmentFilter)
        entailmentFilter = new ZeroPredecessor();
    /*Else if we are trying to build the state space with no regard for size.*/
    else if(0 == entailmentFilter)
        /*Create an entailment filter based on $predecessorRewardSpec$.*/
        entailmentFilter = new EntailmentFilter(predecessorRewardSpec);

    /*Regress the atom copy over the \class{State}
      \argument{propositions} applying the $entailmentFilter$.*/
    double reward =
        labelSetCopy->sequenceTraversal(propositions, *entailmentFilter);
    
    /*Parents reward specification is no longer needed (NULL is OK).*/
    delete *rewardSpecs.first;
    
    /*The new reward specification is the same length as the $labelSet$.*/
    *rewardSpecs.first = labelSet->copy();
    predecessorRewardSpec = *rewardSpecs.first;

    /*Configure the new reward specification according to the results
      of the $labelSetCopy$ traversal. Every specification that we
      traverse below _IS_ the same size.*/
    RewardSpecification::const_iterator labelRegrElement;
    RewardSpecification::iterator rewardElement;
    RewardSpecification::const_iterator labelElement;
    for(labelRegrElement = labelSetCopy->begin(),
            rewardElement = predecessorRewardSpec->begin(),
            labelElement = labelSet->begin()
            ; rewardElement != predecessorRewardSpec->end()
            ; ++labelRegrElement, ++rewardElement, ++labelElement)
    {
        formula* tmp;
        
        /*If the formula regresses to $false$.*/
        if(assLiteral(false) == *labelRegrElement->second.form)
            tmp = new lnot(labelElement->second.form->copy());
        else/*Else the formula regressed to $true$.*/
            tmp = labelElement->second.form->copy();

        /*Delete the predecessor reward element*/
        delete rewardElement->second.form;

        /*and replace it with the new calculated
          reward. Simplification should be of no use at this point.*/
        rewardElement->second.form = tmp;//->simplify();
        
    }

    delete labelSetCopy;
    
    return reward;
}
