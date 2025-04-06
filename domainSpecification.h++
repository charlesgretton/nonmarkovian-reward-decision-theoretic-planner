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
 * Representation of and operations concerning NMRDPs.
 * 
 */
#ifndef DOMAIN_SPEC
#define DOMAIN_SPEC

#include"formula.h++"
#include"SpecificationTypes.h++"
#include"actionSpecification.h++"

using namespace std;
using namespace Formula;

namespace MDP
{ 
    class DomainSpecification
    {
    public:
        /*Containment type for state characterising propositions.*/
        typedef vector<proposition> PropositionVector;

        /*Type used for indexing propositional variables in ADDs.*/
        typedef unsigned int propositionIndex;

        /*Set containment for propositional indices.*/
        typedef vector<propositionIndex> PropositionIndexVector;

        /*Set containment type for state characterising propositions.*/
        typedef std::set<proposition> PropositionSet;

        /*Domain construction achieves the following:
         *
         *\begin{itemize}
         *
         *\item Initialisation of \member{rewardSpecification}.
         *
         *\item Initialisation of \member{actionSpecification}.
         *
         *\item Initialisation of responsibility for action
         *specification (see \member{deleteActions}). Thus, instance
         *destruction also destroys the action specification.
         *
         *\end{itemize} */
        DomainSpecification();

        /*Domain copy construction achieves the following:
         *
         *\begin{itemize}
         *
         *\item The argument \member{rewardSpecification} is copied,
         *formula are cloned.
         *
         *\item The argument \member{actionSpecification} reference is
         *copied. Thus, specifications that result from copy
         *construction do not \member{deleteActions} on destruction.
         *
         *\item The argument \member{startStatePropositions} are copied.
         *
         *\item The argument domain \member{propositions} are copied.
         *
         *\end{itemize} */
        DomainSpecification(const DomainSpecification&);

        /*Frees resources taken by both the
         *\member{rewardSpecification} and where required (see
         *\member{deleteActions}) the \member{actionSpecification}.*/
        virtual ~DomainSpecification();

        /*Actions associated with NMRDP.*/
        ActionSpecification* getActionSpecification();

        /*Reward associated with NMRDP.*/
        RewardSpecification* getRewardSpecification();

        /*see \member{rewardSpecification}.*/
        RewardSpecification const* getRewardSpecification()const;
        
        /*Add state characterising proposition to the start state.*/
        void addToStartState(proposition);

        /*Set of propositions that hold in the start state.*/
        PropositionSet getStartStatePropositions()const;

        /*Add proposition.*/
        void addProposition(proposition);

        /*Returns the string identification of the argument
         *\argument{proposition}.*/
         proposition getProposition(const propositionIndex)const;

        /*Returns the integer identification of the argument
         *\argument{proposition} or an invalid \type{proposition}
         *depending on whether the \argument{proposition} has or
         *hasn't yet been registered respectively.
         *
         *In the case that the \argument{proposition} is not a domain
         *characterising one, the index returned is $1 +
         *propositions.size$.*/
        propositionIndex getProposition(const proposition&)const;

        /*Get the integer identities of the argument set of
         *propositions.*/
        PropositionIndexVector getAssociatedPropositions(const PropositionSet&)const;

        /*Set of possible state characterising propositions.*/
        PropositionVector getPropositions() const;

        /*Add reward specification to this domain.*/
        void addSpecComponent(string s, double d, formula* f);

        /*Add action specification to this domain.*/
        void addSpecComponent(action s, ActionSpecification::CPTS* cpt);

        /*Approximately the amount of memory taken by this
         *specification. The result is a number of Kb. The
         *\member{actionSpecification} is ignored.*/
        virtual unsigned int memory()const;

        /*Print this domain specification to the output stream.  The
         *\member{startStatePropositions},
         *\member{rewardSpecification} and
         *\member{actionSpecification} are included.*/
        void printDomain()const;

        /*Is this equal to the argument specification.
        *
        *Actions are not considered important to this test, thus only
        *the equality of the pointer to the actions
        *\member{actionSpecification} is considered when testing the
        *equality between domain specifications. Also, whether actions
        *are to be deleted or not, ie: the state encapsulated in
        *\member{deleteActions}, is not considered.
        *
        *FUTURE :: Actions shall be considered in a future release.
        **/
        virtual bool operator==(const DomainSpecification&) const;

        /*See \method{operator==()}.*/
        virtual bool operator!=(const DomainSpecification&) const;
    protected:
        /*Set of all possible state characterising propositions.*/
        PropositionVector propositions;

        /*Set of propositions that hold in the domain start state.*/
        PropositionSet startStatePropositions;

        /*Actions associated with domain.*/
        ActionSpecification* actionSpecification;
	
        /*Rewards associated with domain.*/
        RewardSpecification* rewardSpecification;

    private:
        /*Should this object be responsible for storage taken by the
         *\member{actionSpecification}?*/
        bool deleteActions;
    };

    /*A labelling of a set of states corresponds to a mapping from
     *sets of state characterising propositions to reward
     *specifications.*/
    typedef map<DomainSpecification::PropositionSet, RewardSpecification*> StateLabelling;
    
}
#endif
