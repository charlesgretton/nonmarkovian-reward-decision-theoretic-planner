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

#include"Preprocessors.h++"
#include"States.h++"
#include"domainSpecification_Anytime_or_Explicit.h++"

using namespace MDP;

/*************************************** Preprocessor*/

Preprocessor::Preprocessor()
{}

eStateSet Preprocessor::operator()(eStateSet& oldFringeStates,
                                   explicitDomainSpecification& domSpec)const
{
    for(eStateSet::iterator oldState = oldFringeStates.begin()
            ; oldState != oldFringeStates.end()
            ; ++oldState)
        (*oldState)->calculateReward();
    
    return oldFringeStates;
}

/*************************************** PhaseIPreprocessor */

PhaseIPreprocessor::PhaseIPreprocessor(PhaseI const* pI)
    :phaseI(pI)
{}

eStateSet PhaseIPreprocessor::operator()(eStateSet& oldFringeStates,
                                         explicitDomainSpecification& domSpec)const
{
    eStateSet newFringeStates;
    
    for(eStateSet::iterator oldState = oldFringeStates.begin()
            ; oldState != oldFringeStates.end()
            ; ++oldState)
    {
        /*New state has a copy of the first arguments reward
          specification and a handle to the second argument
          specification.*/
        basedExpandedState* newState =
            new basedExpandedState(**oldState, (*phaseI)(domSpec));
        
        /*We assume none of the fringe states have predecessors at
          this point as they are all start states. This means that the
          copy of the old states reward specification is deleted.*/

        newState->hasPredecessor(false);
        
        newState->calculateReward();
        
        /*Configure the start states expansion based on a base state.*/
        newFringeStates.insert(newState);
        
        delete *oldState;
    }

    return newFringeStates;
}

/*************************************** PhaseIPreprocessor */

MinimalPhaseIPreprocessor::MinimalPhaseIPreprocessor(PhaseI const* phI)
    :PhaseIPreprocessor(phI)
{}

eStateSet MinimalPhaseIPreprocessor::operator()(eStateSet& oldFringeStates,
                                                explicitDomainSpecification& domSpec)const
{   
    /*Among other things, this ensures that the fringe is composed
      of \class{basedExpandedState}s and that these have a
      \member{labelSet}. The fringe is old as it is to be discarded
      after its members \member{labelSet} have been configured.*/
    const eStateSet& oldFringe =
        PhaseIPreprocessor::operator()(oldFringeStates, domSpec);
    
    domSpec.setFringe(oldFringe);
    
    /*Find all base (NMRDP) states.*/
    while(!domSpec.expandFringe(Expansion()));

    /*Find all states $allStates$ in the state space.*/ 
    vector<eState*> allStates;    
    allStates = domSpec.getStates(allStates);
    
    /*Has a \member{labelSet} of a state space element been changed?*/
    bool changed = true;
    while(changed)
        {
            changed = false;
            
            /*For every NMRDP state calculate the reward assuming no
              predecessor \footnote{Regress the label set and store
              this in the states reward specification.}.*/
            for(vector<eState*>::const_iterator state = allStates.begin()
                    ; state != allStates.end()
                    ; ++state)
                {
                    /*Remove any old reward calculations as
                      predecessor information is not important at this
                      stage.*/
                    (*state)->hasPredecessor(false);
                    
                    /*This instantiates plain regression of the $state$s \member{labelSet}
                      with no filtration. The result of the filtration becomes the $state$
                      reward.*/
                    (*state)->generateReward();

                }
            
            /*For every NMRDP state.*/
            for(vector<eState*>::const_iterator state = allStates.begin()
                    ; state != allStates.end()
                    ; ++state)
                {
                    /*Obtain the set of $basedExpandedState$ successors.*/
                    StateSet successors = (*state)->getSuccessors();

                    unsigned int labelLength
                                = dynamic_cast<basedExpandedState*>(*state)->getLabelLength();
                    
                    /*For each successor.*/
                    for(StateSet::const_iterator sucState = successors.begin()
                            ; sucState != successors.end()
                            ; ++sucState)
                        dynamic_cast<basedExpandedState*>(*sucState)
                            ->updatePredecessorReward(*dynamic_cast<basedExpandedState*>(*state));

                    /*If $state$s label length has changed, then a
                      further iteration is necessary.*/
                    if(labelLength < dynamic_cast<basedExpandedState*>(*state)->getLabelLength())
                        changed = true;
                }
        }

    eStateSet answerFringe;
    
    for (eStateSet::const_iterator  fringeState = oldFringe.begin()
             ; fringeState != oldFringe.end()
             ; ++fringeState)
        {
            /*All elements of the fringe are start states.*/
            (*fringeState)->hasPredecessor(false);

            /*Calculate the reward associated with the fringe state.*/
            (*fringeState)->calculateReward();

            /*Add the fringe state to the set of states that shall be
              returned by this function.*/
            answerFringe.insert( dynamic_cast<basedExpandedState*>((*fringeState)->copy()) );
        }
    
    
    /*Move all base states to the fringe.*/
    domSpec.allAreNMRS();

    return answerFringe;
}
