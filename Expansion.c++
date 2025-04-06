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

#include"Expansion.h++"

#include"EntailmentFilter.h++"

using namespace MDP;

/******************************************************************/

bool ZeroProbability::operator()(const TransitionPair& pair) const
{   
    return 0.0 == pair.first;
}

/******************************************************************/

void RemoveWithZeroProbability::operator()(TransitionPair& pair) const
    {
        ZeroProbability zeroProbability;
        
        if(zeroProbability(pair))
            delete pair.second;
    }

/******************************************************************/
State* Expansion::newState() const
{
    State* tmp = stateLastToExpand->copy();
 
    tmp->clear();
    
    return tmp;
}

void Expansion::produceTransitions(const DomainSpecification& domSpec,
                                   const vector<double>& probabilities,
                                   ActionPossibilities& actions) const
{
    /*For every element in \argument{probabilities}*/
    for(unsigned int i = 0; probabilities.size() > i; i++)
    {
        /*If there are no transitions currently, then add states in
          which the proposition associated with $i$ is true and untrue
          respectively.*/
        if(0 == actions.size())
        {
            State *tmp;
            if(0.0 != probabilities[i])
                {
                    tmp = newState();
            
                    tmp->addProposition(domSpec.getProposition(i));
        
                    actions.push_back(TransitionPair(probabilities[i],
                                             tmp));
                }
            if(0.0 != 1 - probabilities[i])
                {
                    tmp = newState();
                    
                    actions.push_back(TransitionPair(1 - probabilities[i],
                                                     tmp));
                }
        }
        else
        {
            /*Number of elements in the action specification before we add
              transitions associated with $probabilities[i]$.*/
            unsigned int end = actions.size();

            /*Are we required to add states with $domSpec.getProposition(i)$.*/
            if(0.0 != probabilities[i])
                {
                    /*Add states to the action transitions where proposition $i$ is
                      true and adjust the probability of such a state being according
                      to $probabilities[i]$.*/
                    for(unsigned int j = 0; j != end; ++j)
                        {
                            State *tmp = actions[j].second->copy();
                            
                            tmp->addProposition(domSpec.getProposition(i));
                            
                            TransitionPair tmptTP =
                                TransitionPair(actions[j].first * probabilities[i],
                                               tmp);
                            
                            actions.push_back(tmptTP);
                        }
                }

            if(0.0 != 1 - probabilities[i])
                /*The probabilities of previous states, states in which $i$ is not
                  true, being transitioned to, are adjusted according to
                  $probabilities[i]$.*/
                for(unsigned int j = 0; j != end; ++j)
                    actions[j].first *= 1 - probabilities[i];
            else
                {
                    /*Clean up states to be deleted.*/
                    for(unsigned int j = 0; j != end; ++j)
                        delete actions[j].second;

                    ActionPossibilities tmpActions;
                    for(unsigned int j = end; j != actions.size(); ++j)
                        tmpActions.push_back(actions[j]);
                    actions = tmpActions;
                }
        }
    
        /*Clean up states that have a zero transition probability.*/

//          ZeroProbability zProb;
    
//          ActionPossibilities::iterator deleteFrom = 
//              remove_if(actions.begin(), actions.end(), zProb);

//          RemoveWithZeroProbability removeWithZeroProbability;
//          for_each(deleteFrom, actions.end(), removeWithZeroProbability);
    
//          actions.erase(deleteFrom, actions.end());
    }
}

double Expansion::getProbability(DdNode const* node,
                                 Cudd const* _cudd,
                                 const DomainSpecification::PropositionIndexVector& propositions) const
{
    double result = 0.0;
    
    Cudd* cudd = const_cast<Cudd*>(_cudd);
    
    if(((node)->index == CUDD_CONST_INDEX))
    {
        if(node != cudd->background().getNode()
           && node != cudd->addZero().getNode())
            return (node)->type.value;/*Value associated with a child node.*/
    }
    else
    {
        /*Node that leads to the desired probability.*/
        DdNode* nextNode;

        DomainSpecification::PropositionIndexVector::const_iterator id =
            find(propositions.begin(), propositions.end(), node->index);

        /*If the proposition is deemed $true$.*/
        if(id != propositions.end())
            nextNode  = ((node)->type.kids.T);
        /*If the proposition is deemed $false$.*/
        else
            nextNode  = ((node)->type.kids.E);

        /*Search the $nextNode$ child*/
        result += getProbability(nextNode, cudd, propositions);
    }
  
    return result;
}

void Expansion::adjustStates(ActionPossibilities& actions,
                             const StateLabelling& nmrsLabels) const
{}

StateTransitionMatrices* Expansion::operator()
    (State& state,
     DomainSpecification& domSpec,
     const StateLabelling& nmrsLabels) const
{
    /*Handle to \argument{domSpec} \class{ActionSpecification}.*/
    ActionSpecification* asp = domSpec.getActionSpecification();

    /*Domain Specification propositions.*/
    DomainSpecification::PropositionVector propVec =
            domSpec.getPropositions();

    /*Identity of propositions that are $true$ in the state this is
      expanding.*/
    DomainSpecification::PropositionIndexVector statePropositionIds =
        domSpec.getAssociatedPropositions(state.getPropositions());

    /*Vector of probabilities of variables corresponding to a vector
      index becoming true on a transition.*/
    vector<double> probabilities(propVec.size());

    /*Transition information that this function builds.*/
    StateTransitionMatrices *stms = new StateTransitionMatrices();

    /*The \argument{state} is the last state to be expanded.*/
    stateLastToExpand = &state;
    
    /*For each $action$.*/
    for(ActionSpecification::caIterator action = asp->begin()
            ; action != asp->end()
            ; ++action)
    {    
        /*For each proposition $prop$.*/
        for(DomainSpecification::PropositionVector::const_iterator prop =
                propVec.begin()
                ; prop != propVec.end()
                ; ++prop)
        {
            /*Find the ADD in $action$ associated with $prop$.*/
            ActionSpecification::ccIterator id = action->second->find(*prop);

            /*If the probability of $prop$ becoming true is determined
              by its truth in this state (truth is maintained all
              other things being equal).*/
            if(action->second->end() == id)
            {   
                /*Is $prop$ true in the $state$?*/
                DomainSpecification::PropositionSet::const_iterator id
                    = state.getPropositions().find(*prop);

                /*If the proposition is seen to become $false$.*/
                if(id == state.getPropositions().end())
                    probabilities[domSpec.getProposition(*prop)] = 0.0;
                else
                    probabilities[domSpec.getProposition(*prop)] = 1.0;
            }
            else/*There is a non-zero probability associated with $prop$
                  becoming true.*/
            {
                /*Make a $tmp$ handle to ADD in $p$ associated with $prop$.*/
                ADD tmp = id->second;

                probabilities[domSpec.getProposition(*prop)] =
                    getProbability(tmp.getNode(),
                                   tmp.manager(),
                                   statePropositionIds);
            }
        }

        /*Containment for the successor state and probability of
          transition to that state from $stateLastToExpand$ with
          action $p->second$.*/
        ActionPossibilities actions;

        produceTransitions(domSpec, probabilities, actions);

        /*see \method{adjustStates()}*/
        adjustStates(actions, nmrsLabels);
        
        (*stms)[action->first] = actions;  
    }

    return stms;
}


/******************************************************************/

void StateAnnotationExpansion::adjustStates(ActionPossibilities& actions,
                                            const StateLabelling& nmrsLabels) const
{   
    /*Calculate and adjust the reward specifications of the states.*/
    for(ActionPossibilities::iterator act = actions.begin()
            ; act != actions.end()
            ; ++act)
        {
            /*If we are going for a minimised state space.*/
            if(0 != nmrsLabels.size())
                {
                    const DomainSpecification::PropositionSet& some
                        = act->second->getPropositions();
                    
                    StateLabelling* tmp = const_cast<StateLabelling*>(&nmrsLabels);
                    
                    dynamic_cast<basedExpandedState*>(act->second)
                        ->setLabelSet( *(*tmp)[some] );

                    
                    /*In some instances state adjustment requires a filter.*/
                    MinimalEntailmentFilter filter =
                        MinimalEntailmentFilter(dynamic_cast<basedExpandedState*>(act->second)->getRewardSpecification());
                    
                    act->second->calculateReward(&filter);
                }
            else
                act->second->calculateReward();
        }
}
