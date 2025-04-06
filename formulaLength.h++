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
 * This module contains the implementation of a formula length
 * measurement. Formula length measurement is achieved by visitation
 * over connectives (see \module{formula}). The length measured is
 * given by the number of connectives which the argument formula
 * comprised.*/
#ifndef FORMULA_LENGTH
#define FORMULA_LENGTH

#include"formulaVisitation.h++"

using namespace std;

namespace Formula
{
    template<class C>
    class PClength :   
        public Builder<unsigned int>,
        public virtual PCvisitor,
        public C
    {
    public:
        PClength(){item = new unsigned int(0);}
        ~PClength() {}
	
        void repass(const formula &c);

        virtual void visit(const conj&);
        virtual void visit(const disj&);
        virtual void visit(const literal&);
        virtual void visit(const assLiteral&);
        virtual void visit(const iff&);
        virtual void visit(const imp&);
        virtual void visit(const lnot&);

        virtual unsigned int* initBuild();
    };

    class FLTLlength :
        public PClength<FLTLvisitor>
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

    class PLTLlength : 
        public PClength<PLTLvisitor>
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

#include"formulaLength_templates.h++"

#endif
