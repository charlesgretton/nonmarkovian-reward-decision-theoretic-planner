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

/*
 * \paragraph{:Purpose:}
 *
 * Representation of and operations concerning NMRDPs action
 * specifications.
 **/

#ifndef ACTION_SPEC
#define ACTION_SPEC

#include"include/cuddObj.hh"
#include<string>
#include<map>

#include"SpecificationTypes.h++"

using namespace std;

namespace MDP
{ 
    class ActionSpecification
    {
    public:
        /*Vector initialisation and specification construction.*/
        ActionSpecification(){}

        /*Copy construction.*/
        ActionSpecification(const ActionSpecification&);
        
        /*Type definition for the conditional probability table.*/
        typedef map<proposition, ADD> CPTS;

        /*Constant CPTS iterator.*/
        typedef CPTS::const_iterator ccIterator;

        /*CPTS iterator.*/
        typedef CPTS::iterator cIterator;

        /*Storage type for action specifications.*/
        typedef map<action, CPTS*> ActionSpec;

        /*Constant action iterator.*/
        typedef ActionSpec::const_iterator caIterator;

        /*Action iterator.*/
        typedef ActionSpec::iterator aIterator;

        /*Free resources taken by the free store CPTS.*/

        ~ActionSpecification();

        typedef ActionSpec::const_iterator const_iterator;
        typedef ActionSpec::iterator iterator;
        
        const_iterator begin()const;
        const_iterator end()const;
        iterator begin();
        iterator end();

        /*Obtain the cudd manager.*/
        Cudd& getManager() const;
      
        /*Print the actions to the argument output stream.*/
        void printActions() const;

        /*Add reward specification to the world.*/
        void addSpecComponent(action, CPTS*);

        /*See \member{actionSpec.size()}*/
        unsigned int size()const;
    protected:
        /*Cudd manager. This is used for third party garbage
          collection and ADD node construction.*/
        mutable Cudd mgr;

        /*Action specification. Actions are identifiable by string
          keys.*/
        ActionSpec actionSpec;
    };
}
#endif
