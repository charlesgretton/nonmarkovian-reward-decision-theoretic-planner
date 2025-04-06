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

#include"States.h++"
#include"domainSpecification.h++"
#include"rewardSpecification.h++"
#include"RewardCalculation.h++"

using namespace MDP;

/*******************************************************State equality*/


bool StateEqual::operator()(State const* sL, State const* sR) const
{
    return operator()(*sL, *sR);
}


bool StateEqual::operator()(const State& sL, const State& sR) const
{
    return sL == sR;
}

/*******************************************************State equality*/

bool StateValueGeq::operator()(State const* s1, State const* s2) const
{
    return s1->getValue() < s2->getValue(); 
}

bool StateValueGeq::operator()(const State& s1, const State& s2) const
{
    return operator()(&s1, &s2); 
}

/*******************************************************State possibility*/

bool IsPossible::operator()(State const* s) const
{
    return s->isPossible();
}

/*******************************************************Pair possibility*/

bool PairIsPossible::operator()(const TransitionPair& tp)const
{
    return tp.second->isPossible();
}

/*******************************************************State Implementation*/


State::State(const DomainSpecification::PropositionSet& propositions)
    :propositions(propositions),
     possible(true),
     colouring(FRINGE),
     stmsEmptied(false)
{
    setReward( 0.0 );
}

State::State(const State& inState)
    :propositions(inState.propositions),
     possible(inState.possible),
     colouring(FRINGE),
     stmsEmptied(false)
     
{
    setReward( inState.reward );
}

State::~State()
{}

bool State::operator==(const State& s) const
{
    return (reward == s.reward) &&
        possible == s.possible &&
        propositions == s.propositions;
}

bool State::operator<(const State& s) const
{
    return (reward < s.reward) ||
        ( reward == s.reward && ( possible < s.possible ||
                                  ( possible == s.possible &&
                                    (propositions < s.propositions)) ) );
}

bool State::isPossible()const
{
    return possible;
}

void State::calculatePossibility()
{
    if(!stmsEmptied)
        possible = !stms.empty();
    else
        possible = true;
}


const DomainSpecification::PropositionSet& State::getPropositions() const
{
    return propositions;
}

void State::addProposition(proposition p)
{
    propositions.insert(p);
}

void State::removeProposition(proposition p)
{
    propositions.erase(p);
}

DomainSpecification::PropositionSet State::getNonPropositions
(const DomainSpecification& domSpec) //const
{
    /*The set of domain propositions.*/
    DomainSpecification::PropositionVector domSpecPropositionsVector =
        domSpec.getPropositions();

    /*An actual set representation of the domain propositions.*/
    DomainSpecification::PropositionSet domProps =
        DomainSpecification::PropositionSet(domSpecPropositionsVector.begin(),
                                            domSpecPropositionsVector.end());

    DomainSpecification::PropositionVector nonPropositions_(
        domProps.size() - propositions.size());

    /*Obtain the domain propositions that are not true in this state.*/
    set_difference(domProps.begin(),
                   domProps.end(),
                   propositions.begin(),
                   propositions.end(),
                   nonPropositions_.begin());

    DomainSpecification::PropositionSet
        nonPropositions(nonPropositions_.begin(),
                        nonPropositions_.end());
    
    return nonPropositions;
}

void State::addSuccessors(const StateTransitionMatrices& stms)
{
    this->stms = stms;
}

State* State::copy() const
{
    return new State(*this);
}

void State::clear()
{
    StateTransitionMatrices tmpSTM;
    DomainSpecification::PropositionSet prop;
    
    stms = tmpSTM;
    
    propositions = prop;
}

string State::toString(bool identify)const
{
    ostringstream answer;

    if(!identify)
    {
        answer<<"State :: \n";
        
        answer<<"\tState at address "<<this<<endl;
        answer<<"\tPossible = "<<possible<<endl;
        answer<<"REWARD = "<<reward<<endl;
        answer<<"\tPropositions that are true in this state are :: \n";
        
        answer<<propositionsToString();
        answer<<endl;
    
        answer<<transitionsString();
    } else {
        answer<<possible;
        answer<<reward;
        answer<<propositionsToString();
    }
    
    return answer.str();
}

string State::propositionsToString()const
{
    ostringstream answer;
    
    for(DomainSpecification::PropositionSet::const_iterator p = propositions.begin()
            ; p != propositions.end()
            ; ++p)
        answer<<*p<<", ";
    
    return answer.str();
}

bool State::calculateReward(BuildFilter<formula>* buildFilter)
{
    return true;
}

double  State::getReward()const
{
    return reward;
}

void  State::setReward(double reward)
{
    this->reward = value = reward;
}

unsigned int State::memory()const
{
    unsigned int size = sizeof(*this);

    for(DomainSpecification::PropositionSet::const_iterator p
            = propositions.begin()
            ; p != propositions.end()
            ; ++p)
        size += sizeof(char) * p->size();

    size += sizeof(reward);
    size += sizeof(value);
    size += sizeof(possible);

    for(StateTransitionMatrices::const_iterator p = stms.begin()
            ; p != stms.end()
            ; ++p )
        {
            size += sizeof(char) * p->first.size();
                       for(ActionPossibilities::const_iterator q = p->second.begin()
                               ; q != p->second.end()
                               ; ++q)
                       {
                           size += sizeof(q->first);
                           size += sizeof(q->second);
                       }
                       
        }                       
            
    return size;
}

void State::setColour(int colour)
{
    colouring = colour;
}

int State::getColour()
{
    return colouring;
}

string State::transitionsString()const
{
    ostringstream answer;
    
    for(StateTransitionMatrices::const_iterator p = stms.begin()
            ; p != stms.end()
            ; ++p)
    {
        answer<<"\tOn action "<<p->first
              <<" the following states may be reached ::\n";
        
        for(ActionPossibilities::const_iterator q = p->second.begin()
                ; q != p->second.end()
                ; ++q)
        {
            answer<<"\t\t"<<q->second<<" "<<q->first<<endl;
        }
    }
    
    return answer.str();
}


StateSet State::getSuccessors()const
{
    StateSet result;

    /*For all the possible actions.*/
    for(StateTransitionMatrices::const_iterator p = stms.begin()
            ; p != stms.end()
            ; ++p)
    {
        /*For every transition pair.*/
        for(ActionPossibilities::const_iterator q = p->second.begin()
                ; q != p->second.end()
                ; ++q)
        {
            result.insert(q->second);
        }
    }

    return result;
}

void State::removeImpossibleActions()
{
    StateTransitionMatrices tmp = stms;

    PairIsPossible isPossible;

    /*For all executable $actions$.*/
    for(StateTransitionMatrices::const_iterator p = tmp.begin()
            ; p != tmp.end()
            ; ++p)
    {
        /*For every successor on execution of the $action$. */
        ActionPossibilities::const_iterator id;
        for(id = p->second.begin()
                ; id != p->second.end() && isPossible(*id)
                ; ++id);
        
        if(p->second.end() != id)
            stms.erase(p->first);
    }
}

void State::removeAllActions()
{
    stms = StateTransitionMatrices();
    stmsEmptied = true;
}

void State::setValue(double value)
{
    this->value = value;
}

double State::getValue()const
{
    return value;
}

/*******************************************************eState Implementation*/

eState::eState(DomainSpecification::PropositionSet& propositions
               , RewardSpecification& rewardSpecification)
    :State(propositions)
{
    this->rewardSpecification = rewardSpecification.copy();
    
    //possible = calculateReward();
}
        
eState::eState(const eState& eState)
    :State(eState)
{
    rewardSpecification = eState.rewardSpecification->copy();
}

eState::~eState()
{
    delete rewardSpecification;
}

bool eState::operator==(const State& s) const
{
    eState const* tmp = dynamic_cast<eState const*>(&s);
    
    if(0 != tmp)
        return (*this) == *tmp;
    
    return false;
}

bool eState::operator==(const eState& s) const
{
      return (State::operator==(s)) &&
          (rewardSpecification->toString() == s.rewardSpecification->toString());
}

bool eState::operator<(const eState& s) const
{
    return (State::operator<(s)) || ( State::operator==(s) && 
        (rewardSpecification->toString() < s.rewardSpecification->toString()) );
}

State* eState::copy() const
{
    return new eState(*this);
}

void eState::clear()
{
    State::clear();
}

void eState::hasPredecessor(bool hp)
{}

bool eState::calculateReward(BuildFilter<formula>* buildFilter)
{
    RewardCalculator* rewardCalculator = rewardSpecification->getRewardCalculator();
    pair<RewardSpecification**, RewardSpecification*> p;
    p.first = &rewardSpecification;
    setReward( (*rewardCalculator)(p, propositions) );
    delete rewardCalculator;

    possible = rewardSpecification->isPossible();
    
    return possible;
}

void eState::generateReward()
{
    calculateReward();
}

RewardSpecification const* eState::getRewardSpecification() const
{
    return rewardSpecification;
}

string eState::toString(bool identify)const
{
    string answer;
    
    if(!identify)
    {
        answer = State::toString() + "\n"
            + "With reward specification\n" + rewardSpecification->toString();
    } else {
        answer = State::toString() + rewardSpecification->toString();
    }
    
    return answer;
}

unsigned int eState::memory()const
{
    unsigned int size = State::memory() + sizeof(*this) - sizeof(State);

    size += rewardSpecification->size();
    
    return size;
}


/******************************************basedExpandedState Implementation*/

basedExpandedState::basedExpandedState
(DomainSpecification::PropositionSet& propositionSet,
 RewardSpecification& parentReward,
 RewardSpecification& labelSet)
    :eState(propositionSet, parentReward)
{
    this->labelSet = labelSet.copy();
}

basedExpandedState::basedExpandedState(const basedExpandedState& baseState)
    :eState(baseState)
{
    this->labelSet = baseState.labelSet->copy();
}

basedExpandedState::basedExpandedState(const eState& baseState,
                                       RewardSpecification* labelSet)
    :eState(baseState)
{
    this->labelSet = labelSet;
}

basedExpandedState::~basedExpandedState()
{
    delete labelSet;
}

State* basedExpandedState::copy() const
{
    return new basedExpandedState(*this);
}

void basedExpandedState::hasPredecessor(bool hp)
{
    if(!hp)
        {
            delete rewardSpecification;
            rewardSpecification = 0;
        }
}

bool basedExpandedState::calculateReward(BuildFilter<formula>* buildFilter)
{
    /*One may be encouraged by the implementation of
      \method{eState.calculateReward()} to use the \member{rewardSpecification}
      as a means of obtaining a \class{RewardCalculator}, however this
      could be a grave mistake. What if this is a start state with no
      predecessor? In that case there is no parent reward
      specification and so to obtain the appropriate regression we
      shall use the \member{labelSet} instead. */
    RewardCalculator* rewardCalculator = labelSet->getRewardCalculator();
    pair<RewardSpecification**, RewardSpecification*> p;
    p.first = &rewardSpecification;
    p.second = labelSet;
    setReward( (*rewardCalculator)(p, propositions, buildFilter) );
    delete rewardCalculator;

    possible = rewardSpecification->isPossible();
    
    return possible;
}

void basedExpandedState::generateReward()
{
    /*The reward specification should be a copy of the labelling,*/
    rewardSpecification = labelSet->copy();
    /*that has been traversed without filtering.*/
    rewardSpecification->sequenceTraversal(propositions);
}

unsigned int basedExpandedState::getLabelLength() const
{
    return labelSet->size();
}

void basedExpandedState::updatePredecessorReward(basedExpandedState& bes) const
{
    /*For every reward element.*/
    for(RewardSpecification::const_iterator reward = rewardSpecification->begin()
            ; reward != rewardSpecification->end()
            ; ++reward)
    {
        /*If the reward element is not an assigned literal.*/
        if( 0 == dynamic_cast<assLiteral*>(reward->second.form) )
        {   
            string* rewardLabel = reward->second.form->print();
            
            /*Conditional because we think find should be faster than replace.*/
            if( bes.labelSet->end() == bes.labelSet->find(*rewardLabel) )
                bes.labelSet->addSpecComponent(*rewardLabel, 0, reward->second.form->copy());
            
            delete rewardLabel;
        }
    }
}


RewardSpecification* basedExpandedState::getLabelSet()
{
    return labelSet;
}

void basedExpandedState::setLabelSet(const RewardSpecification& newLabelSet)
{
    delete labelSet;
    labelSet = newLabelSet.copy();
}

unsigned int basedExpandedState::memory()const
{
    unsigned int size = eState::memory() + sizeof(*this) - sizeof(eState);

    size += labelSet->size();
    
    return size;
}
