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
 * Utilities used for \class{formula} manipulation.
 * */
#ifndef FORMULA_UTILITIES
#define FORMULA_UTILITIES

#include"rewardSpecification.h++"
#include"formula.h++"

using namespace MDP;
using namespace Formula;

namespace Formula
{
    /*Accumulation generates a mapping \member{result} that contains
     *string indexed maplets to handles of copies of traversed object
     *aggregate formulae. Moreover these are contained in the
     *traversal friendly \class{RewardSpecification} container indexed
     *by the string representation of the corresponding range element.
     */
    class AccumulateFormula
    {
    public:
        /*Initialise the \class{RewardSpecification} \member{result} on
         *the heap.
         *
         *\textbf{NOTE:} Deletion of \member{result} is not the
         *responsibility of \class{AccumulateFormula}.
         **/
        AccumulateFormula();
        
        /*\textbf{NOTE:} Copy construction does not copy the reward
         *specification of argument accumulator hence the
         *non--constness of argument.*/
        AccumulateFormula(AccumulateFormula&);

        /*\member{result} is assigned to \argument{RewardSpecification}.*/
        AccumulateFormula(RewardSpecification*);
        
        /*Adjusts the \member{result} to take into consideration the
         *argument subformulae as follows:
         *
         *\begin{itemize}
         *
         *\item If the formula doesn't yet feature in the
         *\member{result} it is added and indexed at the label which
         *is a string representation of the \argument{formula}.
         *
         *\item \argument{double} is assumed to be the reward
         *associated with \argument{formula}.
         *
         *\end{itemize}*/
        void operator()(double, formula const*);

        /*Obtain a handle to the resulting specification.*/
        RewardSpecification* getSpecification();
    private:
        /*Resulting specification.*/
        RewardSpecification* result;
    };
    
    /*Applies the \argument{function} to all \argument{formula}
     *sub--elements inclusive of the \argument{formula}. The
     *\argument{function} takes a reward pair $(reward, formula)$ (see
     *\class{AccumulateFormula}) as arguments to the
     *accumulation. These correspond to the \argument{double} and
     *\argument{formula} passed to this function.
     *
     *In the case that the \argument{formula} is an
     *\class{aggregate<formula*, Index>} a call is made to
     *\function{descendAggregate()} on an \class{aggregate<formula*,
     *Index>} cast of the \argument{formula}. This call simply passes
     *other argument data as is.
     **/
    template<class Index, class function>
    void descendAggregate(double, formula const*, function);

    /*Applies the \argument{function} to all \class{formula}
     *sub--elements of the \argument{aggregate}. Further descent
     *ignores the \argument{double} argument and substitutes
     *$0.0$. Subformulae do not have reward associated with them.*/
    template<class Index, class function>
    void descendAggregate(double, aggregate<formula*, Index> const*, function);
}

#include"formulaUtilities_templates.h++"

#endif
