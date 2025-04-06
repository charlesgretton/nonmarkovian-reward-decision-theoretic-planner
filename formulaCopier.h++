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

/*
 * \paragraph{:Purpose:}
 *
 * This module contains the implementation of a formula
 * copier. Formula copying is achieved by visitation. */
#ifndef FORMULA_COPIER
#define FORMULA_COPIER

#include"formulaVisitation.h++"
#include"formula.h++"

namespace Formula
{    
    template<class C>
    class PCcopier : 
        public Builder<formula>,
        public virtual PCvisitor,
        public C
    {
    public:
        void repass(const formula &c);

        void visit(const conj&);
        void visit(const disj&);
        void visit(const literal&);
        void visit(const assLiteral&);
        void visit(const iff&);
        void visit(const imp&);
        void visit(const lnot&);

        virtual formula* initBuild()
            {
                return 0;
            }
    };
    
    class FLTLcopier :
        public PCcopier<FLTLvisitor>
    {
    public:
        void visit(const nxt&);
        void visit(const fut&);
        void visit(const strFut&);
        void visit(const fbx&);
        void visit(const fdi&);
        void visit(const dollars&);
        
        void visit(const nxtDisj&);
        void visit(const nxtConj&);
        void visit(const nxtNest&);
        
    protected:
        void accept_(formula* f)const {FLTLvisitor::accept_(f);}
    }; 
    
    class PLTLcopier :
        public PCcopier<PLTLvisitor>
    {
    public:
        void visit(const startStateProposition&);
        void visit(const prv&);
        void visit(const snc&);
        void visit(const pbx&);
        void visit(const pdi&);

        void visit(const prvConj&);
        void visit(const prvDisj&);
        void visit(const prvNest&);
        
    protected:
        void accept_(formula* f)const {PLTLvisitor::accept_(f);}
    };    
}

#include"formulaCopier_templates.h++"

#endif
//  LocalWords:  PCcopier FLTLcopier PLTLcopier formulaCopier



