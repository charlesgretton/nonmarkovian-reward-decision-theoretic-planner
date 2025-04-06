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

#ifndef FORMULA_SPECIFICATION_template
#define FORMULA_SPECIFICATION_template

namespace MDP
{ 
    /* export */ template<class Traverser, typename Propositions>
    double RewardSpecification::sequenceTraversal
    (Traverser& traverser,
     const Propositions& propositions,
     const BuildFilter<formula>& buildFilter)
    {    
        clearRewardCache();/*Spec may change.*/
    
        /*Total reward obtained by sequence traversal.*/
        double totalReward = 0.0;
	
        /*Configure the traverser with state characterising
          propositions.*/
        traverser.setPropositionContainer(&propositions);

        /*Configure the building filter for traversal.*/
        traverser.setBuildFilter(&buildFilter);
            
        /*Traverse all specification reward formulae.*/
        for(RewardSpecification::rIterator p = begin()
                ; p != end()
                ; ++p)
	    {  
            traverser.initBuild();
            (p->second.form)->accept(&traverser);/*Traverse the formula*/
		
            try{/*Reward establishment may lead to exceptional
                  circumstances if the rewards specified were
                  abnormal.*/

                /*Reward established through traversal can change
                  after rewardability has been checked.*/
                bool rewarding = traverser.rewarding();
                
                totalReward += getReward(rewarding, 
                                         p, 
                                         traverser.getBuild());
            }catch(...)
            {
                delete traverser.getBuild();
                throw;
            }
            
            delete p->second.form;
            p->second.form = traverser.getBuild()->simplify();/*Simplify the result of traversal*/
            
            delete traverser.getBuild();
	    }

        return totalReward;
    }
    
    template<typename F>
    bool RewardSpecification::FormulaEqual<F>::operator()
        (const SpecificationContainer::value_type& rSpec, const F& f) const
    {
        return f == *rSpec.second.form;
    }
}
#endif
