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

/* ADDstateCounter.c++
 */

#include "common.h++"
#ifdef HEADER

#include "ExtADD.h++"

class ADDstateCounter
{
private:
    double count;
};

#endif
#include "ADDstateCounter.h++"
#include <cmath>
using namespace std;

PUBLIC ADDstateCounter::ADDstateCounter(ExtADD add, int num_variables)
{
    count = visitNode(add, num_variables);
}

PUBLIC double ADDstateCounter::getCount()
{
    return count;
}

PUBLIC double ADDstateCounter::visitNode(const ExtADD& node, int free_variables)
{
    if (node.isConstant()) {
        if (node.value() > 0)
            return pow(2.0, free_variables);
        return 0.0;
    } else {
        return
            visitNode(node.thenChild(), free_variables - 1) +
            visitNode(node.elseChild(), free_variables - 1);
    }
}

