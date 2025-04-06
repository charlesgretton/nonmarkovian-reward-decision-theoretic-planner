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

#ifndef FORMULA_UTILITIES_template
#define FORMULA_UTILITIES_template

#include"formulaUtilities.h++"

namespace Formula
{
    template<class Index, class unary_function>
    void descendAggregate(double reward,
                          formula const* form,
                          unary_function funct)
    {
        funct(reward, form);

        /*Cast \argument{form} into an aggregate.*/
        aggregate<formula*, Index> const* aggregateOf =
            dynamic_cast<aggregate<formula*, Index> const*>(form);

        /*If the \argument{form} is an aggregate.*/
        if(0 != aggregateOf)
            descendAggregate/*Continue descent.*/
                <Index, AccumulateFormula>
                (reward, aggregateOf, funct);
    }
    
    /*Applies the argument unary_function to all sub--elements of the
     *aggregate.*/
    template<class Index, class unary_function>
    void descendAggregate(double value,
                          aggregate<formula*, Index> const* e,
                          unary_function funct)
    {
        /*We assume the argument \class{aggregate} is a formula.*/
        formula* tmp = dynamic_cast<formula const*>(e)->copy();
        if(0 == tmp)assert(0);
        
        summaryUnary* summary
            = dynamic_cast<summaryUnary*>(tmp);

        /*If the argument is a summary operator.*/
        if(0 != summary)
            if(--(*summary))/*Is the subformula of the summary a summary?*/
            { 
                descendAggregate<Index, AccumulateFormula>(0.0, tmp, funct);
                delete tmp;
                return;
            }

        delete tmp;

        /*In the case that the argument was a normal aggregate.*/
        for(Index i = e->begin()
                ; i != e->end()
                ; ++i)
            descendAggregate<Index, AccumulateFormula>(0.0, (*e)[i], funct);
    }
}
#endif

