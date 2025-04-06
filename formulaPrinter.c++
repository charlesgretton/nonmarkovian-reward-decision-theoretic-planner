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

#include"formulaPrinter.h++"
#include"formula.h++"

using namespace std;

using namespace Formula;


void FLTLprinter::visit(const nxt& f)
{item->append("(");
    item->append(" nxt ");
    accept_(f[0]);
item->append(")");}

void FLTLprinter::visit(const nxtDisj& f)
{item->append("(");
    ostringstream numeric;
    numeric<<f.getDepth();
    item->append(" nxt or < "+numeric.str());
    accept_(f[0]);
item->append(")");}

void FLTLprinter::visit(const nxtConj& f)
{item->append("(");
    ostringstream numeric;
    numeric<<f.getDepth();
    item->append(" nxt and < "+numeric.str());
    accept_(f[0]);
item->append(")");}

void FLTLprinter::visit(const nxtNest& f)
{item->append("(");
    ostringstream numeric;
    numeric<<f.getDepth();
    item->append(" nxt ^ "+numeric.str());
    accept_(f[0]);
item->append(")");}

void FLTLprinter::visit(const fut& f)
{item->append("(");
    accept_(f[0]);
    item->append(" fut ");
    accept_(f[1]);
item->append(")");}

void FLTLprinter::visit(const strFut& f)
{item->append("(");
    accept_(f[0]);
    item->append(" strFut ");
    accept_(f[1]);
item->append(")");}

void FLTLprinter::visit(const fbx& f)
{item->append("(");
    item->append(" fbx ");
    accept_(f[0]);
item->append(")");}

void FLTLprinter::visit(const fdi& f)
{item->append("(");
    item->append(" fdi ");
    accept_(f[0]);
item->append(")");}

void FLTLprinter::visit(const dollars& dol)
{item->append("(");
    item->append("$");
item->append(")");}

void PLTLprinter::visit(const startStateProposition&)
{item->append("(");
    item->append(" START_STATE ");
item->append(")");}

void PLTLprinter::visit(const prv& f)
{item->append("(");
    item->append(" prv ");
    accept_(f[0]);
item->append(")");}

void PLTLprinter::visit(const prvDisj& f)
{item->append("(");
    ostringstream numeric;
    numeric<<f.getDepth();
    item->append(" prv or < "+numeric.str());
    accept_(f[0]);
item->append(")");}

void PLTLprinter::visit(const prvConj& f)
{item->append("(");
    ostringstream numeric;
    numeric<<f.getDepth();
    item->append(" prv and < "+numeric.str());
    accept_(f[0]);
item->append(")");}

void PLTLprinter::visit(const prvNest& f)
{item->append("(");
    ostringstream numeric;
    numeric<<f.getDepth();
    item->append(" prv ^ "+numeric.str());
    accept_(f[0]);
item->append(")");}

void PLTLprinter::visit(const snc& f)
{item->append("(");
    accept_(f[0]);
    item->append(" snc ");
    accept_(f[1]);
item->append(")");}

void PLTLprinter::visit(const pbx& f)
{item->append("(");
    item->append(" pbx ");
    accept_(f[0]);
item->append(")");}

void PLTLprinter::visit(const pdi& f)
{item->append("(");
    item->append(" pdi ");
    accept_(f[0]);
item->append(")");}
