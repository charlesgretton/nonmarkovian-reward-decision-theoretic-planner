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

#ifndef FORMULA_LENGTH_template
#define FORMULA_LENGTH_template

namespace Formula
{ 
    template<class C>
    void PClength<C>::repass(const formula &c)
    {
        if(needRepass())
	    {
	        unsigned int tmp = c.length();
            (*item) += tmp;
	    }
        
        PCvisitor::repass(c);
    }

    template<class C>
    void PClength<C>::visit(const literal& l)
    {
        ++(*item);
    }

    template<class C>
    void PClength<C>::visit(const assLiteral& al)
    {
        ++(*item);
    }

    template<class C>
    void PClength<C>::visit(const conj& f)
    {
        ++(*item);
        accept_(f[0]);
        accept_(f[1]);
    }

    template<class C>
    void PClength<C>::visit(const disj& f)
    {
        ++(*item);
        accept_(f[0]);
        accept_(f[1]);
    }

    template<class C>
    void PClength<C>::visit(const iff& f)
    {
        ++(*item);
        accept_(f[0]);
        accept_(f[1]);
    }

    template<class C>
    void PClength<C>::visit(const imp& f)
    {
        ++(*item);
        accept_(f[0]);
        accept_(f[1]);
    }

    template<class C>
    void PClength<C>::visit(const lnot& f)
    {
        ++(*item);
        accept_(f[0]);
    }

    template<class C>
    unsigned int* PClength<C>::initBuild()
    {
        delete item;
        item = new unsigned int(0);
        return item;
    }
}
#endif
