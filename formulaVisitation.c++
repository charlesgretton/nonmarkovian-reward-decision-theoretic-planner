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

#include"formulaVisitation.h++"
#include"formula.h++"

using namespace Formula;

/*Acceptance during formula visitation is dependent on the type of the
  visitor instance. Conditionals on type are not used in the
  visitation architecture, thus the acceptance calls must be made by
  visitor instances.*/


void PCvisitor::accept_(formula const* f)const {f->accept(this);}
void FLTLvisitor::accept_(formula const* f)const {f->accept(this);}
void PLTLvisitor::accept_(formula const* f)const {f->accept(this);}
