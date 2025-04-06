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

/* ADDvisitor.c++
 *
 * \paragraph{:purpose:}
 *
 * Defines an interface for classes that can traverse an ADD.
 */

#include "common.h++"
#ifdef HEADER

class ExtADD;

class ADDvisitor
{
};

#endif
#include "ADDvisitor.h++"

PUBLIC VIRTUAL void ADDvisitor::visitLeaf(const ExtADD& node)
{
}

PUBLIC VIRTUAL void ADDvisitor::visitInternal(const ExtADD& node)
{
}

PUBLIC VIRTUAL void ADDvisitor::visitLeaf(const ExtADD& node) const
{
}

PUBLIC VIRTUAL void ADDvisitor::visitInternal(const ExtADD& node) const
{
}
