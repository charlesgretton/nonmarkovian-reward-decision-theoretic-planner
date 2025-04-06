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

#include"domainSpecification.h++"

#include"rewardSpecification.h++"
#include"formulaPrinter.h++"
#include"formulaNormaliser.h++"

using namespace MDP;

using namespace Formula;

DomainSpecification::DomainSpecification()
{
    rewardSpecification = new RewardSpecification();
    actionSpecification = new ActionSpecification();

    /*This object is responsible for the action specification
      deletion.*/
    deleteActions = true;
}

DomainSpecification::DomainSpecification(const DomainSpecification& domSpec)
{
    /*Make a copy of the reward specification.*/
    rewardSpecification = domSpec.rewardSpecification->copy();
    
    /*Make a copy of the action specification.*/
    actionSpecification = domSpec.actionSpecification;

    /*Because this borrows the actions of the caller it must not
      delete the actions.*/
    deleteActions = false;
    
    /*Make a copy of the start state.*/
    startStatePropositions = domSpec.startStatePropositions;

    /*Make a copy of the state characterising propositions.*/
    propositions = domSpec.propositions;
}

DomainSpecification::~DomainSpecification()
{
    delete rewardSpecification;
    
    /*Does the action specification require deletion?*/
    if(deleteActions)
        delete actionSpecification;
}

void DomainSpecification::addToStartState(string str)
{
    startStatePropositions.insert(str);
}

DomainSpecification::PropositionSet DomainSpecification::getStartStatePropositions()const
{
    return startStatePropositions;
}

void DomainSpecification::addProposition(proposition s)
{
    PropositionVector::iterator id;

    id = find(propositions.begin(), propositions.end(), s);

    /*If $s$ is not found.*/
    if(propositions.end() == id)
	{
	    propositions.push_back(s);
	}
}

string DomainSpecification::getProposition(const propositionIndex proposition)const
{
    return propositions[proposition];
}

DomainSpecification::propositionIndex DomainSpecification::getProposition(const proposition& label) const
{
    PropositionVector::const_iterator id;

    id = find(propositions.begin(), propositions.end(), label);

    /*If $label$ is found.*/
    if(propositions.end() != id)
        return distance(propositions.begin(), id);
    else
	{
	    propositionIndex ret = propositions.size();
	    return ++ret;
	}
}

DomainSpecification::PropositionIndexVector DomainSpecification::getAssociatedPropositions(const PropositionSet& propositionSet) const
{
    PropositionIndexVector answer(propositionSet.size());
    
    int i;
    
    PropositionSet::const_iterator p;
    
    for(i = 0, p = propositionSet.begin()
            ; p != propositionSet.end()
            ; ++p, ++i)
        answer[i] = getProposition(*p);
    
    return answer;
}


ActionSpecification* DomainSpecification::getActionSpecification()
{
    return actionSpecification;
}

RewardSpecification* DomainSpecification::getRewardSpecification()
{
    return rewardSpecification;
}

RewardSpecification const* DomainSpecification::getRewardSpecification()const
{
    return rewardSpecification;
}

DomainSpecification::PropositionVector DomainSpecification::getPropositions() const
{
    return propositions;
}

void DomainSpecification::addSpecComponent(string s, double d, formula* f)
{
    RewardSpecification *tmp = rewardSpecification->addSpecComponent(s, d, f);
    
    /*If a change was made to the reward specification.*/
    if(0 != tmp)
	{
	    /*Remove the old reward type.*/
	    if(rewardSpecification != tmp)
        {
            delete rewardSpecification;
            
            /*Keep the new reward type.*/
            rewardSpecification = tmp;
        }
	}    
}


void DomainSpecification::addSpecComponent(action s, ActionSpecification::CPTS* cpt)
{
    actionSpecification->addSpecComponent(s, cpt);
}

unsigned int DomainSpecification::memory()const
{
    unsigned int size = sizeof(*this);
    
    size += rewardSpecification->memory();

    for(PropositionVector::const_iterator p
            = propositions.begin()
            ; p != propositions.end()
            ; ++p)
        size += sizeof(char) * p->size();

    for(PropositionSet::const_iterator p
            = startStatePropositions.begin()
            ; p != startStatePropositions.end()
            ; ++p)
        size += sizeof(char) * p->size();
    
    size += sizeof(deleteActions);
    return size;
}

void DomainSpecification::printDomain()const
{
    cout<<"Domain start state comprises propositions:\n";
    cout<<"{";
    for(PropositionSet::const_iterator p = startStatePropositions.begin()
            ; p != startStatePropositions.end()
            ; ++p)
    {
        /*We want a string based representation of every state in this
          specification.*/
        cout<<(*p)<<",";
    }
    cout<<"}"<<endl;
    
    cout<<"Domain reward is:\n";
    cout<<rewardSpecification->toString();
    cout<<"Domain actions are:\n";
    actionSpecification->printActions();
}

bool DomainSpecification::operator==(const DomainSpecification& domSpec) const
{
    return propositions == domSpec.getPropositions() &&
        startStatePropositions == domSpec.getStartStatePropositions() &&
        actionSpecification == domSpec.actionSpecification &&//domSpec.getActionSpecification() &&
        (*rewardSpecification) == *(domSpec.rewardSpecification);//*(domSpec.getRewardSpecification());
}

bool DomainSpecification::operator!=(const DomainSpecification& domSpec) const
{
    return !(operator==(domSpec));
}
