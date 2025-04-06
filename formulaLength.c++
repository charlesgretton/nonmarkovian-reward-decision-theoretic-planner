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

#include"formulaLength.h++"
#include"formula.h++"

using namespace std;

using namespace Formula;


void FLTLlength::visit(const nxt& f)
{
    ++(*item);
    accept_(f[0]);
}

void FLTLlength::visit(const nxtDisj& f)
{
    ++(*item);
    accept_(f[0]);
}

void FLTLlength::visit(const nxtConj& f)
{
    ++(*item);
    accept_(f[0]);
}

void FLTLlength::visit(const nxtNest& f)
{
    ++(*item);
    accept_(f[0]);
}

void FLTLlength::visit(const fut& f)
{
    ++(*item);
    accept_(f[0]);
    accept_(f[1]);
}

void FLTLlength::visit(const strFut& f)
{
    ++(*item);
    accept_(f[0]);
    accept_(f[1]);
}

void FLTLlength::visit(const fbx& f)
{
    ++(*item);
    accept_(f[0]);
}

void FLTLlength::visit(const fdi& f)
{
    ++(*item);
    accept_(f[0]);
}

void FLTLlength::visit(const dollars& dol)
{
    ++(*item);
}

void PLTLlength::visit(const startStateProposition& f)
{
    ++(*item);
} 

void PLTLlength::visit(const prv& f)
{
    ++(*item);
    accept_(f[0]);
}

void PLTLlength::visit(const prvDisj& f)
{
    ++(*item);
    accept_(f[0]);
}

void PLTLlength::visit(const prvConj& f)
{
    ++(*item);
    accept_(f[0]);
}

void PLTLlength::visit(const prvNest& f)
{
    ++(*item);
    accept_(f[0]);
}

void PLTLlength::visit(const snc& f)
{
    ++(*item);
    accept_(f[0]);
    accept_(f[1]);
}

void PLTLlength::visit(const pbx& f)
{
    ++(*item);
    accept_(f[0]);
}

void PLTLlength::visit(const pdi& f)
{
    ++(*item);
    accept_(f[0]);
}
