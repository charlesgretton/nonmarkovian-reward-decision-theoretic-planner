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

/* ActionADDmap.c++
 *
 * \paragraph{:Purpose:}
 *
 * Simplifies the construction and storage of action diagrams for each
 * action in the domain specification.
 */

#include "common.h++"
#ifdef HEADER

#include <map>
#include "ActionADD.h++"
using namespace std;

class ActionADDmap : public map<action, ExtADD>
{
private:
};

#endif
#include "ActionADDmap.h++"
#include "domainSpecification.h++"

PUBLIC ActionADDmap::ActionADDmap(DomainSpecification& domSpec, const ExtADD& constraint)
{
    ActionSpecification& actionSpec = *domSpec.getActionSpecification();
    map<action, ActionSpecification::CPTS*>::iterator i;
    
    for (i = actionSpec.begin(); i != actionSpec.end(); ++i) {
        (*this)[i->first] = ActionADD(domSpec, i->second, constraint).getADD();
    }
}

