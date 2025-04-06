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

#include"formulaSimplifier.h++"
#include"formulaNormaliser.h++"

using namespace Formula; 

/*FLTL*/

void FLTLsimplifier::visit(const nxt& f)
{    
    accept_(f[0]);

    item = new nxt(item);
}

void FLTLsimplifier::visit(const nxtDisj& f)
{    
    accept_(f[0]);

    item = new nxtDisj(item, f.getDepth());
}

void FLTLsimplifier::visit(const nxtConj& f)
{    
    accept_(f[0]);

    item = new nxtConj(item, f.getDepth());
}

void FLTLsimplifier::visit(const nxtNest& f)
{    
    accept_(f[0]);

    item = new nxtNest(item, f.getDepth());
}

void FLTLsimplifier::visit(const fut& f)
{    
    accept_(f[0]);
    formula *tmp = item;
    accept_(f[1]);

    item = new fut(tmp, item);
}

void FLTLsimplifier::visit(const strFut& f)
{    
    accept_(f[0]);
    formula *tmp = item;
    accept_(f[1]);

    item = new strFut(tmp, item);
}

void FLTLsimplifier::visit(const fbx& f)
{    
    accept_(f[0]);

    item = new fbx(item);
}

void FLTLsimplifier::visit(const fdi& f)
{    
    accept_(f[0]);

    item = new fdi(item);
}

void FLTLsimplifier::visit(const dollars& dol)
{
    item = new dollars(dol);
}

/*PLTL*/

void PLTLsimplifier::visit(const startStateProposition& f)
{
    item = new startStateProposition();
}

void PLTLsimplifier::visit(const prv& f)
{    
    accept_(f[0]);

    item = new prv(item);
}

void PLTLsimplifier::visit(const prvDisj& f)
{    
    accept_(f[0]);

    item = new prvDisj(item, f.getDepth());
}

void PLTLsimplifier::visit(const prvConj& f)
{    
    accept_(f[0]);

    item = new prvConj(item, f.getDepth());
}

void PLTLsimplifier::visit(const prvNest& f)
{    
    accept_(f[0]);

    item = new prvNest(item, f.getDepth());
}

void PLTLsimplifier::visit(const snc& f)
{    
    accept_(f[0]);
    formula *tmp = item;
    accept_(f[1]);

    item = new snc(tmp, item);
}

void PLTLsimplifier::visit(const pbx& f)
{    
    accept_(f[0]);

    item = new pbx(item);
}

void PLTLsimplifier::visit(const pdi& f)
{
    accept_(f[0]);

    item = new pdi(item);
}

