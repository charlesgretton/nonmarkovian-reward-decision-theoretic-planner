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

/*
 * \paragraph{:Purpose:}
 *
 * This module contains traversal friendly \class{BuildFilter}s.
 **/

#ifndef ENTAILMENT_FILTER
#define ENTAILMENT_FILTER

#include"formulaTypes.h++"
#include"SpecificationTypes.h++"
#include"formulaVisitation.h++"

using namespace Formula;

namespace MDP
{
    /*This can be used in Phase II of the annotated expansion
     *algorithm to establish which atom suits a newly expanded
     *state.
     *
     *The filter is designed to act during traversal (see
     *\class{PLTLprogression}).*/
    class EntailmentFilter : public BuildFilter<formula>
    {
    public:
        EntailmentFilter(RewardSpecification const*);

        /*Translates the argument formula into
         *\class{assLiteral(true)} or \class{assLiteral(false)}
         *literals depending on whether or not the
         *\member{rewardSpecification} entails the formula. If the
         *argument is non--temporal then the result is the argument.
         *
         *The \argument{form} is deleted on translation.*/
        formula* operator()(formula* form, bool lastChance = false)const;
    protected:
        /*This is called by \method{operator()} when the filter is
         *deemed applicable. Filters may distinguish between
         *entailment during and after traversal.*/
        formula* applyFilter(formula* form)const;
        
        /*The specification which contains the subformula that
         *are considered valid by this filter.*/
        RewardSpecification const* rewardSpecification;
    };


    class MinimalEntailmentFilter : public EntailmentFilter
    {
    public:
        MinimalEntailmentFilter(RewardSpecification const*);
        
        /*Translates the argument formula into
         *\class{assLiteral(true)} or \class{assLiteral(false)}
         *literals depending on whether or not the
         *\member{rewardSpecification} entails the formula.
         *
         *The \argument{form} is deleted on translation.*/
        formula* operator()(formula* form, bool lastChance = false)const;
    };
    
    /*An entailment filter for a starting state such as
     *\member{explicitDomainSpecification.startState}. As with its
     *parent this class is for use with \class{PLTLprogression}.*/
    class ZeroPredecessor : public EntailmentFilter
    {
      public:
        ZeroPredecessor();
        
        /*Translates the argument formula into
         *\class{assLiteral(true)} or \class{assLiteral(false)}
         *literals depending on whether or not the temporal operator
         *argument can be true if a state had no predecessors. If the
         *argument is non--temporal then the result is the argument.
         *
         *The \argument{form} is deleted on translation.*/
        formula* operator()(formula* form, bool lastChance = false)const;
    };
}
#endif
