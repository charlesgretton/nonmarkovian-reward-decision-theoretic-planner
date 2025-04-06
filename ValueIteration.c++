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

#include "ValueIteration.h++"
#include"domainSpecification_Anytime_or_Explicit.h++"

using namespace mtl;

ValueIteration::ValueIteration(DomainSpecification* domSpec)
    : Algorithm(*domSpec),
      epoch(0),
      stateId(0)
{
    cerr<<"An attempt to execute explicit policy iteration with a non--explicit domain will fail!\n";
}

ValueIteration::ValueIteration(explicitDomainSpecification& domSpec, double gamma, double epsilon, bool delayInitialisation)
    : Algorithm(domSpec, gamma, epsilon),
      epoch(0),
      stateId(0)
{
    if(!delayInitialisation)
    {
        domainStates = domSpec.getStates(domainStates);
        
        configurePolicy(domSpec);
        
        identifyStates();
        
        initialiseActions();
        
        configureActions();
        
        initialiseValueVectors();
    }
}

ValueIteration::ValueIteration(explicitDomainSpecification& domSpec)
    : Algorithm(domSpec),
      epoch(0),
      stateId(0)
{
    domainStates = domSpec.getStates(domainStates);

    configurePolicy(domSpec);

    identifyStates();

    initialiseActions();
    
    configureActions();

    initialiseValueVectors();
}

void ValueIteration::configurePolicy(explicitDomainSpecification& domSpec)
{
    policy = domSpec.getPolicy();
}

void ValueIteration::initialiseValueVectors()
{
    lastEpoch = ValueVector(domainStates.size());
    
    thisEpoch = ValueVector(domainStates.size());

    initialReward = ValueVector(domainStates.size());
    
    for(vector<eState*>::const_iterator domainState = domainStates.begin()
            ; domainState != domainStates.end()
            ; ++domainState)
    {
        /*If the state is expanded then all transitional information
          has been removed and the value associated with that state is
          the expected reward.*/
        if(FRINGE == (*domainState)->getColour()
           || IMPLICIT == (*domainState)->getColour())
            initialReward[stateIds[*domainState]] = (*domainState)->getValue();
        else{
           //EXPLICIT == (*domainState)->getColour());
//              cout<<"ExplicitState \n";
//              cout<<(*domainState)->toString();

//              char ch;

//              cin>>ch;
            
            initialReward[stateIds[*domainState]] = (*domainState)->getReward();
        }
        
        
        lastEpoch[stateIds[*domainState]]
            = initialReward[stateIds[*domainState]];
    }
    
}

void ValueIteration::configureActions(bool ignoreExplicit)
{
    configureActions(domainStates.begin(), ignoreExplicit);
}

void ValueIteration::configureActions(vector<eState*>::const_iterator domainState, bool ignoreExplicit)
{
    /*For each domain state.*/
    for(//vector<eState*>::const_iterator domainState = domainStates.begin()
            ; domainState != domainStates.end()
            ; ++domainState)
        if(ignoreExplicit || EXPLICIT == (*domainState)->getColour())
            /*And each action possible at that state.*/
            for(State::iterator stateAction = (*domainState)->begin()
                    ; stateAction != (*domainState)->end()
                    ; ++stateAction)
                /*For each possible transition given the action.*/
                for(ActionPossibilities::iterator transitionPair =
                        stateAction->second.begin()
                        ; transitionPair != stateAction->second.end()
                        ; ++transitionPair)
                    if(ignoreExplicit
                       || stateIds.end() != stateIds.find(dynamic_cast<eState*>(transitionPair->second)))
                    {
                        actionMatrices[stateAction->first]
                            (stateIds[*domainState],
                             stateIds[dynamic_cast<eState*>(transitionPair->second)])
                            = transitionPair->first;
                    }
}


void ValueIteration::identifyStates()
{
    stateId = 0;
    identifyStates(domainStates.begin());
}
    
void ValueIteration::identifyStates(vector<eState*>::const_iterator domainState)
{   
    /*Make the states identifiable.*/
    for(//domainState = domainStates.begin()
            ; domainState != domainStates.end()
            ; ++domainState, ++stateId)
        stateIds[*domainState] = stateId;
}

void ValueIteration::initialiseActions()
{
    ActionSpecification* actions = domSpec->getActionSpecification();
    
    for(ActionSpecification::const_iterator i = actions->begin()
            ; i != actions->end()
            ; i++)
        actionMatrices[i->first] = ActionMatrix(domainStates.size(), domainStates.size());
}

void ValueIteration::updateStateValues()
{
    int i = 0;
    vector<eState*>::iterator state;
    for(i = 0, state = domainStates.begin()
            ; state != domainStates.end()
            ; ++state, ++i)
        (*state)->setValue(thisEpoch[i]);
}

bool ValueIteration::terminate(const ValueVector thisEpoch, const ValueVector lastEpoch)
{   
    ValueVector negatedLastEpoch(lastEpoch.size());
    ValueVector norm(lastEpoch.size());
    
    copy(scaled(lastEpoch, -1.0), negatedLastEpoch);
    add(thisEpoch, negatedLastEpoch, norm);
    int normIndex = max_index(norm);

    Algorithm::error = norm[normIndex];
    
    return (norm[normIndex] <= (epsilon * ((1 - gamma)/(2*gamma))));
}

bool ValueIteration::operator()()
{
    epoch++;

    thisEpoch = ValueVector(domainStates.size());
    
    /*For each action.*/
    for(ActionMatrices::const_iterator action = actionMatrices.begin()
            ; action != actionMatrices.end()
            ; ++action)
    {
        ValueVector tmp(domainStates.size());
        
        mult(action->second, lastEpoch, tmp);
        
        copy(scaled(tmp, gamma), tmp);
        
        /*Was this action superior for any state?*/
        for(unsigned int j = 0;  j < domainStates.size(); ++j)
        {
            if(tmp[j] > thisEpoch[j])
            {
                thisEpoch[j] = tmp[j];
                (*policy)[domainStates[j]] = action->first;
            }
        }    
    }

    add(thisEpoch, initialReward, thisEpoch);
 
    bool doTerminate = terminate(thisEpoch, lastEpoch);

    copy(thisEpoch, lastEpoch);
    
    return doTerminate;
}
