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

#include"ValueIteration.h++"
#include"PolicyIteration.h++"
#include"domainSpecification_Anytime_or_Explicit.h++"

using namespace MDP;

/**************************BEGIN CONSTRUCTION****************************************************/

template<class ExplicitAlgorithm>
LAO<ExplicitAlgorithm>::LAO(DomainSpecification* domSpec)
    : Algorithm(*domSpec),
      dynamicProgrammingStep(domSpec)
{
    cerr<<"Failing to execute LAO* with a non--explicit domain.";
}
template<class ExplicitAlgorithm>
LAO<ExplicitAlgorithm>::LAO(explicitDomainSpecification& domSpec,
                            double gamma,
                            double epsilon,
                            bool useControlKnowledge)
    : Algorithm(domSpec, gamma, epsilon),
      dynamicProgrammingStep(domSpec, gamma, epsilon),
      useControlKnowledge(useControlKnowledge),
      controlKnowledgeUsed(false)
{
    this->explDomSpec = &domSpec;
    
    startState = explDomSpec->getStartState();
    
    /*Obtain maximum possible reward*/
    double maximumPossibleReward = this->explDomSpec->getRewardSpecification()->getRewardSum();
    
    /*Obtain expected reward for fringe states.*/
    expectedFringeReward = maximumPossibleReward / (1 - gamma);

    policy = domSpec.getPolicy();
    oldPolicy = *policy;
}


template<class ExplicitAlgorithm>
LAO<ExplicitAlgorithm>::LAO(explicitDomainSpecification& domSpec,
                            bool useControlKnowledge)
    : Algorithm(domSpec),
      dynamicProgrammingStep(domSpec),
      useControlKnowledge(useControlKnowledge),
      controlKnowledgeUsed(false)
{
    this->explDomSpec = &domSpec;
    
    startState = explDomSpec->getStartState();

    /*Obtain maximum possible reward*/
    double maximumPossibleReward = this->explDomSpec->getRewardSpecification()->getRewardSum();
    
    /*Obtain expected reward for fringe states.*/
    expectedFringeReward = maximumPossibleReward / (1 - gamma);

    policy = domSpec.getPolicy();
    oldPolicy = *policy;
}

/**************************END CONSTRUCTION****************************************************/

/**************************EXPANSION****************************************************/
template<class ExplicitAlgorithm>
typename LAO<ExplicitAlgorithm>::SuccesorList*
LAO<ExplicitAlgorithm>::expandFringeNode(eState* toExpand, std::set<eState*>& allStatesSet)
{
//    /*This function can only expand $FRINGE$ states.*/
//    assert(FRINGE == toExpand->getColour());
    
    SuccesorList* resultantFringeStates = new SuccesorList();
    
    /*Expand the $oldTop$ \member{explDomSpec} fringe node.*/
    explDomSpec->expandFringe(toExpand);

    /*Respect control knowledge.*/
    if(useControlKnowledge)
        /*If the control knowledge altered the state space.*/
        if(controlKnowledgeUsed = explDomSpec->removeImpossibleStates())
            return resultantFringeStates;
    
    /*For every action that can be executed in the state just expanded.*/
    for(StateTransitionMatrices::const_iterator transition = toExpand->begin()
            ; transition != toExpand->end()
            ; ++transition)
        {
            /*For every successor state.*/
            for(ActionPossibilities::const_iterator successor = transition->second.begin()
                    ; successor != transition->second.end()
                    ; ++successor)
                {
                    unsigned int oldSize = allStatesSet.size();
                    allStatesSet.insert(dynamic_cast<eState*>(successor->second));
                    
                    if(oldSize < allStatesSet.size())
                        {
                            resultantFringeStates
                                ->push_back(dynamic_cast<eState*>(successor->second));/*Add to fringe.*/
                        }
                    
                }
        }

    return resultantFringeStates;
}

template<class ExplicitAlgorithm>
pair<pair<typename LAO<ExplicitAlgorithm>::SuccesorList*, typename LAO<ExplicitAlgorithm>::SuccesorList*>* , eState*>
LAO<ExplicitAlgorithm>::successorsOnAction(action act, eState* state, int NodeColour)
{
    /*This vector shall contain all the states that are reachable from $state$
      on execution of action $act$.*/
    SuccesorList* result = new SuccesorList();

    /*This vector shall contain all the states that are reachable from $state$
      on execution of all possible action _OTHER THAN_ $act$.*/
    SuccesorList* baggage = new SuccesorList();

    /*If a fringe node is found during execution of this algorithm then it is
      assigned to $fringeNode$. NOTE: The default value of $0$.*/
    eState* fringeNode = 0;

    /*Obtain the state transition information for the \argument{act}
      concerning the \argument{state}.*/
    StateTransitionMatrices::const_iterator transition
        = state->find(act);
    
    /*If we are ignoring control knowledge.*/ 
    if(!useControlKnowledge)
        /*We assume that the action that was passed in is executable
          on the given state.*/
        assert(transition != state->end());
    else if(useControlKnowledge && transition == state->end())
    {
        transition = state->begin();
        act = transition->first;
    }

    /*For every successor state.*/
    for(ActionPossibilities::const_iterator successor = transition->second.begin()
            ; successor != transition->second.end()
            ; ++successor)
    {
        if(NodeColour == successor->second->getColour())/*Have we found a node we are searching for?*/
            fringeNode = dynamic_cast<eState*>(successor->second);
        
        result->push_back(dynamic_cast<eState*>(successor->second));
    }

    /*For every action that can be executed in the state just expanded.*/
    for(StateTransitionMatrices::const_iterator transition = state->begin()
            ; transition != state->end()
            ; ++transition)
        if(transition->first != act)
            {
                /*For every successor state.*/
                for(ActionPossibilities::const_iterator successor = transition->second.begin()
                        ; successor != transition->second.end()
                        ; ++successor)
                    baggage->push_back(dynamic_cast<eState*>(successor->second));
            }
    
    pair<SuccesorList*, SuccesorList*>* temporary =  new pair<SuccesorList*, SuccesorList*>(result, baggage);
    return pair<pair<SuccesorList*, SuccesorList*>* , eState*>(temporary, fringeNode);
}
/**************************EXPANSION****************************************************/

#include<set>
using namespace std;

template<class ExplicitAlgorithm>
void LAO<ExplicitAlgorithm>::completeExplicitGraph(std::set<eState*>& allStatesSet)
{
    std::set<eState*> extraToAdd;
    std::set<eState*> explicitStateSet;
    std::set<eState*> implictStateSet;

    /*Add all EXPLICIT states in $allStatesSet$ to the
      $explicitStateSet$ and all IMPLICIT states in $allStatesSet$ to
      the $implictStateSet$.*/
    for(std::set<eState*>::const_iterator state = allStatesSet.begin()
            ; state != allStatesSet.end()
            ; ++state)
        if(EXPLICIT == (*state)->getColour())
            explicitStateSet.insert(dynamic_cast<eState*>(*state));
        else if(IMPLICIT == (*state)->getColour())
            implictStateSet.insert(dynamic_cast<eState*>(*state));

    /*Loop until no more new EXPLICIT or IMPLICIT states are found.*/
    bool again = true;
    while(again)
    {
        /*Assume */
        again = false;

        /*Temporary copy of the $implictStateSet$.*/
        std::set<eState*> tmpImplictStateSet = implictStateSet;

        /*For all states that were in the $implictStateSet$ at the
          start of this iteration.*/
        for(std::set<eState*>::const_iterator state = tmpImplictStateSet.begin()
            ; state != tmpImplictStateSet.end()
            ; ++state)
        {
            /*Execute the policy on the n'th state in the expansion
              list. When executing the policy we are searching for an
              EXPLICIT state.*/
            pair<pair<SuccesorList*, SuccesorList*>* , eState*>
                tmp = successorsOnAction((*policy)[dynamic_cast<eState*>(*state)],
                                         dynamic_cast<eState*>(*state),
                                         EXPLICIT);

            /*For all direct successors via $policy$.*/
            for(SuccesorList::const_iterator toAdd = tmp.first->first->begin()
                    ; toAdd != tmp.first->first->end()
                    ; ++toAdd)
                /*If there is a direct successor of $*state$ which is
                  IMPLICIT then add this to the $implictStateSet$.*/
                if(!(FRINGE == (*toAdd)->getColour())
                   && !(EXPLICIT == (*toAdd)->getColour()))
                {
                    unsigned int oldSize = implictStateSet.size();
                    implictStateSet.insert(dynamic_cast<eState*>(*toAdd));

                    /*If another implicit state was found then another
                      iteration of the loop is required.*/
                    if(oldSize < implictStateSet.size())
                        again = true;
                }

            /*If the IMPLICIT state can transition to an EXPLICIT
              state when the optimal policy is executed.*/
            if(0 != tmp.second)
            {
                /*We make $*state$ EXPLICIT.*/
                (*state)->setColour(EXPLICIT);

                /*Update the local state sets according to the new EXPLICIT state.*/
                implictStateSet.erase(dynamic_cast<eState*>(*state));
                explicitStateSet.insert(dynamic_cast<eState*>(*state));

                /*Only successors of an EXPLICIT state are to be
                  considered by the dynamic programming algorithm.*/
                for(SuccesorList::const_iterator toAdd = tmp.first->second->begin()
                        ; toAdd != tmp.first->second->end()
                        ; ++toAdd)
                    extraToAdd.insert(dynamic_cast<eState*>(*toAdd));
                for(SuccesorList::const_iterator toAdd = tmp.first->first->begin()
                        ; toAdd != tmp.first->first->end()
                        ; ++toAdd)
                    allStatesSet.insert(dynamic_cast<eState*>(*toAdd));

                /*A new EXPLICIT state has been found, thus all
                  IMPLICIT states must be re-checked for a transition
                  to $*state$.*/
                again = true;
            }
            delete tmp.first->first;
            delete tmp.first->second;
            delete tmp.first;
        }
    }
    
    for(std::set<eState*>::const_iterator state = explicitStateSet.begin()
            ; state != explicitStateSet.end()
            ; ++state)
        allStatesSet.insert(dynamic_cast<eState*>(*state));
    
    for(std::set<eState*>::const_iterator state = extraToAdd.begin()
            ; state != extraToAdd.end()
            ; ++state)
        allStatesSet.insert(dynamic_cast<eState*>(*state));
}



/*****************************SEARCH******************************************/

template<class ExplicitAlgorithm>
bool LAO<ExplicitAlgorithm>::forwardSearch(Policy* policy,
                                           ExpansionStack& expansionStack,
                                           std::set<eState*>& allStatesSet)
{
    /*A state which is found to be on the fringe of the domain.*/
    eState* toExpand = 0;

    /*States from which the expansion begins.*/
    SuccesorList* expansionList = expansionStack.back();//*(--(expansionStack.end()));

    /*For every state in $expansionList$, ie: starting state.*/
    for(SuccesorList::const_iterator expansionFringe = expansionList->begin()
            ; expansionFringe != expansionList->end()
            ; ++expansionFringe)
    {
        /*Direct successor states which are reachable, when executing
          the optimal policy, from $*expansionFringe$. This will only
          contain states which the search has not discovered so far.*/
        SuccesorList* succesorList = new SuccesorList();
        
        /*Direct successor states which are reachable, when executing
          a non-optimal policy, from $*expansionFringe$.*/
        SuccesorList* nonOptSuccesorList = new SuccesorList();

        /*If we were not asked to explore from a FRINGE state.*/
        if(FRINGE != (*expansionFringe)->getColour())
            {
                /*Execute the policy on the n'th state in the
                  expansion list. When executing the policy we are
                  searching for a FRINGE state.*/
                pair<pair<SuccesorList*, SuccesorList*>* , eState*>
                    tmp = successorsOnAction((*policy)[dynamic_cast<eState*>(*expansionFringe)],
                                             dynamic_cast<eState*>(*expansionFringe));
                
                /*If a fringe state has been found, neither $toExpand$
                  or $*expansionFringe$ will have been explored
                  previously.*/
                if(0 != tmp.second)
                    toExpand = tmp.second;

                /*For all the direct successors of $*expansionFringe$
                  that may be reached when executing the prescribed
                  policy $policy$.*/
                for(SuccesorList::const_iterator possiblyNewState = tmp.first->first->begin()
                        ; possiblyNewState != tmp.first->first->end()
                        ; ++possiblyNewState)
                    {
                        /*Ensure a $*possiblyNewState$ is in the $allStatesSet$.*/
                        unsigned int oldSize = allStatesSet.size();
                        allStatesSet.insert(dynamic_cast<eState*>(*possiblyNewState));

                        /*If the $*possiblyNewState$ was not
                          previously in $allStatesSet$, add it to the
                          $succesorList$.*/
                        if(oldSize < allStatesSet.size())
                        {
                            succesorList
                                ->push_back(dynamic_cast<eState*>(*possiblyNewState));
                        }
                    }

                /*Clean up return storage no longer required.*/
                delete tmp.first->first;
                nonOptSuccesorList = tmp.first->second;
                //delete tmp.first->second;
                delete tmp.first;
            }
        else/*The state from which to explore was a FRINGE state.*/
            {
                toExpand = *expansionFringe;
            }
                
        /*If we have yet to find a fringe node to expand, 
          some successors of $*expansionFringe$ were found.*/
        if(0 < succesorList->size() && toExpand == 0)
        {
            expansionStack.push_back(nonOptSuccesorList);
            expansionStack.push_back(succesorList);

            (*expansionFringe)->setColour(EXPLICIT);
            
            /*CONTINUE EXPANSION!

              If a fringe node has been expanded, then all work is complete.*/
            if (forwardSearch(policy, expansionStack, allStatesSet)) return true;
            
            if(!keepEverything)
            {
                /*We are no longer trying to use these states to
                  achieve an expansion.*/
                expansionStack.pop_back();
                expansionStack.pop_back();

                /*The states in the explicit graph are not longer needed.*/
                for(SuccesorList::const_iterator succesorState = succesorList->begin()
                        ; succesorState != succesorList->end()
                        ; ++succesorState)
                    allStatesSet.erase(*succesorState);
            
                delete succesorList;
                delete nonOptSuccesorList;
            
                /*Recursive expansion has failed, thus we must
                  continue to search for a fringe node.*/
                (*expansionFringe)->setColour(IMPLICIT);
            }
            
        }
        /*The last expansion yielded no fringe or new states.*/
        else if(0 == succesorList->size() && toExpand == 0)
        {
            if(keepEverything)
            {
                expansionStack.push_back(nonOptSuccesorList);
                expansionStack.push_back(succesorList);
                (*expansionFringe)->setColour(EXPLICIT);
            }
            else
            {
                delete succesorList;
                delete nonOptSuccesorList;
            }
            
        }
        /*A fringe node has been found.*/
        else if(0 != toExpand)
        {
            expansionStack.push_back(nonOptSuccesorList);
            
            expansionStack.push_back(succesorList);
            
            (*expansionFringe)->setColour(EXPLICIT);            

            toExpand->setColour(EXPLICIT);

            /*We have to generate the transition information
              associated with the new state as it is EXPLICIT.*/
            SuccesorList* expandingStates = expandFringeNode(toExpand, allStatesSet);
            delete expandingStates;
            
            if(controlKnowledgeUsed)
                return false;

            /*Complete the explicit graph.*/
            completeExplicitGraph(allStatesSet);
            
            return true;
        }
        /*One of the above three steps must be taken.*/
        else
            assert(0);
    }

    return false;
}

/*****************************SEARCH******************************************/


/*(LAO*)*/
template<class ExplicitAlgorithm>
bool LAO<ExplicitAlgorithm>::operator()()
{
    /*This structure is non-trivial. It acts as a storage container
      for pairs of vectors of states. States stored at even indices
      (+1) are those reachable from states stored at lower odd indices
      (+1) when executing actions which are not recommended by the
      current policy. States stored at odd indices (+1) are those
      reachable if the optimal policy is adhered to.*/
    ExpansionStack expansionStack = ExpansionStack();

    /*The set of all the states that are passed to the dynamic
      programming algorithm.*/
    std::set<eState*> allStatesSet = std::set<eState*>();

    /*Configure the list from which the domain search will begin (see
      \function{forwardSearch()}).*/
    SuccesorList* startStateList = new SuccesorList();
    startStateList->push_back(startState);

    /*If the start state has already been expanded.*/
    if(IMPLICIT == startState->getColour())
        /*Then the start state may be labelled as EXPLICIT.*/
        startState->setColour(EXPLICIT);
    
    expansionStack.push_back(startStateList);
    allStatesSet.insert(dynamic_cast<eState*>(startState));

    /*We have not expanded a node yet.*/
    keepEverything = false;
    
    /*Calculate the portion of the state space that dynamic
      programming should be executed on.*/
    if(!forwardSearch(policy, expansionStack, allStatesSet))
        {
            /*If control knowledge caused the \method{forwardSearch()}
              to fail then another attempt must be made at LAO*.*/
            if(controlKnowledgeUsed)
            {
                /*Confirmation of use of control knowledge is no
                  longer required.*/
                controlKnowledgeUsed = false;
                
                /*Clean up memory from expansion.*/
                for(ExpansionStack::iterator p = expansionStack.begin()
                        ; p != expansionStack.end()
                        ; ++p)
                    delete *p;

                /*Try LAO* again.*/
                return operator()();
            }
            /*Current policy does not yield a fringe state.*/
            else
            {    
                keepEverything = true;
                allStatesSet = std::set<eState*>();
                allStatesSet.insert(dynamic_cast<eState*>(startState));
                
                /*If the start state has already been expanded.*/
                if(IMPLICIT == startState->getColour())
                    /*Then the start state may be labelled as EXPLICIT.*/
                    startState->setColour(EXPLICIT);
                
                forwardSearch(policy, expansionStack, allStatesSet);
            }
        }

    ExpansionStack::const_iterator expansionSet = expansionStack.begin();
    for(++expansionSet
            ; expansionSet != expansionStack.end()
            ; ++expansionSet, ++expansionSet)
        if(expansionSet != expansionStack.end())
            for(SuccesorList::const_iterator tmpSuccesorList = (*expansionSet)->begin()
                    ; tmpSuccesorList != (*expansionSet)->end()
                    ; ++tmpSuccesorList)
            {
                allStatesSet.insert(dynamic_cast<eState*>(*tmpSuccesorList));
            }
    

    /*Execute Bellman backup until convergence.*/
        
    /*Algorithm used for Bellman backup.*/
    dynamicProgrammingStep = ExplicitAlgorithm(*explDomSpec, gamma, epsilon, true);

    dynamicProgrammingStep.domainStates = vector<eState*>(0);

    for(std::set<eState*>::const_iterator state = allStatesSet.begin()
            ; state != allStatesSet.end()
            ; ++state)
        dynamicProgrammingStep.domainStates.push_back(*state);
        
    /*Use state colours to assign values to them.*/
    for(unsigned int i = 0; i < dynamicProgrammingStep.domainStates.size(); ++i)
        if(FRINGE == dynamicProgrammingStep.domainStates[i]->getColour())
            dynamicProgrammingStep.domainStates[i]->setValue(expectedFringeReward);
    
    dynamicProgrammingStep.stateIds = map<eState*, int>();
    
    dynamicProgrammingStep.identifyStates();
    
    dynamicProgrammingStep.configurePolicy(*explDomSpec);

    dynamicProgrammingStep.initialiseActions();
    
    dynamicProgrammingStep.configureActions(false);

    dynamicProgrammingStep.initialiseValueVectors();
    
    /*Run Bellman backup, dynamic programming step, until
      convergence.*/
    while(!dynamicProgrammingStep())
    {/*TODO:: Weaker stopping condition in the case of Value Iteration.*/};
 
    /*Update the values associated with domain states. Only values
      associated with states coloured for deletion ($MARKED$) are
      used.*/
    dynamicProgrammingStep.updateStateValues();

    /*Remove all transitions from newly explored states and colour
      them as "having been explored".*/
    for(unsigned int i = 0; i < dynamicProgrammingStep.domainStates.size(); ++i)
        {
            if(EXPLICIT == dynamicProgrammingStep.domainStates[i]->getColour())
                dynamicProgrammingStep.domainStates[i]->setColour(IMPLICIT);
        }

    /*Clean up memory from expansion.*/
    for(ExpansionStack::iterator p = expansionStack.begin()
            ; p != expansionStack.end()
            ; ++p)
        delete *p;
    
    /*Terminate when the same policy is given twice in a row.*/
    bool terminate = (oldPolicy == (*policy)) && keepEverything;

    oldPolicy = (*policy);
    return terminate;
}

template<class ExplicitAlgorithm>
double LAO<ExplicitAlgorithm>::getError()const
{
    return dynamicProgrammingStep.getError();
}

