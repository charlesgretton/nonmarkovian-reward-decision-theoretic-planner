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
 * This module contains the implementation of a formula simplifier
 * (boolean simplification only). Formula simplification is achieved
 * by visitation.
 *
 * This visitor is designed for use with both progression and
 * regression of FLTL and PLTL formula respectively. The boolean
 *
 * simplifications which it supports include:
 *
 * \begin{eqnarray}
 * a \land (a \land b) \equiv a \land b \\
 * a \land (b \land a) \equiv a \land b \\
 * a \lor (a \land b)  \equiv a \\
 * a \lor (b \land a)  \equiv a \\
 * b \lor b            \equiv b \\
 * b \land b           \equiv b \\
 * \sim \sim b         \equiv b  \\
 * \bot \land a        \equiv \bot \\
 * \top \land a        \equiv a \\
 * \top \lor a         \equiv \top \\
 * \bot \lor a         \equiv a \\
 * \sim \top           \equiv \bot \\
 * \sim \bot           \equiv \top \\
 * \end{eqnarray}
 *
 */
#ifndef FORMULA_SIMPLIFIER
#define FORMULA_SIMPLIFIER

#include"formulaVisitation.h++"
#include"formula.h++"

namespace Formula
{    
    template<class C>
    class PCsimplifier : 
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

        virtual formula* initBuild();
    };
    
    class FLTLsimplifier :
        public PCsimplifier<FLTLvisitor>
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
        void accept_(formula const* f)const {FLTLvisitor::accept_(f);}
    }; 
    
    class PLTLsimplifier :
        public PCsimplifier<PLTLvisitor>
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
        void accept_(formula const* f)const {PLTLvisitor::accept_(f);}
    };    
}

#include"formulaSimplifier_templates.h++"

#endif
