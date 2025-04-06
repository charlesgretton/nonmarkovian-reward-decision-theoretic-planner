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

#include "PolicyIteration.h++"
#include"domainSpecification_Anytime_or_Explicit.h++"

using namespace mtl;

PolicyIteration::PolicyIteration(DomainSpecification* domSpec)
    : Algorithm(*domSpec),
      iteration(0),
      stateId(0)
{
    cerr<<"An attempt to execute explicit policy iteration with a non--explicit domain will fail!\n";
}

PolicyIteration::PolicyIteration(explicitDomainSpecification& domSpec,
                                 double gamma,
                                 double epsilon,
                                 bool delayInitialisation)
    : Algorithm(domSpec, gamma, epsilon),
      iteration(0),
      stateId(0)
{
    if(!delayInitialisation)
    {
        domainStates = domSpec.getStates(domainStates);
        
        identifyStates();
        
        initialiseActions();
        
        configureActions();
        
        initialiseValueVectors();
        
        configurePolicy(domSpec);
    }
}

PolicyIteration::PolicyIteration(explicitDomainSpecification& domSpec)
    : Algorithm(domSpec),
      iteration(0),
      stateId(0)
{
    domainStates = domSpec.getStates(domainStates);

    identifyStates();

    initialiseActions();
    
    configureActions();

    initialiseValueVectors();

    configurePolicy(domSpec);
}

void PolicyIteration::configurePolicy(explicitDomainSpecification& domSpec)
{
    policy = domSpec.getPolicy();
}

void PolicyIteration::initialiseValueVectors()
{
    lastIteration = ValueVector(domainStates.size());
    
    thisIteration = ValueVector(domainStates.size());

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
        else
            initialReward[stateIds[*domainState]] = (*domainState)->getReward();
        
        lastIteration[stateIds[*domainState]]
            = initialReward[stateIds[*domainState]];
    }
    
}

void PolicyIteration::configureActions(bool ignoreExplicit)
{
    configureActions(domainStates.begin(), ignoreExplicit);
}

void PolicyIteration::configureActions(vector<eState*>::const_iterator domainState, bool ignoreExplicit)
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


void PolicyIteration::identifyStates()
{
    stateId = 0;
    identifyStates(domainStates.begin());
}
    
void PolicyIteration::identifyStates(vector<eState*>::const_iterator domainState)
{   
    /*Make the states identifiable.*/
    for(//domainState = domainStates.begin()
            ; domainState != domainStates.end()
            ; ++domainState, ++stateId)
        stateIds[*domainState] = stateId;
}

void PolicyIteration::initialiseActions()
{
    ActionSpecification* actions = domSpec->getActionSpecification();
    
    for(ActionSpecification::const_iterator i = actions->begin()
            ; i != actions->end()
            ; i++)
        actionMatrices[i->first] = Matrix(domainStates.size(), domainStates.size());
    
}

void PolicyIteration::updateStateValues()
{
    int i = 0;
    vector<eState*>::iterator state;
    for(i = 0, state = domainStates.begin()
            ; state != domainStates.end()
            ; ++state, ++i)
        (*state)->setValue(thisIteration[i]);
    
}


bool PolicyIteration::terminate(const ValueVector thisIteration, const ValueVector lastIteration)
{   
    ValueVector negatedLastIteration(lastIteration.size());
    ValueVector norm(lastIteration.size());
    
    copy(scaled(lastIteration, -1.0), negatedLastIteration);
    add(thisIteration, negatedLastIteration, norm);
    int normIndex = max_index(norm);

    Algorithm::error = norm[normIndex];
    
    return (norm[normIndex] <= (epsilon * ((1 - gamma)/(2*gamma))));
}

bool PolicyIteration::operator()()
{
    iteration++;

    /*The value function that shall be calculated during this
      iteration.*/
    thisIteration = ValueVector(domainStates.size());

    /*State transition matrix relevant to the current policy.*/
    Matrix LU(domainStates.size(), domainStates.size());

    /*Initialise $LU$.

      For each state.*/
    for(vector<eState*>::const_iterator state = domainStates.begin()
        ; state != domainStates.end()
        ; ++state)
    {
        /*What is the policy at the current state?*/
        string action;
        if((*policy).count(*state)
           )//&& (*state)->getColour() != EXPLICIT)/*If a policy exists*/
            action = (*policy)[*state];
        else/*otherwise, must be a fringe state.*/
            action = (*policy)[*state] = actionMatrices.begin()->first;
            
        /*Add the transition probabilities associated with $state$ to
          the appropriate row of $LU$.*/
        copy(actionMatrices[action][stateIds[*state]], LU[stateIds[*state]]);
    }
    
    /*Create a square identity matrix with the same dimensions as
      $LU$.*/
    IdentMat I(domainStates.size(), domainStates.size(), 0, 0);
    mtl::set_value(I, 1.0);
    
    copy(scaled(LU, gamma), LU);
    
    add(scaled(I, -1.0), LU);
    
    /*Pivot vector.*/
    dense1D<int> pvector(domainStates.size());

    ValueVector initialRewardT(domainStates.size());
    copy(initialReward, initialRewardT);
    scale(initialRewardT, -1);
    
    Matrix LUcopy(domainStates.size(), domainStates.size());
    copy(LU, LUcopy);
    
    /*Solve $(LU - I) V_pi = - initialReward$*/
    lu_factorize(LU, pvector);
    lu_solve(LU, pvector, initialRewardT, thisIteration);
    
    /*Storage for the best improvment value found so far.*/
    ValueVector bestValue = ValueVector(domainStates.size());
    copy(thisIteration, bestValue);
    
    /*For each action.*/
    for(DenseActionMatrices::const_iterator action = actionMatrices.begin()
            ; action != actionMatrices.end()
            ; ++action)
    {
        ValueVector tmp(domainStates.size());
        
        mult(action->second, thisIteration, tmp);
        
        copy(scaled(tmp, gamma), tmp);
        
        add(initialReward, tmp);
        
        /*Was this $action$ superior for any state?*/
        for(unsigned int j = 0;  j < domainStates.size(); ++j)
        {
            if(tmp[j] > bestValue[j])//thisIteration[j])
            {
                //thisIteration[j] = tmp[j];
                bestValue[j] = tmp[j];
                (*policy)[domainStates[j]] = action->first;
            }
        }    
    }
    
    terminate(thisIteration, lastIteration);
    copy(thisIteration, lastIteration);
    
    bool terminate = (*policy == oldPolicy);
    oldPolicy = *policy;
    return terminate;
}
