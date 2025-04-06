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

#include<sstream>

#include"domainSpecification_Anytime_or_Explicit.h++"

#include"rewardSpecification.h++"
#include"PhaseI.h++"

using namespace MDP;
            
        /*
         *Construction
         */

explicitDomainSpecification::explicitDomainSpecification
(const DomainSpecification& domSpec)
    :DomainSpecification(domSpec),
     initialisedStateStore(false)
{
    startState = new eState(startStatePropositions, *rewardSpecification);
    fringeStates.insert(startState);
}
        
        /*
         *Destruction
         */

explicitDomainSpecification::~explicitDomainSpecification()
{
    /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;    
    allStates = getStates(allStates);

    for(vector<eState*>::iterator p = allStates.begin()
            ; p != allStates.end()
            ; ++p)
        delete (*p);
}

        /*
         *Functionality
         */

void explicitDomainSpecification::preprocess(const Preprocessor& preprocessor)
{
    /*Preprocess the old fringe.*/
    setFringe( preprocessor(fringeStates, *this) );
}

void explicitDomainSpecification::expandFringe(eState* stateToExpand,
                                               const Expansion& expansion)
{   
    /*Calculate the possible state transitions from
      \argument{stateToExpand} given this specification.*/
    StateTransitionMatrices* stateTransitionMatrices
        = expansion(*stateToExpand, *this, getNMRSLabels());
    
    fringeStates.erase(stateToExpand);    
    domainStates.insert(stateToExpand);
    
    /*Add the new found states to the set of
      \member{fringeStates}. Also replace state duplicates with states
      already generated.*/
    
    addToFringe(*stateTransitionMatrices);
    
    /*Configure the previous fringe states' successors.*/
    stateToExpand->addSuccessors(*stateTransitionMatrices);
        
    delete stateTransitionMatrices;
}

bool explicitDomainSpecification::expandFringe(const Expansion& expansion)
{
    /*If there is a fringe to speak of.*/
    if(0 != fringeStates.size())
        expandFringe(*fringeStates.begin(), expansion);

    return 0 == fringeStates.size();
}

bool explicitDomainSpecification::removeImpossibleStates()
{
    /*Returns $result = true$ if a \class{eState} is removed (see
      \method{containsImpossibilities();}). Assume that all states
      remain.*/
    bool result = false;
    
    /*Are some impossible actions removed?*/
    if(removeImpossibleActions())
    {
        /*States which have been confirmed possible.*/
        StateSet visited;

        /*States which have yet to be explored for possibility.*/
        StateSet unvisited;

        /*Start exploration from the start state.*/
        unvisited.insert(startState);
        
        /*The size of the old set of visited states.*/
        unsigned int oldVisitedSize;
    
        /*Traverse the state space from the start state taking note of
          visited states.*/
        do{
            /*The size of the old set of visited states.*/
            oldVisitedSize = visited.size();

            /*States that are found to require visitation.*/
            StateSet newUnvisited;

            /*For every unvisited state.*/
            for(StateSet::const_iterator visiting = unvisited.begin()
                    ; visiting != unvisited.end()
                    ; ++visiting)
            {
                /*The state has now been visited.*/
                visited.insert(*visiting);

                /*What states can succeed the state we are $visiting$.*/
                StateSet transitionStates = (*visiting)->getSuccessors();

                /*set_union of $newUnvisited$ with $transitionStates$.*/
                for(StateSet::const_iterator adding = transitionStates.begin()
                        ; adding != transitionStates.end()
                        ; ++adding)
                    newUnvisited.insert(*adding);
            }

            /*States yet to be visited are now as follows.*/
            unvisited = newUnvisited;
            
            /*Filter visited states from the unvisited collection.*/
            for(StateSet::const_iterator adding = newUnvisited.begin()
                    ; adding != newUnvisited.end()
                    ; ++adding)
            {
                StateSet::const_iterator i;
                if(visited.end() != (i = visited.find(*adding)))
                    unvisited.erase(*adding);
            }
        
        }while(oldVisitedSize != visited.size());
        
        /*Find all states $allStates$ in the state space.*/ 
        vector<eState*> allStates;    
        allStates = getStates(allStates);
        
        /*Remove all states that were not visited in the above traversal.*/
        for(vector<eState*>::const_iterator state = allStates.begin()
                ; state != allStates.end()
                ; ++state)
        {
            StateSet::const_iterator i;
            
            if(visited.end() != /*This find could be destroyed by deletion.*/
               ( i = visited.find(dynamic_cast<State*>(*state)) ) );
            else/*If the state is unreachable then remove it from the
                  state space and free memory taken by the unreachable
                  state on the heap.*/
            {
                result = true;
                fringeStates.erase(*state);
                domainStates.erase(*state);
                
                allStatesSet.erase(**state);
                
                delete *state;
            }
        }
    }/*If states were unreachable.*/

    return result;
}

bool explicitDomainSpecification::removeImpossibleActions()
{
    bool containsImpossibilities =
        explicitDomainSpecification::containsImpossibilities();

    /*If there are no impossible states then return informing the
      caller of this.*/
    if(!containsImpossibilities)
        return false;
    
    while(containsImpossibilities)
    {
        /*Assume that this iteration of the loop shall remove all
          impossibilities.*/
        containsImpossibilities = false;
        
        /*Find all states $allStates$ in the state space.*/ 
        vector<eState*> allStates;
        allStates = getStates(allStates);
        
        for(vector<eState*>::iterator state = allStates.begin()
                ; state != allStates.end()
                ; ++state)
        {
            if((*state)->isPossible())
            {
                /*Remove the impossible actions form the state.*/
                (*state)->removeImpossibleActions();
                
                /*Mark the state as impossible if there are no
                  possible actions from it and it is not in the
                  fringe.*/
                if(fringeStates.end() ==
                   fringeStates.find(dynamic_cast<eState*>(*state)))
                    (*state)->calculatePossibility();
                
                containsImpossibilities = !(*state)->isPossible();
            }/*Removal of impossible states from the state space shall
               be done later.*/
        }
    }

    return true;
}

void explicitDomainSpecification::addToFringe(StateTransitionMatrices& possibleNewStates)
{     
    if(!initialisedStateStore){         
        /*Find all states $allStates$ in the state space.*/ 
        vector<eState*> allStates;    
        allStates = getStates(allStates);
        
        initialisedStateStore = true;
        for(vector<eState*>::const_iterator state = allStates.begin()
                ; state != allStates.end()
                ; ++state)
            //(allStatesSet[(*state)->toString(true)])[**state] = *state;
            allStatesSet[**state] = *state;
    }
    
    /*For each maplet or action in the \argument{possibleNewStates}
      transition matrix.*/
    for(StateTransitionMatrices::iterator action = possibleNewStates.begin()
            ; action != possibleNewStates.end()
            ; ++action)
        /*For each action possibility pair <reward, State*> in the
          $action$.*/
        for(ActionPossibilities::iterator pair = action->second.begin()
                ; pair != action->second.end()
                ; ++pair)
        {
            eState *tmp = dynamic_cast<eState*>(pair->second);
            
            if(0 == tmp)
                assert(0);

            //map<eState, eState*>::iterator id = (allStatesSet[tmp->toString(true)]).find(*tmp);
            map<eState, eState*>::iterator id = allStatesSet.find(*tmp);
            
            //if((allStatesSet[tmp->toString(true)]).end() == id)
            if(allStatesSet.end() == id)
            {
                fringeStates.insert(tmp);/*Add to fringe.*/
                //(allStatesSet[tmp->toString(true)])[*tmp] = tmp;
                allStatesSet[*tmp] = tmp;
            }
            else
            {
                delete pair->second;
                pair->second = id->second;
            }
            
        }
}

void explicitDomainSpecification::setFringe(const eStateSet& newFringe)
{
    fringeStates = newFringe;

    /*The start state may have changed or been deleted.*/
    startState = *fringeStates.begin();
}

void explicitDomainSpecification::allAreNMRS()
{
    /*Find $allStates$ in the state space.*/ 
    vector<eState*> allStates;    
    allStates = getStates(allStates);

    /*Move every state to the \member{nmrsLabels}.*/
    for(vector<eState*>::const_iterator state = allStates.begin()
            ; state != allStates.end()
            ; ++state)
        {
            nmrsLabels[(*state)->getPropositions()]
                = dynamic_cast<basedExpandedState*>(*state)->getLabelSet()->copy();
            
            domainStates.erase(*state);
            fringeStates.erase(*state);
            
            allStatesSet.erase(**state);
            
            delete *state;
        }
    /*\member{allStatesSet} should be empty.*/
    initialisedStateStore = false;
}

        /*
         *Queries + Accessors
         */

eState* explicitDomainSpecification::getStartState()
{
    return startState;
}

StateLabelling& explicitDomainSpecification::getNMRSLabels()
{
    return nmrsLabels;
}

string explicitDomainSpecification::toString(bool withStartStateLabel)const
{
    ostringstream answer;

    /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;    
    allStates = getStates(allStates);

    if(withStartStateLabel)
    {
        
        startStateProposition ss;
        string* tmp = ss.print();
        startState->addProposition(*tmp);
        delete tmp;
    }
    
    for(vector<eState*>::const_iterator p = allStates.begin()
            ; p != allStates.end()
            ; ++p)
    {
        /*We want a string based representation of every state in this
          specification.*/
        answer<<(*p)->toString()<<endl;
    }

    if(withStartStateLabel)
    {
        startStateProposition ss;
        string* tmp = ss.print();
        startState->removeProposition(*tmp);
        delete tmp;
    }
    
    return answer.str();
}

string explicitDomainSpecification::policyToString()const
{
    ostringstream answer;

    for(Policy::const_iterator polItem = policy.begin()
            ; polItem != policy.end()
            ; ++polItem)
    {
        answer<<"({";
        bool first = true;
        
        for(DomainSpecification::PropositionSet::const_iterator
                proposition = polItem->first->getPropositions().begin()
                ; proposition != polItem->first->getPropositions().end()
                ; ++proposition)
        {
            if(!first)
                answer<<", ";
            first = false;
            answer<<*proposition;
        }

        answer<<"}, {";
        

        first = true;
        for(RewardSpecification::const_iterator
                reward = polItem->first->getRewardSpecification()->begin()
                ; reward != polItem->first->getRewardSpecification()->end()
                ; ++reward)
        {
            if(!first)
                answer<<", ";
            first = false;
            string* tmp = reward->second.form->print();
            answer<<*tmp;
            delete tmp;
        }
        answer<<"}) |-> "<<polItem->second<<endl;
    }
    
    return answer.str();
}

vector<eState*>& explicitDomainSpecification::getStates(vector<eState*>& answer)const
{
    /*Configure the vector size.*/
    answer.resize(domainStates.size()+fringeStates.size());
    
    /*Find all states $allStates$ in the state space.*/ 
    set_union(domainStates.begin(),
              domainStates.end(),
              fringeStates.begin(),
              fringeStates.end(),
              answer.begin());
    
    return answer;
}

vector<eState*>& explicitDomainSpecification::getFringeStates(vector<eState*>& answer)const
{
    return answer = vector<eState*>(fringeStates.begin(), fringeStates.end());
}

vector<eState*>& explicitDomainSpecification::getDomainStates(vector<eState*>& answer)const
{
    return answer = vector<eState*>(domainStates.begin(), domainStates.end());
}


bool explicitDomainSpecification::containsImpossibilities()const
{
    /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;    
    allStates = getStates(allStates);

    /*Unary state possibility verifier.*/
    IsPossible isPossible;
    
    vector<eState*>::iterator id;
    
    /*Find an impossible state in the domain states.*/
    for(id = allStates.begin(); id != allStates.end() && isPossible(*id); ++id);
    
    if(allStates.end() == id)
        return false;
    else
        return true;
}

Policy* explicitDomainSpecification::getPolicy()
{
    return &policy;
}

unsigned int explicitDomainSpecification::memory()const
{
    unsigned int size = DomainSpecification::memory()
        + sizeof(*this)
        - sizeof(DomainSpecification);

    size += sizeof(startState);

    /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;    
    allStates = getStates(allStates);

    for(vector<eState*>::const_iterator p = allStates.begin()
        ; p != allStates.end()
        ; ++p)
        size += (*p)->memory();
    
    return size;
}

unsigned int explicitDomainSpecification::numberOfStates()const
{
    return domainStates.size() + fringeStates.size();
}

unsigned int explicitDomainSpecification::longestLabel()const
{
    unsigned int labelSize = 0;
    /*It may become useful some day to account \member{nmrsLabels} but
      for the moment we refrain from doing so.*/
    
//      /*For each non--Markovian reward state.*/
//      for(StateLabelling::const_iterator p = nmrsLabels.begin()
//          ; p != nmrsLabels.end()
//          ; ++p)
//      {
//          unsigned int totalSize = 0;
        
//          /*Sum the labels.*/
//          for(RewardSpecification::const_iterator q = p->second->begin()
//                  ; q != p->second->end()
//                  ; ++q)
//              totalSize += q->second.form->length();
        
        
//          if(totalSize > labelSize)
//              labelSize = totalSize;
//      }

    /*In the case that the longest label is simply the reward
      specification of the first state.*/
//      if(0 == nmrsLabels.size())
//      {
        labelSize = longestStateLabelSize();
//      }
    
    return labelSize;
}

unsigned int explicitDomainSpecification::averageLabelSize()const
{
    /*It may become useful some day to account \member{nmrsLabels} but
      for the moment we refrain from doing so.*/
//      unsigned int labelSize = 0;

//      /*For each non--Markovian reward state.*/
//      for(StateLabelling::const_iterator p = nmrsLabels.begin()
//              ; p != nmrsLabels.end()
//              ; ++p)
//          /*Sum the labels.*/
//          for(RewardSpecification::const_iterator q = p->second->begin()
//                  ; q != p->second->end()
//                  ; ++q)
//              labelSize += q->second.form->length();

    /*In the case that the longest label is simply the reward
      specification of the first state.*/
//      if(0 == nmrsLabels.size())
//      {
        return averageStateLabelSize();
//      }
    
//      if(labelSize != 0)
//          /*Take the average.*/
//          return static_cast<unsigned int>(labelSize / nmrsLabels.size());
//      else
//          return labelSize;
}
    
unsigned int explicitDomainSpecification::longestStateLabelSize() const
{
    /*If there is no start state.*/
    if(0 == startState)
        return 0;
    
    unsigned int labelSize = 0;

    /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;
    allStates = getStates(allStates);
    
    for(vector<eState*>::const_iterator p = allStates.begin()
            ; p != allStates.end()
            ; ++p)
    {    
        unsigned int totalSize = 0;
        
        for(RewardSpecification::const_iterator q
            = (*p)->getRewardSpecification()->begin()
            ; q != (*p)->getRewardSpecification()->end()
            ; ++q)
            totalSize += q->second.form->length();
        
        if(totalSize > labelSize)
            labelSize = totalSize;
    }
    
    return labelSize;
}
 
unsigned int explicitDomainSpecification::averageStateLabelSize() const
{
    /*If there is no start state.*/
    if(0 == startState)
        return 0;

     /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;
    allStates = getStates(allStates);
    
    unsigned int labelSize = 0;

    for(vector<eState*>::const_iterator p = allStates.begin()
            ; p != allStates.end()
            ; ++p)
    {
        for(RewardSpecification::const_iterator q
            = (*p)->getRewardSpecification()->begin()
            ; q != (*p)->getRewardSpecification()->end()
            ; ++q)
            labelSize += q->second.form->length();
    }
    
    if(labelSize != 0)
        /*Take the average.*/
        return static_cast<unsigned int>(labelSize / allStates.size());
    else
        return labelSize;
}

bool explicitDomainSpecification::operator==(const DomainSpecification& domSpec) const
{
    return DomainSpecification::operator==(domSpec);
}

bool explicitDomainSpecification::basedOn(const DomainSpecification& domSpec) const
{return operator==(domSpec);}

bool explicitDomainSpecification::operator!=(const DomainSpecification& domSpec) const
{
    return DomainSpecification::operator!=(domSpec);
}
