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

#include"formulaSize.h++"
#include"formula.h++"

using namespace std;

using namespace Formula;


void FLTLsize::visit(const nxt& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void FLTLsize::visit(const nxtDisj& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void FLTLsize::visit(const nxtConj& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void FLTLsize::visit(const nxtNest& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void FLTLsize::visit(const fut& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
    accept_(f[1]);
}

void FLTLsize::visit(const strFut& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
    accept_(f[1]);
}

void FLTLsize::visit(const fbx& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void FLTLsize::visit(const fdi& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void FLTLsize::visit(const dollars& dol)
{
    (*item) += sizeof(dol);
}

void PLTLsize::visit(const startStateProposition& f)
{
    (*item) += sizeof(f);
}

void PLTLsize::visit(const prv& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void PLTLsize::visit(const prvDisj& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void PLTLsize::visit(const prvConj& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void PLTLsize::visit(const prvNest& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void PLTLsize::visit(const snc& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
    accept_(f[1]);
}

void PLTLsize::visit(const pbx& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}

void PLTLsize::visit(const pdi& f)
{
    (*item) += sizeof(f);
    accept_(f[0]);
}
