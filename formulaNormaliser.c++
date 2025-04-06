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

#include"formulaNormaliser.h++"

using namespace Formula;

/*FLTL*/

void FLTLnormaliser::visit(const nxt& f)
{
    accept_(f[0]);

    item = new nxt(item);

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const nxtDisj& f)
{
    bool tCarry = carry;
    accept_(f[0]);

    if(!tCarry)
        item = new nxtDisj(item, f.getDepth());
    else
        item = new nxtConj(item, f.getDepth());

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const nxtConj& f)
{
    bool tCarry = carry;
    accept_(f[0]);

    if(!tCarry)
        item = new nxtConj(item, f.getDepth());
    else
        item = new nxtDisj(item, f.getDepth());
    
    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const nxtNest& f)
{
    accept_(f[0]);

    item = new nxtNest(item, f.getDepth());

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const fut& f)
{
    if(!reporting && carry)
        throw InvalidFLTL();
    
    bool tCarry = carry;

    accept_(f[0]);
    formula *f1 = item;

    carry = tCarry;

    accept_(f[1]);
    formula *f2 = item;

    if(tCarry)
        {
            carry = tCarry;
            accept_(f[1]);
            formula *f3 = new conj(f1, item);

            item = new strFut(f2, f3);
        }
    else
        item = new fut(f1, f2);

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const strFut& f)
{
    if(!reporting && !carry)
        throw InvalidFLTL();
    
    bool tCarry = carry;

    accept_(f[0]);
    formula *f1 = item;

    carry = tCarry;

    accept_(f[1]);
    formula *f2 = item;


    if(tCarry)
        {
            carry = tCarry;
            accept_(f[1]);
            formula *f3 = new conj(f1, item);

            item = new fut(f2, f3);
        }
    else
        item = new strFut(f1, f2);

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const fbx& f)
{
    if(!reporting && carry)
        throw InvalidFLTL();
    
    bool tCarry = carry;

    accept_(f[0]);

    if(tCarry)
        item = new fdi(item);
    else
        item = new fbx(item);

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const fdi& f)
{
    if(!reporting && !carry)
        throw InvalidFLTL();
    
    bool tCarry = carry;

    accept_(f[0]);

    if(tCarry)
        item = new fbx(item);
    else
        item = new fdi(item);

    item->setNegNormal(true);/*Item is negation normal*/
}

void FLTLnormaliser::visit(const dollars& dol)
{
    if(!reporting && carry)
        throw InvalidFLTL();
    
    /*If we are carrying then that constitutes an error.*/
    if(!carry)
        item = new dollars();
    else
        item = new lnot(new dollars());
    
    item->setNegNormal(true);/*Item is negation normal*/
}

/*PLTL*/
void PLTLnormaliser::visit(const startStateProposition& f)
{
    if(carry)
        item = new lnot(new startStateProposition());
    else 
        item = new startStateProposition();
    
    carry = false;

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const prv& f)
{
    accept_(f[0]);

    item = new prv(item);

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const prvDisj& f)
{
    bool tCarry = carry;
    accept_(f[0]);

    if(!tCarry)
        item = new prvDisj(item, f.getDepth());
    else
        item = new prvConj(item, f.getDepth());

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const prvConj& f)
{
    bool tCarry = carry;
    accept_(f[0]);

    if(!tCarry)
        item = new prvConj(item, f.getDepth());
    else
        item = new prvDisj(item, f.getDepth());

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const prvNest& f)
{
    accept_(f[0]);

    item = new prvNest(item, f.getDepth());

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const snc& f)
{
    bool tCarry = carry;

    accept_(f[0]);
    formula *f1 = item;
    
    carry = tCarry;
    
    accept_(f[1]);
    formula *f2 = item;

    if(!tCarry)
        {
            item = new snc(f1, f2);
        }
    else
        item = new disj(new pbx(f2)
                        , new snc(f2->copy(), new conj(f1, f2->copy()) ));

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const pbx& f)
{
    bool tCarry = carry;

    accept_(f[0]);

    if(tCarry)
        item = new pdi(item);
    else
        item = new pbx(item);

    item->setNegNormal(true);/*Item is negation normal*/
}

void PLTLnormaliser::visit(const pdi& f)
{    
    bool tCarry = carry;

    accept_(f[0]);

    if(tCarry)
        item = new pbx(item);
    else
        item = new pdi(item);

    item->setNegNormal(true);/*Item is negation normal*/
}
