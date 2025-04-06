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

#include"PhaseI.h++"
#include"domainSpecification_Anytime_or_Explicit.h++"

using namespace MDP;
using namespace Formula;

/******************************************************************/

RewardSpecification* Subformulas::operator()
    (const explicitDomainSpecification& domSpec)const
{   
    /*Reward associated with the argument domain.*/
    RewardSpecification const* rewSpec = domSpec.getRewardSpecification();

    /*Formula accumulator.*/
    AccumulateFormula accumulateFormula;
    
    /*For each formula component of the reward specification.*/
    for(RewardSpecification::const_iterator reward = rewSpec->begin()
            ; reward != rewSpec->end()
            ; ++reward)
        {
            /*Accumulate the set of sub--formulae.*/
            descendAggregate<OperatorIndexType, AccumulateFormula&>
                (reward->second.value,
                 reward->second.form,
                 accumulateFormula);
        }
    
    return accumulateFormula.getSpecification();
}
/******************************************************************/

RewardSpecification* Minimal::operator()
    (const explicitDomainSpecification& domSpec)const
{
    /*Reward associated with the argument domain.*/
    RewardSpecification const* rewSpec = domSpec.getRewardSpecification();

    /*The new state reward.*/
    RewardSpecification* stateReward = new RewardSpecification();
    
    /*For each formula component of the reward specification.*/
    for(RewardSpecification::const_iterator reward = rewSpec->begin()
            ; reward != rewSpec->end()
            ; ++reward)
    {
        string* label = reward->second.form->print();
        
        RewardSpecification *tmp
            = stateReward->addSpecComponent(*label,
                                            reward->second.value,
                                            reward->second.form->copy());
        
        
        /*If a change was made to the reward specification.*/
        if(0 != tmp)
        {
            /*Remove the old reward type.*/
            if(stateReward != tmp)
            {
                delete stateReward;
                
                /*Keep the new reward type.*/
                stateReward = tmp;
            }
        }
        
        delete label;
    }

    return stateReward;
}

