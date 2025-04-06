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

#include"rewardSpecification.h++"

#include"formulaPrinter.h++"
#include"formulaNormaliser.h++"
#include"formulaSequenceTraversal.h++"
#include"RewardCalculation.h++"

using namespace MDP;
using namespace Formula;
using namespace std;

/*******************************************************FSec*/


bool RewardSpecification::FSpec::operator==(const FSpec& fSpec)const
{
    return ((*form) == *fSpec.form)
        && (value == fSpec.value);
}  

/*******************************************************ContainedFormulaEqual*/

bool RewardSpecification::ContainedFormulaEqual::operator()
    (const RewardSpecification::SpecificationContainer::value_type& rSpec,
     const vector<formula const*>& f) const
{
    return (*f[0]) == *rSpec.second.form;
}
    

/*******************************************************RewardAbnormality*/

RewardSpecification::RewardAbnormality::RewardAbnormality
(RewardSpecification::rIterator &p)
    :message("Formula: " + p->first + " is reward abnormal.")
{}

const string RewardSpecification::RewardAbnormality::getMessage()const
{
    return message;
}


/*******************************************************RewardSpecification*/

RewardSpecification::RewardSpecification()
    :expansionRequired(true),
     rewardCache(0)
{}

RewardSpecification::RewardSpecification(const RewardSpecification& rewardSpecification)
    :rewardCache(0)
{
    expansionRequired = rewardSpecification.expansionRequired;

    specificationContents = rewardSpecification.specificationContents;
    
    /*Make copies of the argument specification formulae*/
    for(rIterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
	    p->second.form =
            p->second.form->copy();
	}
}

RewardSpecification::~RewardSpecification()
{
    /*Doesn't matter if this is NULL*/
    delete rewardCache;
    
    for(rIterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
	    delete  p->second.form;
	}
}

RewardSpecification* RewardSpecification::addSpecComponent(string label, double value, formula* form)
{
    /*The specification is changed when an element is added to it.*/
    clearRewardCache();
    
    if(typeExpansionPossible())/*Is an expansion attempt required?*/
	{   
	    /*Pointer to possible expansion.*/
	    RewardSpecification* tmp = expandsTo();

	    /*Have we reached the end of expansion possibilities?*/
	    if(0 == tmp)
            return this;/*Yes*/

	    RewardSpecification* tmp_ = tmp->addSpecComponent(label, value, form->copy()); 

	    /*If a further expansion occurred.*/
	    if(tmp != tmp_)
            delete tmp;
	    
	    /*$tmp_$ shall be $0$ if no expansion was possible.*/
	    if(0 != tmp_ && !tmp_->typeExpansionPossible())
        {
            delete form;/*We don't require this formula anymore*/
            return tmp_;/*as we are going to use the expansion.*/
        }
        else if(0 != tmp_)/*If we aren't going to use the expansion
                            then we delete it.*/
            delete tmp_;
	}

    FSpec fSpec;
    
    fSpec.form = form;
    fSpec.value = value;

    /*If a maplet associated with \argument{formula} has already been
      added to the \member{specificationContents} then it is deleted
      before replacement.*/
    if(specificationContents.end() != specificationContents.find(label))
        delete specificationContents[label].form;
    
    specificationContents[label]=fSpec;

    return this;
}

bool RewardSpecification::typeExpansionPossible()const
{
    return expansionRequired;
}

bool RewardSpecification::isPossible()const
{
    return true;
}

double RewardSpecification::getReward(const string& label)
{
    return (specificationContents[label].value);
}

RewardSpecification::crIterator RewardSpecification::begin()const
{
    return specificationContents.begin();
}

RewardSpecification::crIterator RewardSpecification::end()const
{
    return specificationContents.end();
}

RewardSpecification::rIterator RewardSpecification::begin()
{
    clearRewardCache();/*Spec may change.*/
    
    return specificationContents.begin();
}

RewardSpecification::rIterator RewardSpecification::end()
{
    clearRewardCache();/*Spec may change.*/
    
    return specificationContents.end();
}
    
RewardSpecification::FSpec& RewardSpecification::operator[](const string& label)
{
    clearRewardCache();/*Spec may change.*/
    
    return specificationContents[label];
}

void RewardSpecification::erase(const string& label)
{
    clearRewardCache();/*Spec does change.*/
    
    specificationContents.erase(label);
}

string RewardSpecification::checkDuplication()const
{
    string str;

    for(SpecificationContainer::const_iterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
	    SpecificationContainer::const_iterator q = p;
	    for(++q
		    ; q != specificationContents.end()
		    ; ++q)
		{
		    if((p->second.form)->operator==(*(q->second.form)))
                str += p->first + " is equal to " + q->first + "\n";
		}
	}

    return str;
}

void RewardSpecification::normalise()
{
    clearRewardCache();/*Spec may change.*/
    
    for(rIterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
        formula* tmp;
        
        try
        {
            /*Try to obtain a negation normal version of the $p$
              specification formula.*/
            tmp = (p->second.form)->negNormalise();
        }
        catch(InvalidFLTL invFLTL)/*For some reason the normalised formula
                                    is no good for further use.*/
        {
            /*Create a new negation normaliser for FLTL formulae.*/
            FLTLnormaliser normaliser(false);
            
            /*Normalisation is now for the purpose of reporting an error.*/
            normaliser.isReporting();
            
            /*Normalise the problem formula and store result.*/
            (p->second.form)->accept(&normaliser);    
            formula *complete = normaliser.getBuild();
            
            /*Report the problem formula to the output stream.*/
            cout<<"(Parse Error) -- Reward abnormal formula detected.\n";
            
            cout<<"The reward abnormal formula was:\n";
            cout<<"\t"<<p->first<<" :: "<<*(p->second.form)->print()<<endl;
            cout<<"In negation normal form, this formula appears to be :\n";
            cout<<"\t"<<p->first<<" :: "<<*complete->print()<<endl;
            
            /*No point in any deletion before the application terminates.*/
            
            /*Application shall terminate when a reward formula is
              not reward normal. We throw this exception that is
              not caught in this version of the application. In
              future versions it may be caught.*/
            throw;
        }
        
	    delete p->second.form;
	    p->second.form = tmp;
	}
}

void RewardSpecification::simplify()
{
    clearRewardCache();/*Spec may change.*/
    
    for(rIterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
	    formula *tmp = (p->second.form)->simplify();
	    delete p->second.form;
	    p->second.form = tmp;
	}
}


string& RewardSpecification::toString()const
{
    if(0 != rewardCache)
        return *rewardCache;
    
    ostringstream answer;

    for(crIterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
	    answer<<p->first<<" :: "<<p->second.value<<" :: ";
	    string *tmp = p->second.form->print();
	    answer<<*(tmp);
	    delete tmp;
	    answer<<endl;
	}
    
    /*Update the reward cache.*/
    rewardCache = new string(answer.str());
    
    return *rewardCache;
}


RewardSpecification* RewardSpecification::expandsTo()const
{
    return new FLTLrewardSpecification(*this);
}

double RewardSpecification::sequenceTraversal
(const DomainSpecification::PropositionSet& propositionSet,
 const BuildFilter<formula>& buildFilter)
{   
    PLTLregression<DomainSpecification::PropositionSet const> regressor;
    return sequenceTraversal
        (regressor, propositionSet, buildFilter);/*Spec may change.*/
}

double RewardSpecification::getReward(bool rewarding, 
				      RewardSpecification::rIterator &p,
				      formula *build)const
{
    if(rewarding)
        return p->second.value;
    else
        return 0.0;
}

RewardSpecification* RewardSpecification::copy()const
{
    return new RewardSpecification(*this);
}

bool RewardSpecification::operator==(const RewardSpecification& rewardSpecification) const
{
    return specificationContents
        == rewardSpecification.specificationContents;
}

RewardSpecification::iterator RewardSpecification::find(const string& str)
{
    clearRewardCache();/*Spec may change.*/
    
    return specificationContents.find(str);
}

RewardSpecification::const_iterator RewardSpecification::find(const string& str)const
{
    return specificationContents.find(str);
}

unsigned int RewardSpecification::size() const
{
    return specificationContents.size();
}

unsigned int RewardSpecification::memory()const
{
    unsigned int size = sizeof(*this);

    for(crIterator p = specificationContents.begin()
	    ; p != specificationContents.end()
	    ; ++p)
	{
	    size += p->first.size() * sizeof(char);
        size += sizeof(p->second.value);
	    size += p->second.form->size();
	}
    
    return size;
}

RewardCalculator* RewardSpecification::getRewardCalculator()const
{
    return new PLTLrewardCalculator;
}

void RewardSpecification::clearRewardCache()
{
    delete rewardCache;
    rewardCache = 0;
}

double RewardSpecification::getRewardSum()const
{
    double result;
    for(crIterator p = specificationContents.begin()
            ; p != specificationContents.end()
            ; ++p)
	{
	    result += p->second.value;
	}
    
    return result;
}
