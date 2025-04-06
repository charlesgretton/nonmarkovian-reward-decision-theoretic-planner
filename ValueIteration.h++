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
 * Dynamic programming algorithm for optimal policy construction for
 * MDPs. This algorithm is given in \cite{boutilier00}.
 * */
#ifndef VALUE_ITERATION
#define VALUE_ITERATION

#include "StateBasedSolutionTypes.h++"
#include "Algorithm.h++"

using namespace mtl;
using namespace std;

namespace MDP
{
    class ValueIteration : public Algorithm
    {
    public:
        /*Value iteration can be used as the dynamic programming step
         *in \class{LAO}.*/
        friend class LAO<ValueIteration>;

        /*A call to this constructor results in erroneous behaviour as
         *this is a state based algorithm (ie: requires an
         *\class{explicitDomainSpecification}).*/
        ValueIteration(DomainSpecification* domSpec);

        /*Construct the algorithmic object with \argument{domSpec} as
         *the specification on which the algorithms is to be executed,
         *\argument{gamma} the discount factor and \argument{epsilon}
         *the acceptable error.
         *
         *A client who desires finer grained control over the
         *configuration of this \class{Algorithm} should pass an
         *\argument{delayInitialisation} value of $true$.*/
        ValueIteration(explicitDomainSpecification& domSpec,
                       double gamma,
                       double epsilon,
                       bool delayInitialisation = false);

        /*Construct the algorithmic object with \argument{domSpec} as
         *the specification on which the algorithms is to be
         *executed. Discounting and error is taken to be the defaults
         *provided by the \parent{Algorithm}.*/
        ValueIteration(explicitDomainSpecification& domSpec);
          
        /*Set the algorithm policy to that of the
         *\argument{explicitDomainSpecification}*/
        void configurePolicy(explicitDomainSpecification&);
        
        /*Initialise \member{lastEpoch} value to state values
         *\member{State.getValue()} and \member{initialReward} to
         *\member{domSpec} \class{State} immediate reward
         *\member{State.getReward()}.
         *
         *This function also accommodates state colourings. If states
         *are coloured as either FRINGE or IMPLICIT then the immediate
         *reward is considered to be \member{State.getValue()}.*/
        void initialiseValueVectors();

        /*This has the effect of initialising the system dynamics
         *\member{actionMatrices}.*/
        void configureActions(vector<eState*>::const_iterator domainState, bool ignoreExplicit = true);

        /*Assigns integer identities to all the \member{domSpec}
         *states. The resultant mapping from states an integers is
         *available in \member{stateIds}*/
        void identifyStates(vector<eState*>::const_iterator domainState);

        /*See \member{configureActions}.*/
        void configureActions(bool ignoreExplicit = true);

        /*See \member{identifyStates}.*/
        void identifyStates();

        /*Initialise the sparse \member{actionMatrices}, such that
         *they have the appropriate dimensions.*/
        void initialiseActions();

        /*Update the value associated with elements of
         *\member{domainStates} to include the n--epoch values
         *resulting from the last iteration of this algorithm. Only
         *$MARKED$ states are updated.*/
        void updateStateValues();
        
        /*Has this algorithm found a solution with suitable error? The
         *arguments are the current and last epoch
         *\class{ValueVectors} respectively. \member{Algorithm::error}
         *is updated when a call is made. */
        bool terminate(const ValueVector thisEpoch, const ValueVector lastEpoch);

        /*Execute an iteration of this algorithm.*/
        bool operator()();
    private:
        /*Sequence of \class{eState}s that are being iterated.*/
        vector<eState*> domainStates;

        /*Index of states, for the purpoose of iteration.*/
        map<eState*, int> stateIds;

        /*System dynamics*/
        ActionMatrices actionMatrices;

        /*Last epoch value function.*/
        ValueVector lastEpoch;

        /*This epoch value function.*/
        ValueVector thisEpoch;

        /*Initial reward.*/
        ValueVector initialReward;

        /*Epoch number.*/
        int epoch;

        /*Current policy.*/
        Policy* policy;

        /*Next identification number to be given to a state.*/
        int stateId;
    };

};



#endif
