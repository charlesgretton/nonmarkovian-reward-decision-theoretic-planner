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

#ifndef FORMULA_NORMALISER_template
#define FORMULA_NORMALISER_template

namespace Formula
{
    /* export */ template<class C>
    void PCnormaliser<C>::isReporting()
    {
        reporting = true;
    }
    
    /* export */ template<class C>
    void PCnormaliser<C>::repass(const formula &c)
    {
        if(needRepass())
	    {
            item = c.negNormalise(carry);
	    }

        PCvisitor::repass(c);
    }

    /* export */ template<class C>
    void PCnormaliser<C>::visit(const literal& l)
    {
        if(carry)
            item = new lnot(new literal(l));
        else 
            item = new literal(l);

        carry = false;

        item->setNegNormal(true);/*Item is negation normal*/
    }

    /* export */ template<class C>
    void PCnormaliser<C>::visit(const assLiteral& al)
    {
        if(carry)
            item = new assLiteral(!al.getAssignment());
        else
            item = new assLiteral(al.getAssignment());

        carry = false;

        item->setNegNormal(true);/*Item is negation normal*/
    }

    /* export */ template<class C>
    void PCnormaliser<C>::visit(const conj& f)
    {
        bool tCarry = carry;
        formula *tmp;

        accept_(f[0]);
        tmp = item;
        carry = tCarry;
        accept_(f[1]);

        if(tCarry)
            item = new disj(tmp, item);
        else
            item = new conj(tmp, item);

        item->setNegNormal(true);/*Item is negation normal*/
    }


    /* export */ template<class C>
    void PCnormaliser<C>::visit(const disj& f)
    {
        bool tCarry = carry;
        formula *tmp;

        accept_(f[0]);
        tmp = item;
        carry = tCarry;
        accept_(f[1]);

        if(tCarry)
            item = new conj(tmp, item);
        else
            item = new disj(tmp, item);

        item->setNegNormal(true);/*Item is negation normal*/
    }

    /* export */ template<class C>
    void PCnormaliser<C>::visit(const iff& f)
    {
        bool tCarry = carry;

        carry = false;

        accept_(f[0]);
        formula *f1 = item;

        accept_(f[1]);
        formula *f2 = item;

        accept_(f[0]);
        formula *f3 = item;

        accept_(f[1]);
        formula *f4 = item;

        carry = tCarry;

        formula *newF = new conj(new imp(f1, f2), new imp(f4, f3));
  
        newF->accept(this);
        delete newF;

        item->setNegNormal(true);/*Item is negation normal*/
    }

    /* export */ template<class C>
    void PCnormaliser<C>::visit(const imp& f)/*~(p->q) = (p and ~q)*/
    {
        bool tCarry = carry;

        carry = false;

        accept_(f[0]);
        formula *f1 = new lnot(item);

        accept_(f[1]);
        formula *f2 = item;

        carry = tCarry;

        formula *newF = new disj(f1, f2);
        newF->accept(this);
        delete newF;

        item->setNegNormal(true);/*Item is negation normal*/
    }

    /* export */ template<class C>
    void PCnormaliser<C>::visit(const lnot& f)
    {
        carry = !carry;
        accept_(f[0]);

        item->setNegNormal(true);/*Item is negation normal*/
    }
}
#endif
