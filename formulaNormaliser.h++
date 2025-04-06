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
 * This module contains the implementation of formula negation
 * normalisation. Formula normalisation is achieved by visitation.
 *
 * The following rules are used for normalisation where $a$ and $b$
 * are wff and $a'$ and $b'$ their negation normal counterparts (the
 * relation "$\Rightarrow$" is read "normalised is"):
 *
 * \begin{eqnarray}\label{eq:negNorm}
 * a \Rightarrow a'\\
 * \lnot \lnot a \Rightarrow a'\\
 * \lnot (a \land b) \Rightarrow \lnot a' \lor \lnot b'\\
 * \lnot (a \lor b) \Rightarrow \lnot a' \land \lnot b'\\
 * \lnot (a \to b) \Rightarrow p' \land \lnot q'\\
 * \lnot (a iff b) \Rightarrow ( \lnot ( (a \to b) \land (b \to a) ) )'\\
 * \lnot \nxt b \Rightarrow \nxt \lnot b'\\
 * \lnot (a \fut b) \Rightarrow \lnot b' \strFut (\lnot a' \land \lnot b')\\
 * \lnot (a \strFut b) \Rightarrow  \lnot b' \fut (\lnot a' \land \lnot b')\\
 * \lnot \fdi b \Rightarrow \fbx \lnot b'\\
 * \lnot \fbx b \Rightarrow \fdi \lnot b'\\
 * \lnot  tt \Rightarrow ff\\
 * \lnot  ff \Rightarrow tt\\
 * literal \Rightarrow literal\\
 * \lnot \prv b \Rightarrow \prv \lnot b'\\
 * \lnot (a \snc b) \Rightarrow \pbx \lnot b' \lor \lnot b' \snc (\lnot a \land \lnot b)\\
 * \lnot \pdi b \Rightarrow \pbx \lnot b'\\
 * \lnot \pbx b \Rightarrow \pdi \lnot b'
 * \end{eqnarray}
 *
 **/
#ifndef FORMULA_NORMALISER
#define FORMULA_NORMALISER

#include"formulaVisitation.h++"
#include"formula.h++"

namespace Formula
{   
    /*Normalisation of predicate calculus formulae is achieved by
     *\class{PCnormaliser} visitation. If an argument formula contains
     *non-PC components these shall be normalised according to
     *derivative rules (see \class{FLTLnormaliser} and
     *\class{PLTLnormaliser}).
     *
     *\textbf{NOTE:} This is an example of an ignorant visitor. This
     *type of visitor is the base for larger unrelated visitors but
     *must also be able to perform traversal of a composite (formula)
     *on its own.*/
    template<class C>
    class PCnormaliser : 
        public Builder<formula>,
        public virtual PCvisitor,
        public C
    {
    public:
        /*A normaliser on construction is assumed not to be carrying a
         *negation and not to be for the purpose of reporting.*/
        PCnormaliser(bool carry = false):reporting(false),next(carry){}

        void repass(const formula &c);

        virtual void visit(const conj&);
        virtual void visit(const disj&);
        virtual void visit(const literal&);
        virtual void visit(const assLiteral&);
        virtual void visit(const iff&);
        virtual void visit(const imp&);
        virtual void visit(const lnot&);

        virtual formula* initBuild()
            {
                carry = next;

                if(next)
                    next = false;

                return 0;
            }

        /*Normalisation is configured to be for the purposes of reporting.*/
        void isReporting();
    protected:
        /*Is the normaliser carrying a negation?*/
        bool carry;

        /*Is the normalisation for the purpose of reporting (\member{true})
         *or is it for the purpose of future use in a planner?*/
        bool reporting;
    private:
        /*Is this normaliser the result repass at construction?*/
        bool next;
    };    

    class FLTLnormaliser :
        public PCnormaliser<FLTLvisitor>
    {
    public:
        FLTLnormaliser(bool carry):PCnormaliser<FLTLvisitor>(carry){}

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
    
    class PLTLnormaliser :
        public PCnormaliser<PLTLvisitor>
    {
    public:
        PLTLnormaliser(bool carry):PCnormaliser<PLTLvisitor>(carry){}

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
    
    /*Where an \fltl{} \class{DomainSpecification} that contains
     *reward formulae which when translated into their reward normal
     *form contain eventualities or negated $\$$, the specification is
     *invalid and the following exception shall be thrown.*/
    class InvalidFLTL
    {
    public:
        /*Non--Exceptional exception construction.*/
        InvalidFLTL(){}
    };
}

#include"formulaNormaliser_templates.h++"

#endif
