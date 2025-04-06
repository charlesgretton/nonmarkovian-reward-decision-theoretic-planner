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
 * see \class{LAO}. The \class{LAO} algorithm is given in
 * \cite{hansen}.
 *
 * */

#ifndef LAO_H
#define LAO_H

#include "StateBasedSolutionTypes.h++"
#include "Algorithm.h++"
#include<stack>
#include<set>
//#include<vector>

using namespace std;

namespace MDP
{
    /*\parent{Algorithm} implementation of anytime state based policy
     *and value iteration using $LAO*$\cite{hansen}.
     *
     *As with the other state space algorithms, this requires that the
     *domain be preprocessed (see \module{Preprocessors} and
     *\module{PhaseI}) to configure the fringe of the argument
     *\member{explDomSpec}. However, an argument domain should not be
     *expanded (see \module{domainSpecification_Anytime_or_Explicit}
     *and \module{Expansion}) as a fringe is required.
     *
     *This class has access to both explicit value and policy
     *iteration members (see \module{PolicyIteration} and
     *\module{ValueIteration}). It is their friend.
     **/
    template<class ExplicitAlgorithm>
    class LAO : public Algorithm
    {  
    public:
        typedef vector<eState*> SuccesorList;

        typedef vector<SuccesorList*> ExpansionStack;
        
        /*A call to this constructor results in erroneous behaviour as
         *this is a state based algorithm.*/
        LAO(DomainSpecification* domSpec);

        /*Construct the algorithmic object with \argument{domSpec} as
         *the specification on which the algorithm is to be executed,
         *\argument{gamma} the discount factor and \argument{epsilon}
         *the acceptable error.*/
        LAO(explicitDomainSpecification&,
            double gamma,
            double epsilon,
            bool useControlKnowledge = false);
        
        /*Construct the algorithmic object with \argument{domSpec} as
         *the specification on which the algorithm is to be
         *executed. Discounting and error is taken to be the defaults
         *provided by the \parent{Algorithm}.*/
        LAO(explicitDomainSpecification& domSpec, bool useControlKnowledge = false);
        
        /*Perform one expansion of the state space and execute Bellman
         *backup on the resulting explicit graph.*/
        bool operator()();

        /*Error between the current policy and the optimal policy of
         *the explicit state space.*/
        double getError()const;
    private:
        /*This is called when a fringe node is found. At this point,
         *the only states that have been added to the graph which is
         *to be passed to the dynamic programming algorithm, are
         *reachable when executing the $policy$. This function will
         *mark as EXPLICIT all IMPLICIT states which can reach an
         *EXPLICIT state in this graph. Thus, where an IMPLICIT node
         *is made EXPLICIT its successors are added to the graph
         *(again these may need to be marked as EXPLICIT).*/
        void completeExplicitGraph(std::set<eState*>& allStatesSet);
        
        /*This function expands the best solution graph searching for
         *a fringe state. $false$ is returned if no fringe is found. States
         *from which the search begins comprise those in the back of the
         *\argument{expansionStack}.*/
        bool forwardSearch(Policy* policy,
                           ExpansionStack& expansionStack,
                           std::set<eState*>& allStatesSet);

        /*Two vectors of \class{eState}s and one \class{eState} are
         *returned. In declaration order these will correspond to all
         *the states that are reachable from \argument{eState} on
         *execution of action \argument{action}, all the states that
         *are reachable from \argument{eState} on execution of all
         *possible action \textbf{OTHER THAN} \argument{action}, and a
         *state which was found to be coloured as specified by
         *\argument{int}.
         *
         *This function can never have a $FRINGE$ \argument{eState} as
         *this argument is assumed to have already be expanded. For
         *such cases see \function{expandFringeNode}.*/
        pair<pair<SuccesorList*, SuccesorList*>* , eState*> successorsOnAction(action act, eState*, int = FRINGE);

        /*When an \argument{eState} is expanded all successor nodes
         *are added to the \argument{std::set<eState*>}. The return
         *value contains a container of states which are
         *\argument{eState} successors but were not elements of the
         *\argument{std::set<eState*>}.
         *
         *This function can only expand $FRINGE$ states.*/
        SuccesorList* expandFringeNode(eState*, std::set<eState*>&);

        /*\class{eState} from which the explicit graph is expanded.*/
        eState* startState;

        /*A heuristic value to give to all fringe states.*/
        double expectedFringeReward;

        /*Domain for which a policy is computed.*/
        explicitDomainSpecification* explDomSpec;

        /*If the optimal policy yields no FRINGE states during
         *computation of the explicit graph, then we repeat the
         *traversal a second time. This second time, all states that
         *are ever placed on the expansion stack are left on the stack
         *where previously they would be removed. This allows the
         *dynamic programming step to make conclusions as to the
         *current policy even where a FRINGE state can not be found.*/
        bool keepEverything;
        
        /*Current policy. This is a handle on the
         *\member{explDomSpec.policy}*/
        Policy* policy;

        /*Previous policy.*/
        Policy oldPolicy;

        /*Dynamic programming algorithm to be used to calculate a
         *policy.*/
        ExplicitAlgorithm dynamicProgrammingStep;

        /*Should control knowledge be used?*/
        bool useControlKnowledge;
        
        /*Has use of control knowledge altered the state space during
         *a call to \method{operator()()}.*/
        bool controlKnowledgeUsed;
    };
}

#include"LAO_templates.h++"

#endif


