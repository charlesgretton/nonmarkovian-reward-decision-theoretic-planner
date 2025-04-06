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

//#include"actionSpecification.h++"
#include"domainSpecification.h++"

using namespace MDP;
using namespace std;

ActionSpecification::ActionSpecification(const ActionSpecification& actSpec)
{
    mgr = actSpec.mgr;
    actionSpec = actSpec.actionSpec;

    /*Make copies of the argument specification CPTS*/
    for(aIterator p = actionSpec.begin()
	    ; p != actionSpec.end()
	    ; ++p)
	{
	    CPTS *tmp = new CPTS;
	    *tmp = *p->second;
	    p->second = tmp;
	}
}

ActionSpecification::~ActionSpecification()
{
    for(aIterator p = actionSpec.begin()
	    ; p != actionSpec.end()
	    ; ++p)
	{
	    delete  p->second;
	}
}

ActionSpecification::const_iterator ActionSpecification::begin()const
{return actionSpec.begin();}

ActionSpecification::const_iterator ActionSpecification::end()const
{return actionSpec.end();}

ActionSpecification::iterator ActionSpecification::begin()
{return actionSpec.begin();}

ActionSpecification::iterator ActionSpecification::end()
{return actionSpec.end();}


Cudd& ActionSpecification::getManager() const {return mgr;}

void ActionSpecification::addSpecComponent(action s, CPTS* c)
{  
    actionSpec[s]=c;
}

void ActionSpecification::printActions() const
{   
    for(caIterator p = actionSpec.begin()
	    ; p != actionSpec.end()
	    ; ++p)
	{
	    cout<<"Action::"<<p->first<<endl;

	    for(cIterator q = p->second->begin()
		    ; q != p->second->end()
		    ; ++q)
		{
		    cout<<"\t Proposition::"<<q->first<<endl;

		    /*Manifest constants marked for removal*/
		    q->second.print(2,2);
		}
	}
}


unsigned int ActionSpecification::size()const
{
    return actionSpec.size();
}
