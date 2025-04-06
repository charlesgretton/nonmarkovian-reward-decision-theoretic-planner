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
 * An explicit domain specification is one in which states are
 * represented explicitly.
 **/
#ifndef DOMAIN_ANYTIME_EXPLICIT_SPEC
#define DOMAIN_ANYTIME_EXPLICIT_SPEC

#include"SpecificationTypes.h++"
#include"Expansion.h++"
#include"Preprocessors.h++"

// #include<hash_map.h>
// struct string_hash {
//     size_t operator() (const string& key) const {
//         hash<const char*> H;
//         return H(key.c_str());
//     }
// };

namespace MDP
{
    class explicitDomainSpecification : public DomainSpecification
    {
    public:    
        /*
         *Construction
         */
        
        /*Construction evaluates the start state.*/
        explicitDomainSpecification(const DomainSpecification& domSpec);

        /*Free resources taken by this specification.*/
        ~explicitDomainSpecification();
        
        /*
         *Functionality
         */
        
        /*Configure the start/\member{fringe} states expansion to be
         *based on a \class{basedExpandedState}.
         *
         *\textbf{NOTE:} This does not effect the \member{startState}
         *which remains untouched.*/
        void preprocess(const Preprocessor&);

        /*Expand the argument node. This is the process of finding
         *\argument{eState} successors and adding those which are not
         *yet included in the domain to the \member{fringeStates}. The
         *\argument{eState} is added to the \member{domainState}
         *after the expansion operation is complete.*/
        void expandFringe(eState* stateToExpand,
                          const Expansion& expansion = StateAnnotationExpansion());
        
        /*Expand the \member{fringeStates}.*/
        bool expandFringe(const Expansion& expansion = StateAnnotationExpansion());

        /*Add states in the range of the
         *\argument{StateTransitionMatrices} to the fringe where they
         *are not already elements of \member{domainStates} or
         *\member{fringeStates}*/
        void addToFringe(StateTransitionMatrices&);

        /*Remove all the impossible states and related
         *actions. Returns true if a \class{eState} is removed (see
         *\method{containsImpossibilities();}).*/
        bool removeImpossibleStates();

        /*Remove all actions which lead to an impossible
         *state. Returns true if an action is removed.*/
        bool removeImpossibleActions();

        /*Configure the domain fringe.*/
        void explicitDomainSpecification::setFringe(const eStateSet& newFringe);

        /*Makes this specification interpret all domain and fringe states
         *as \member{nmrs}*/
        void explicitDomainSpecification::allAreNMRS();
        
        /*
         *Queries + Accessors
         */

        /*What is the start state?*/
        eState* getStartState();
        
        /*Mapping from NMRDP states to label sets.*/
        StateLabelling& getNMRSLabels();
        
        /*What does this domain specification look like as a
         *string? Both \member{domainStates} and \member{fringeStates}
         *are included.
         *
         *This method is not thread safe is the argument is $true$.*/
        string toString(bool withStartStateLabel = false)const;

        /*String representation of the current \member{policy}.*/
        string policyToString()const;
        
        /*Result is the \argument{vector} resized and filled with all
         *this domains' states.*/
        vector<eState*>& getStates(vector<eState*>&)const;

        /*Result is the \argument{vector} resized and filled with all
         *this domains' \member{fringeStates}.*/
        vector<eState*>& getFringeStates(vector<eState*>&)const;
        
        /*Result is the \argument{vector} resized and filled with all
         *this \member{domainsStates} (non--fringe states).*/
        vector<eState*>& getDomainStates(vector<eState*>&)const;
        
        /*Does the state space include impossible states?*/
        bool containsImpossibilities()const;

        /*Get a handle to this domains policy.*/
        Policy* getPolicy();
        
        /*Approximately the amount of memory taken by this
         *specification. The result is a number of Kb.*/
        unsigned int memory()const;

        /*Number of states.*/
        unsigned int numberOfStates()const;
        
        /*see \member{longestStateLabelSize()}. This does not account
         *for the operator size of the longest label amongst the
         *\member{nmrsLabels}.*/
        unsigned int longestLabel()const;

        /*see \member{averageStateLabelSize()}. This does not account
         *for the average size of \member{nmrsLabels}.*/
        unsigned int averageLabelSize()const;

        /*Size of the longest state label.*/
        unsigned int longestStateLabelSize()const;

        /*Size of the average state label.*/
        unsigned int averageStateLabelSize()const;

        /*See \parent{DomainSpecification::operator==()}.*/
        bool operator==(const DomainSpecification&) const;
        
        /*See \parent{DomainSpecification::operator!=()}.*/
        bool operator!=(const DomainSpecification&) const;

        /*See \method{operator==().}*/
        bool basedOn(const DomainSpecification&) const;
    protected:
        /*Pointer to the starter state.*/
        eState* startState;
	
        /*States which have been expanded.*/
        eStateSet domainStates;

        /*Set of Non--Markovian Reward States labels.*/
        StateLabelling nmrsLabels;
        
        /*States which have yet to be expanded. Also referred to as
         *unexplored domain states.*/
        eStateSet fringeStates;

        /*Current policy.*/
        Policy policy;
    private:
        /*Storage for all domain states.*/
        map<eState, eState*> allStatesSet;
        //hash_map<string, map<eState, eState*>, string_hash> allStatesSet;

        /*Has \member{allStatesSet} been initialised.*/
        bool initialisedStateStore;
    };
};
#endif
