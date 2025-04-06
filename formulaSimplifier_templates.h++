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

#ifndef FORMULA_SIMPLIFIER_template
#define FORMULA_SIMPLIFIER_template

#include"formulaNormaliser.h++"

namespace Formula
{ 
    /* export */ template<class C>
    void PCsimplifier<C>::repass(const formula &c)
    {
        if(needRepass())
	    {
            item = c.simplify();
	    }

        PCvisitor::repass(c);
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const literal& l)
    {
        item =  new literal(l);
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const assLiteral& al)
    {
        item =  new assLiteral(al);
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const conj& f)
    {
        accept_(f[0]);
        formula *fL = item;
        accept_(f[1]);
        formula *fR = item;

        if(assLiteral(false) == *fL || assLiteral(false) == *fR)/*false and a == false, also a and false = false*/
	    {
            item = new assLiteral(false);
            delete fL;
            delete fR;
	    }
        else if(assLiteral(true) == *fL)/*true and a == a*/
	    {
            item = fR;
            delete fL;
	    }
        else if(assLiteral(true) == *fR)/*true and a == a*/
        {
            item = fL;
            delete fR;
        }
//          else if(lnot(fL->copy()) == *fR || lnot(fR->copy()) == *fL)/*a and ~a == false, also ~a and a = false*/
//  	    {
//              item = new assLiteral(false);
//              delete fL;
//              delete fR;
//  	    }
        else if((*fL) == *fR)/*a and a == a*/
	    {
            item = fL;
            delete fR;
	    }
        /* a and (a and b) == a and b. This is a key simplification rule
           for progression.*/
        else if(0 != (item = dynamic_cast<conj*>(fR)) &&
                (*fL) == *(*dynamic_cast<binary*>(fR))[0]) 
        {
            formula* tmp =
                new conj(fL, (*dynamic_cast<binary*>(fR))[1]->copy());
            item = tmp->simplify();
            delete tmp;
            delete fR;
        }
        /* a and (b and a) == a and b. This is a key simplification rule
           for progression.*/
        else if(0 != (item = dynamic_cast<conj*>(fR)) &&
                (*fL) == *(*dynamic_cast<binary*>(fR))[1])
        {
            formula* tmp =
                new conj(fL, (*dynamic_cast<binary*>(fR))[0]->copy());
            item = tmp->simplify();
            delete tmp;
            delete fR;
        }
        /* a and (a or b) == a AND a and (b or a) == a.*/
        else if(0 != (item = dynamic_cast<disj*>(fR)) &&
                ( (*fL) == *(*dynamic_cast<binary*>(fR))[0] ||
                  (*fL) == *(*dynamic_cast<binary*>(fR))[1]) )
        {
            item = fL;
            delete fR;
        }
        else
            item = new conj(fL, fR);/*No simplification.*/
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const disj& f)
    {
        accept_(f[0]);
        formula *fL = item;
        accept_(f[1]);
        formula *fR = item;

        if(assLiteral(true) == *fL || assLiteral(true) == *fR)/*true or a == true, also a or true = true*/
	    {
            item = new assLiteral(true);
            delete fL;
            delete fR;
	    }
        else if(assLiteral(false) == *fL)/*false or a == a, also a or false = a*/
	    {
            item = fR;
            delete fL;
	    }
        else if(assLiteral(false) == *fR)/*false or a == a, also a or false = a*/
        {
            item = fL;
            delete fR;
	    }
//          else if(lnot(fL->copy()) == *fR || lnot(fR->copy()) == *fL)/*a or ~a == true, also ~a or a = true*/
//  	    {
//              item = new assLiteral(true);
//              delete fL;
//              delete fR;
//  	    }
        else if((*fL) == *fR)/*a or a == a*/
	    {
            item = fL;
            delete fR;
	    }
    
        /* a or (a or b) == a or b. This is a key simplification rule
           for progression.*/
        else if(0 != (item = dynamic_cast<disj*>(fR)) &&
                (*fL) == *(*dynamic_cast<binary*>(fR))[0]) 
        {
            formula* tmp =
                new disj(fL, (*dynamic_cast<binary*>(fR))[1]->copy());
            item = tmp->simplify();
            delete tmp;
            delete fR;
        }
        /* a or (b or a) == a or b. This is a key simplification rule
           for progression.*/
        else if(0 != (item = dynamic_cast<disj*>(fR)) &&
                (*fL) == *(*dynamic_cast<binary*>(fR))[1])
        {
            formula* tmp =
                new disj(fL, (*dynamic_cast<binary*>(fR))[0]->copy());
            item = tmp->simplify();
            delete tmp;
            delete fR;
        }
        /* a or (a and b) == a AND a or (b and a) == a.*/
        else if(0 != (item = dynamic_cast<conj*>(fR)) &&
                ( (*fL) == *(*dynamic_cast<binary*>(fR))[0] ||
                  (*fL) == *(*dynamic_cast<binary*>(fR))[1]) )
        {
            item = fL;
            delete fR;
        }
        else
            item = new disj(fL, fR);/*No simplification.*/
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const iff& f)
    {
        /*Obtain negation normal version of $f$.*/
        formula *nn = f.negNormalise();
    
        accept_(nn);

        /*If $f$ does not simplify to a truth value*/
        if(!dynamic_cast<assLiteral*>(item))
	    {	
            delete item;
            accept_(f[0]);
            formula *tmp = item;
            accept_(f[1]);
            item = new iff(tmp, item);    
	    }

        delete nn;
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const imp& f)/*~(p->q) = (p and ~q)*/
    {
        /*Obtain negation normal version of $f$.*/
        formula *nn = f.negNormalise();
    
        accept_(nn);
    
        /*If $f$ does not simplify to a truth value*/
        if(!dynamic_cast<assLiteral*>(item))
	    {
            delete item;
            accept_(f[0]);
            formula *tmp = item;
            accept_(f[1]);
            item = new imp(tmp, item);
	    }

        delete nn;
    }

    /* export */ template<class C>
    void PCsimplifier<C>::visit(const lnot& f)
    {
        accept_(f[0]);

        /*If the subformula is not an assigned literal*/
        if(!dynamic_cast<assLiteral*>(item))
            item = new lnot(item);       
        else
	    {
            item = lnot(item).negNormalise();
	    }
    }

    /* export */ template<class C>
    formula* PCsimplifier<C>::initBuild()
    {
        return 0;
    }
}

#endif
