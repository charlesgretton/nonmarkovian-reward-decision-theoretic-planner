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

#ifndef FORMULA_NORMALISER
#define FORMULA_NORMALISER

#include"formulaVisitation.h++"
#include"formula.h++"

namespace Formula
{    
    class PCprogressor : 
        public Builder<formula*>,
        public virtual PCvisitor
    {
    public:
        void visit(conj&);
        void visit(disj&);
        void visit(literal&);
        void visit(assLiteral&);
        void visit(iff&);
        void visit(imp&);
        void visit(not&);
        virtual formula* getBuild();
        virtual formula* initBuild();
      protected:
        bool behaviour;
    };
    
    class FLTLprogressor :
        public PCprogressor, 
        public virtual FLTLvisitor
    {
    public:
        void visit(conj& f){PCprogressor::visit(f);}
        void visit(disj& f){PCprogressor::visit(f);}
        void visit(literal& f){PCprogressor::visit(f);}
        void visit(assLiteral& f){PCprogressor::visit(f);}
        void visit(iff& f){PCprogressor::visit(f);}
        void visit(imp& f){PCprogressor::visit(f);}
        void visit(not& f){PCprogressor::visit(f);}

        void visit(nxt&);
        void visit(fut&);
        void visit(strFut&);
        void visit(fbx&);
        void visit(fdi&);
        void visit(dollars&);
    protected:
        void accept_(formula* f){FLTLvisitor::accept_(f);}
    }; 
}
#endif
