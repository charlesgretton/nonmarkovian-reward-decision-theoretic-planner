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

#ifndef FORMULA_COPIER_template
#define FORMULA_COPIER_template

namespace Formula
{
    /*export*/ template<class C>
    void PCcopier<C>::repass(const formula &c)
    {
        if(needRepass())
	    {
            item = c.copy();
	    }

        PCvisitor::repass(c);
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const literal& l)
    {
        item = new literal(l);
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const assLiteral& al)
    {
        item = new assLiteral(al.getAssignment());
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const conj& f)
    {
        formula* tmp;
        accept_(f[0]);
        tmp = item;
        accept_(f[1]);
        item = new conj(tmp, item);
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const disj& f)
    {
        formula* tmp;
        accept_(f[0]);
        tmp = item;
        accept_(f[1]);
        item = new disj(tmp, item);
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const iff& f)
    {
        formula* tmp;
        accept_(f[0]);
        tmp = item;
        accept_(f[1]);
        item = new iff(tmp, item);
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const imp& f)/*~(p->q) = (p and ~q)*/
    {
        formula* tmp;
        accept_(f[0]);
        tmp = item;
        accept_(f[1]);
        item = new imp(tmp, item);
    }

    /*export*/ template<class C>
    void PCcopier<C>::visit(const lnot& f)
    {
        accept_(f[0]);
        item = new lnot(item);
    }
}

#endif
