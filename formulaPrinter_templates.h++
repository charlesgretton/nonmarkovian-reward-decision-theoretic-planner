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

#ifndef FORMULA_PRINTER_template
#define FORMULA_PRINTER_template

namespace Formula
{ 
    template<class C>
    void PCprinter<C>::repass(const formula &c)
    {
	if(needRepass())
	    {
	        string *tmp = c.print();
		item->append(*tmp);
		delete tmp;
	    }
    
	PCvisitor::repass(c);
    }

    template<class C>
    void PCprinter<C>::visit(const literal& l)
    {item->append("(");
    item->append(l.getId());
    item->append(")");}

    template<class C>
    void PCprinter<C>::visit(const assLiteral& al)
    {item->append("(");
    if(al.getAssignment()) 
	item->append("tt");
    else
	item->append("ff");
    item->append(")");}

    template<class C>
    void PCprinter<C>::visit(const conj& f)
    {item->append("(");
    accept_(f[0]);
    item->append(" and ");
    accept_(f[1]);
    item->append(")");}

    template<class C>
    void PCprinter<C>::visit(const disj& f)
    {item->append("(");
    accept_(f[0]);
    item->append(" or ");
    accept_(f[1]);
    item->append(")");}

    template<class C>
    void PCprinter<C>::visit(const iff& f)
    {item->append("(");
    accept_(f[0]);
    item->append("<->");
    accept_(f[1]);
    item->append(")");}

    template<class C>
    void PCprinter<C>::visit(const imp& f)
    {item->append("(");
    accept_(f[0]);
    item->append("->");
    accept_(f[1]);
    item->append(")");}

    template<class C>
    void PCprinter<C>::visit(const lnot& f)
    {item->append("(");
    item->append("~");
    accept_(f[0]);
    item->append(")");}

    template<class C>
    string* PCprinter<C>::initBuild()
    {
	delete item;
	item = new string();
	return item;
    }
}
#endif
