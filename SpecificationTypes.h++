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
 *
 * \paragraph{:Purpose:}
 *
 * Datatype definitions concerning the specification and storage of
 * NMRDPs and MDPs.  */
#ifndef SPECIFICATION_TYPES
#define SPECIFICATION_TYPES

#include<string>
#include<set>
#include<vector>
#include<map>
#include<algorithm>

/*A colour for states which have been explored.*/
#define EXPLICIT 2

/*A colour for states which have been marked as explored, but for
  which the exploration is possibly incomplete.*/
#define IMPLICIT 1

/*A colour for states which are on the fringe.*/
#define FRINGE 0

namespace MDP
{   
    class State;

    class eState;
    
    class basedExpandedState;

    class PhaseI;
    
    class DomainSpecification;
    
    class explicitDomainSpecification;
    
    class ActionSpecification;

    class RewardSpecification;
    
    class FLTLrewardSpecification;
    
    class PLTLrewardSpecification;

    /*Name of an action.*/
    typedef std::string action;
    
    /*An MDP policy (function from \class{eStates} to
     *\class{actions}.).*/
    typedef std::map<eState*, action> Policy;
    
    /*Proposition*/
    typedef std::string proposition;
    
    /*Domain eState containment.*/
    typedef std::set<eState*> eStateSet;

    /*Domain State containment.*/
    typedef std::set<State*> StateSet;

    /*State probability pair.*/
    typedef std::pair<double, State*> TransitionPair;
    
    typedef std::vector<TransitionPair> ActionPossibilities;
    
    typedef std::map<action, ActionPossibilities> StateTransitionMatrices;
};

#endif
