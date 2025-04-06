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
 * Non--structured \class{Algorithms} such as \class{LAO},
 * \class{ValueIteration}, \class{PolicyIteration}, etc\ldots require
 * that the state space of the problem domain be explicit. This module
 * contains the mechanisms for expanding a \emph{State} given a
 * \emph{DomainSpecification}.
 **/

#ifndef EXPANSION
#define EXPANSION

#include"domainSpecification.h++"
#include"States.h++"

using namespace std;

namespace MDP
{
    /*Predicate that returns $true$ if the \argument{TransitionPair}
     *occurs with zero probability.*/
    class ZeroProbability : public unary_function<const TransitionPair&,
                            bool>
    {
    public:
        bool operator()(const TransitionPair&) const;
    };
    
/******************************************************************/
    
    /*Free heap memory taken by \argument{TransitionPair} where the reward
     *associated with that pair occurs with zero probability.*/
    class RemoveWithZeroProbability : public unary_function<const TransitionPair&,
                                      void>
    {
    public:
        void operator()(TransitionPair& pair) const;
    };
    
/******************************************************************/

    /*Class for state expansion see \method{operator()}.*/
    class Expansion
    {
    public:
        /*Expands the \argument{State} given the
         *\argument{DomainSpecification}.*/
        virtual StateTransitionMatrices*
        operator()(State&,
                   DomainSpecification&,
                   const StateLabelling& nmrsLabels = StateLabelling()) const;
    protected:
        /*State \method{State.copy()}'d and \method{State.clear()}'d
         *during \function{newState()} generation.*/
        mutable State* stateLastToExpand;

        /*The \argument{ActionPossibilities} contain the probabilities
         *and \class{eStates} associated with some state transition in
         *this domain. The \argument{StateLabelling} comprises the
         *reward labels for grounded or NMRDP states. This method
         *calculates the reward associated with the
         *\argument{ActionPossibilities} \class{eState}s assuming that
         *their current \class{rewardSpecification}s equal those of
         *their parents.*/
        virtual void adjustStates(ActionPossibilities&, const StateLabelling&) const;
        
        /*Generates the \argument{ActionPossibilities} such that no
         *resultant element in that vector occurs with zero
         *probability.*/
        void produceTransitions(const DomainSpecification&,
                                const vector<double>&,
                                ActionPossibilities&) const;

        /*Where the \argument{node} is an ADD action representation, this
         *function returns the probability of the action resulting in a state
         *in which the \argument{propositions} are true.*/
        double getProbability(DdNode const* node,
                              Cudd const* _cudd,
                              const DomainSpecification::PropositionIndexVector&) const;
        
        /*Returns a new state based on that which was an argument to
         *the last \method{operator()} call.*/
        State* newState() const;
    };
    
/******************************************************************/
    
    /*An expansion which labels states.*/
    class StateAnnotationExpansion : public Expansion
    {
    protected:
        void adjustStates(ActionPossibilities&, const StateLabelling&) const;
    };

/******************************************************************/
}

#endif


