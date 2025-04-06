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

#ifndef FORMULA_SEQUENCE_TRAVERSAL_template
#define FORMULA_SEQUENCE_TRAVERSAL_template

#include"formulaNormaliser.h++"
#include"formulaPrinter.h++"
#include"EntailmentFilter.h++"

namespace Formula
{
    /*FLTL*/

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const nxt& f)
    {
        evaluate(&f);   
        item = f[0]->copy();
        
        item = (*buildFilter)(item);
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const nxtDisj& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());

        if(f.getDepth() > 2)
            item = (*buildFilter)(new disj(item,
                                           new nxtDisj(f[0]->copy(),
                                                       f.getDepth() - 1)));
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const nxtConj& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());

        if(f.getDepth() > 2)
            item = (*buildFilter)(new conj(item,
                                           new nxtConj(f[0]->copy(),
                                                       f.getDepth() - 1)));
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const nxtNest& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());
        
        if(f.getDepth() > 1)
            item = (*buildFilter)(new nxtNest(f[0]->copy(),
                                              f.getDepth() - 1));
    }


    /* export */ template<class C>
    void FLTLprogression<C>::visit(const fut& f)
    {
        evaluate(&f);
        accept_(f[1]);
        formula *tmp = item;
        
        /*Simplification could be expensive.*/
        item = item->simplify();
        delete tmp;
        
        /*Is this reward element shall progress to true.*/
        if(assLiteral(true) == *item)
            return;
        tmp = item;
        
        accept_(f[0]);
        /*One is perhaps tempted to now apply the following line of
          code. But we are in the process of visitation and double
          deletion would occur of the visitor if we were to do so...

          item = disj(tmp, new conj(item, f.copy())).simplify();

          The solution is to make use of the copy constructors
          provided by the binary and unary formula bases as follows.
        */
        item = new disj(tmp, new conj(item, (*buildFilter)( new fut(f) )));
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const strFut& f)
    {
        evaluate(&f);   
        cerr<<"Error :: Progression of eventuality attempted.\n";
        assert(0);
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const fbx& f)
    {
        evaluate(&f);
    
        formula *tmp = f[0];
        accept_(tmp = new fut(tmp->copy(), new assLiteral(false)));
        delete tmp;
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const fdi& f)
    {
        evaluate(&f);
        cerr<<"Error :: Progression of eventuality attempted.\n";
        assert(0);
    }

    /* export */ template<class C>
    void FLTLprogression<C>::visit(const dollars& dol)
    {
        control = false;
        evaluate(&dol);
        if(false == reward)
            item = new assLiteral(false);
        else if(true == reward)
            item = new assLiteral(true);
    }

    /* export */ template<class C>
    bool FLTLprogression<C>::rewarding()
    {
        /*Simplify the result of progression.*/
        formula *tmp = item;
        item = item->simplify();
        delete tmp;

        /*Make a repass if this has yet to be done.*/
        if(false == reward)
            /*Has progression yielded $false$?*/
            if(assLiteral(false) == (*item))
		    {
                /*Reward $dollars$ on the repass.*/
                PCsequence_traversal<FLTLvisitor, C>::rewarding(true);
			
                /*We are going to make a repass so the last build
                  can be freed.*/
                delete item;
			
                /*Make the repass.*/
                accept_(visitationCache);

                /*Return how rewarding the repass was.*/
                return rewarding();
		    }

        /*Was progression rewarding?*/
        return (reward && assLiteral(false) != *item);
    }

    /*PLTL*/
    /* export */ template<class C>
    void PLTLregression<C>::visit(const startStateProposition& f)
    {
        if(0 != dynamic_cast<MDP::ZeroPredecessor const*>(buildFilter))
            item = f.startStateAssignment();
        else
        {
            formula* tmp = new lnot(f.startStateAssignment());
            item = tmp->simplify();
            delete tmp;
        }
    }
    
    /* export */ template<class C>
    void PLTLregression<C>::visit(const prv& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());
    }

    /* export */ template<class C>
    void PLTLregression<C>::visit(const prvDisj& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());

        if(f.getDepth() > 2)
            item = new disj(item,
                            (*buildFilter)( new prvDisj(f[0]->copy(),
                                                        f.getDepth() - 1)) );
    }

    /* export */ template<class C>
    void PLTLregression<C>::visit(const prvConj& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());

        if(f.getDepth() > 2)
            item = new conj(item,
                            (*buildFilter)( new prvConj(f[0]->copy(),
                                                        f.getDepth() - 1)) );
    }

    /* export */ template<class C>
    void PLTLregression<C>::visit(const prvNest& f)
    {
        evaluate(&f);
        item = (*buildFilter)(f[0]->copy());
        
        if(f.getDepth() > 1)
            item = (*buildFilter)(new prvNest(f[0]->copy(),
                                              f.getDepth() - 1));
    }

    
    /* export */ template<class C>
    void PLTLregression<C>::visit(const snc& f)
    {
        evaluate(&f);
        accept_(f[1]);
        formula *tmp = item;
        
        /*Simplification could be expensive.*/
        item = item->simplify();
        delete tmp;
        
        /*Did this reward element progress to $true$?*/
        if(assLiteral(true) == *item)
            return;
        tmp = item;
        
        
        accept_(f[0]);

        item = new disj(tmp,
                        new conj(item, (*buildFilter)( new snc(f) )));    
    }

    /* export */ template<class C>
    void PLTLregression<C>::visit(const pbx& f)
    {
        evaluate(&f);    
        accept_(f[0]);

        item = new conj(item, (*buildFilter)( new pbx(f) ));
    }

    /* export */ template<class C>
    void PLTLregression<C>::visit(const pdi& f)
    {
        evaluate(&f);
        accept_(f[0]);

        item = new disj(item, (*buildFilter)( new pdi(f)) );
    }

    /* export */ template<class C>
    bool PLTLregression<C>::rewarding()
    {     
        /*Simplify formula before checking for rewardability.*/
        formula *tmp = item;   
        item = item->simplify(); 
        
        /*Filter the resulting item, and inform the filter that this
         *is to be the result of visitation. Only apply the filter in
         *the case that this is a non--assigned literal.*/
        if(0 == dynamic_cast<assLiteral*>(item))
            item = (*buildFilter)(item, true);
        
        delete tmp;
        
        return assLiteral(true) == *item;
    }

    /*PC*/

    /* export */ template<class C, typename PropositionContainer>
    void PCsequence_traversal<C, PropositionContainer>::visit(const literal& l)
    {
        evaluate(&l);
        set<string>::const_iterator tmp = propositions.find(l.getId());
  
        if(propositions.end() != tmp)
            item = new assLiteral(true);
        else
            item = new assLiteral(false);
    }

    /* export */ template<class C, typename D>
    void PCsequence_traversal<C, D>::visit(const assLiteral& al)
    {
        evaluate(&al);
        item = new assLiteral(al);
    }

    /* export */ template<class C, typename D>
    void PCsequence_traversal<C, D>::visit(const conj& f)
    {
        evaluate(&f);

        accept_(f[0]);
        formula *tmp = item;
        accept_(f[1]);
        item = new conj(tmp, item);
    }

    /* export */ template<class C, typename D>
    void PCsequence_traversal<C, D>::visit(const disj& f)
    {
        evaluate(&f);
        accept_(f[0]);
        formula *tmp = item;
        accept_(f[1]);
        item = new disj(tmp, item);
    }

    /* export */ template<class C, typename D>
    void PCsequence_traversal<C, D>::visit(const iff& f)
    {
        evaluate(&f);
        formula *tmp = f.negNormalise();
        tmp->accept(this);
        delete tmp;
    }

    /* export */ template<class C, typename D>
    void PCsequence_traversal<C, D>::visit(const imp& f)
    {
        evaluate(&f);
        formula *tmp = f.negNormalise();
        tmp->accept(this);
        delete tmp;
    }

    /* export */ template<class C, typename D>
    void PCsequence_traversal<C, D>::visit(const lnot& f)
    {   
        evaluate(&f);
        accept_(f[0]);
        item = new lnot(item);//.simplify();
    }

    /* export */ template<class C, typename D>
    formula* PCsequence_traversal<C, D>::initBuild()
    {
        visitationCache = 0;
        reward = false;
        return 0;
    }
}
#endif

