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

#include"EntailmentFilter.h++"
#include"rewardSpecification.h++"
#include"formula.h++"

using namespace MDP;

EntailmentFilter::EntailmentFilter(RewardSpecification const* rewardSpec)
    :rewardSpecification(rewardSpec)
{}

formula* EntailmentFilter::operator()(formula* form, bool lastChance)const
{
    /*If this is not the result of visitation then apply this filter.*/
    if(!lastChance)
        return applyFilter(form);

    return form;
}

formula* EntailmentFilter::applyFilter(formula* form) const
{
    string* formStr;
    assLiteral* answer = 0;

    if(0 != dynamic_cast<startStateProposition*>(form))
        {
            answer = new assLiteral(false);
            delete form;
            return answer;
        }

    formStr = form->print();
    
    RewardSpecification::const_iterator element
        = rewardSpecification->find(*formStr);
    
    /*Does the element appear in the \member{rewardSpecification}?*/
    if(element != rewardSpecification->end())
    {   
        /*If \argument{form} is the same as that to which it corresponds
          in \member{rewardSpecification}.*/
        if((*form) == *element->second.form)
        {
            answer = new assLiteral(true);
        }   
    }
    /*If the answer has yet to be decided.*/
    if(0 == answer)
    {
        answer = new assLiteral(false);
    }
    delete formStr;
    delete form;
    return answer;
}

/****************************************************/

MinimalEntailmentFilter::MinimalEntailmentFilter(RewardSpecification const* rewardSpec)
    :EntailmentFilter(rewardSpec)
{}

formula* MinimalEntailmentFilter::operator()(formula* form, bool lastChance)const
{
    /*If this is not the result of visitation then do not apply a filter.*/
    if(!lastChance)
        return form;
    
    return applyFilter(form);
}

/****************************************************/
ZeroPredecessor::ZeroPredecessor()
    :EntailmentFilter(0)
{}

formula* ZeroPredecessor::operator()(formula* form, bool lastChance)const
{
    /*This filter does not act in a \argument{lastChance} situation.*/
    if(!lastChance)
    {
        PLTLformula* tmp = dynamic_cast<PLTLformula*>(form);
        
        if(0 != tmp)/*If the \argument{formula} is a \class{PLTLformula}.*/
            {
                formula* result = tmp->startStateAssignment();
                delete tmp;
                return result;
            }
        /*If a \class{literal} is required to be $true$ in the previous
        state and there is no previous state, then regression
        gives that literal a value of $false$.

        else if(0 != dynamic_cast<literal*>(form))
            {
                delete form;
                return new assLiteral(false);
            }
        
        This applies equally well to any formula whose main connective
        is non--temporal.  Recall that this encapsulation of a test
        for filtering acts on temporal formulae and the arguments to
        the unary next/previously modalities. Anything that is
        operated on by such a modality in the first state we
        "consider" to be $false$. In fact, although most would consider
        $~ \prv a$ to be true in the initial state, we argue,
        appealing to undefinedness, that this is $false$ for the
        purposes of this application.*/
        else
        {   
            delete form;
            return new assLiteral(false);
        } 
    }
        
    return form;
}

