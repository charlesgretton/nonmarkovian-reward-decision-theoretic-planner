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
 * Data structures that encapsulate logical formulae.
 *
 * For implementations of non--traversal class functions and methods
 * relating to formula:
 * 
 * see \module{formula}
 * 
 * For the implementation of formula equality:
 * 
 * see \module{formulaEquality}
 * */
#ifndef FORMULA
#define FORMULA

#include"formulaVisitation.h++"
#include<string>
#include<cassert>

using namespace std;

namespace Formula
{ 
    /*Parent class for formulae and their subcomponents
     *(subformulae).*/
    class formula
    {
    public:
        /*A formula on construction is assumed not to be negation
         *normal.*/
        formula():negNormal(false), v(0){}

        formula(const formula& f):negNormal(f.negNormal), v(0){}

        formula(bool negNormal):negNormal(negNormal){}
        
        /*Ensure derivation cleaning.*/
        virtual ~formula() {} 

        /*Generic visitation for composite formula tree traversal is
         *publicly implemented only within the base class of all
         *composites.*/
        template<typename Visit>
        void accept(Visit v) const
            {
                this->v = new Visitor;
                this->v->setVisitor(v);
                accept_();
                delete this->v;
            }
        
        /*\class{formula} equality is based on syntactic equality
         *between this and the \argument{formula} as follows::
         * 
         *\begin{enumerate}
         *
         *\item Boolean test on address equality of argument formula.
         *
         *\item Equality comparison of this and \argument{formulae}
         *once translated, as necessary, into their negation normal
         *form.
         *
         *\end{enumerate}
         * */
        virtual bool operator==(formula& f) const;

        /*Check for syntactic inequality between this and the
         *\argument{formula} (see \method{operator==(formula\&)}).*/
        bool operator!=(formula& f) const;
        
        /*Check for syntactic equality between this and the
         *\argument{formula} (see \method{operator==(formula\&)}).*/
        bool operator==(formula* f) const;

        /*Check for syntactic inequality between this and the
         *\argument{formula} (see \method{operator==(formula\&)}).*/
        bool operator!=(formula* f) const;
        
        /*Is the formula negation normal?*/
        bool isNegNormal() const;
        
        /*Configure negation normality of the formula.*/
        void setNegNormal(bool);
        
        /*Negation normalise this formula. This method doesn't perform
         *a transformation but rather, the result is the
         *transformation.*/
        virtual formula* negNormalise(bool b = false) const = 0;
        
        /*Obtain a copy of this formula.*/
        virtual formula* copy() const = 0;
	
        /*Simplified copy of formula.*/
        virtual formula* simplify() const = 0;
	
        /*String represented copy of formula.*/
        virtual string* print() const = 0;

        /*Size of the formula. Full traversal using \class{PCsize} is
         *executed. Size is measured by calls to $sizeof()$ operator
         *on the child.*/
        virtual unsigned int size() const = 0;

        /*Length of the formula. This is a count of the number of
         *operators from which the formula is composed.*/
        virtual unsigned int length() const = 0;
    protected:
        /*Is the formula negation normal?*/
        bool negNormal;

        /*Visitation is the means by which the task of traversing
         *composite formulae is delegated to a visitation object (see
         *\module{formulaVisitation}).*/
        mutable Visitor *v;

        /*Derivatives implement visitor acceptance (see accept()).*/
        virtual void accept_() const = 0;
	
        /*Traversal for formula structure with */
        template<typename Return, class Visitor>
        Return* givenTraversal(Visitor&) const;
    };
    
    /*Abstract base type for any composite of $Elements$. $Elements$
     *can be accessed via the $Index$.*/
    template<class Element, class Index>
    class aggregate
    {
    public:
        virtual Element operator[](Index) const = 0;

        /*Index of the first element.*/
        virtual Index begin() const = 0;
        
        /*Index of the last element.*/
        virtual Index end() const = 0;
    };
    
    /*Formula whose primary connective is a unary operator.*/
    class unary : public aggregate<formula*, OperatorIndexType>
    {
    public:
        /*Copy constructor.*/
        unary(const unary& u);

        /*Default construction yields program failure.*/
        unary();

        /*Construction with argument as wff on the RHS of the primary
         *connective.*/
        unary(formula*);

        /*Free resources taken by formula and subformulae.*/
        virtual ~unary();

        /*Obtain wff that is operated on.*/
        formula* operator[](const OperatorIndexType) const;

        /*The \argument{formula} is deemed equal to this on a
         *successful cast to \parent{unary}, where an equality test on
         *subformula (\member{f}) equality of the argument and this
         *yields $true$.*/
        virtual bool operator==(formula& f) const;

        /*see \parent{aggregate}.*/
        OperatorIndexType begin()const;
        
        /*see \parent{aggregate}.*/
        OperatorIndexType end()const;
    private:
        /*wff that is operated on.*/
        formula* f;
        
        /*Index of the first and only aggregate.*/
        static const OperatorIndexType vBegin = 0;
        static const OperatorIndexType vEnd = 1;
    };

    /*Formulae that would typically require an extremely deep tree if
     *they were to be written in terms of their aggregate operators
     *(such as \child{prvNest}) are encapsulated as summaries.*/
    class summaryUnary : public unary
    {
    public:
        /*Call results in program termination. A \parent{unary} operator must
         *take an argument.*/
        summaryUnary();

        /*Copy construction.*/
        summaryUnary(const summaryUnary& u);

        /*Summary creation satisfies the following assumptions:
        *
        *\begin{itemize}
        *
        *\item The summarised unary acts on the \argument{formula}.
        *
        *\item The \argument{depth} of the summary, or the number of
        *nested summary operators is initialised (see \member{depth}).
        *
        *\item The minimum number of nestings, implies the semantics
        *of the depth in respect of the operator being nested.
        *
        *\end{itemize}
        **/        
        summaryUnary(formula*, unsigned int depth, unsigned int MIN_DEPTH);
        
        /*Decrements the operators depth if this is possible. Returns
         *$true$ only when the decrement was possible.*/
        bool operator--();

        /*Equality test. This test is mostly syntactic. For more
         *details see (formulaEquality.c++).*/
        bool operator==(formula& f)const;
        
        /*Depth to which this operator is nested.*/
        unsigned int getDepth()const;
    private:
        /*Depth to which this operator is nested.*/
        unsigned int depth;

        /*Minimum depths of nesting allowable.*/
        const unsigned int MIN_DEPTH;
    };

    /*Formula whose primary connective is a binary operator.*/
    class binary : public aggregate<formula*, OperatorIndexType>
    {
    public:
        /*A $binaryCommutative$ operator shall have access to the
         *private members of a \class{binary} operator (its parent).*/
        friend class binaryCommutative;

        /*Copy constructor.*/
        binary(const binary& b);
	
        /*Default construction yields program failure.*/
        binary();

        /*Construction with arguments as LHS and RHS of primary
         *connective respectively .*/
        binary (formula*, formula*);

        /*Free resources taken by formula and subformulae.*/
        virtual ~binary();

        /*Obtain LHS and RHS of primary connective with $0$ and $\neq
         *0$ arguments respectively.*/
        formula* operator[](const OperatorIndexType) const;

        /*Equality test. This test is mostly syntactic. For more
         *details see (formulaEquality.c++).*/
        virtual bool operator==(formula& f) const;

        OperatorIndexType begin()const;
        
        OperatorIndexType end()const;
    private:
        /*wff on LHS of operator.*/
        formula* l;

        /*wff on RHS of operator.*/
        formula* r;

        /*Index of the LHS and RHS aggregates.*/
        static const OperatorIndexType vBegin = 0;
        static const OperatorIndexType vEnd = 2;
    };

    /*Encapsulation of a binary commutative operator.*/
    class binaryCommutative 
        : public binary
    {
    public:
        binaryCommutative(const binaryCommutative& bc):binary(bc){}
        binaryCommutative(formula *l, formula *r):binary(l, r){}
        virtual bool operator==(formula& f) const;
    };

    /*PC formula*/

    /*Predicate calculus formula.*/
    class PCformula 
        : public formula
    {
    public:
        /*Equality test. This test is mostly syntactic. For more
         *details see (formulaEquality.c++).*/
        bool operator==(formula& f) const;
	
        /*Visitation calls.*/

        /*Negation normal copy of $PCformula$.*/
        formula* negNormalise(bool b = false) const;
        
        /*Copy of $PCformula$.*/
        formula* copy() const;
        
        /*Simplified copy of $PCformula$.*/
        formula* simplify() const;
        
        /*String represented copy of formula.*/
        string* print() const;
        
        /*Size of the formula.*/
        unsigned int size()const;

        /*Length of the formula.*/
        unsigned int length()const;
    };

    class literal 
        : public PCformula
    {
    public:
        /*export*/ literal(const literal& l);
        literal(string);
        ~literal();
        const string& getId() const;
        void setId(string& str);
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    private:
        string id;
    };
  
    /*Literal that has been assigned to boolean.*/
    class assLiteral 
        : public PCformula
    {
    public:
        assLiteral(const assLiteral& al);
        assLiteral(bool b = false);
        ~assLiteral();
        bool getAssignment() const;
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    private:
        bool assignment;
    };

    class conj 
        : public PCformula, 
          public binaryCommutative
    {
    public:
        conj(const conj& c);
        conj(formula *l, formula *r);
        ~conj();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    class disj 
        : public PCformula,
          public binaryCommutative
    { 
    public:
        disj(const disj& c);
        disj(formula *l, formula *r);
        ~disj();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };


    class iff 
        : public PCformula,
          public binaryCommutative
    {   
    public:
        iff(const iff& c);
        iff(formula *l, formula *r);
        ~iff();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    class imp 
        : public PCformula, 
          public binary
    {
    public:
        imp(const imp& c);
        imp(formula *l, formula *r);
        ~imp();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    class lnot 
        : public PCformula,
          public unary
    {
    public:
        lnot(const lnot& c);
        lnot(formula *f);
        ~lnot();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    /*FLTL formula*/
    
    class FLTLformula 
        : public formula
    {
    public:
        bool operator==(formula& f) const;
        formula* negNormalise(bool b = false) const;
        formula* copy() const;
        
        /*Simplified copy of $FLTLformula$.*/
        formula* simplify() const;
        
        /*String represented copy of formula.*/
        string* print() const;
        
        /*Size of the formula.*/
        unsigned int size()const;
        
        /*Length of the formula.*/
        unsigned int length()const;
    };

    /*Next*/
    class nxt 
        : public FLTLformula,
          public unary 
    {
    public:
        nxt(const nxt& c);
        nxt(formula *f);
        ~nxt();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };
    
    /*Disjunction of the next operator.*/
    class nxtDisj
        : public FLTLformula,
          public summaryUnary
    {
    public:
        nxtDisj(const nxtDisj& c);
        nxtDisj(formula *f, unsigned int depth);
        ~nxtDisj();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    /*Conjunction of the next operator.*/
    class nxtConj
        : public FLTLformula,
          public summaryUnary
    {
    public:
        nxtConj(const nxtConj& c);
        nxtConj(formula *f, unsigned int depth);
        ~nxtConj();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };
    
    /*Nested next operator.*/
    class nxtNest
        : public FLTLformula,
          public summaryUnary 
    {
    public:
        nxtNest(const nxtNest& c);
        nxtNest(formula *f, unsigned int depth);
        ~nxtNest();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };
  
    /*Week until*/
    class fut  
        : public FLTLformula,
          public binary
    {
    public:
        fut(const fut& c);
        fut(formula *l, formula *r);
        ~fut();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };
  
    /*Strong until*/
    class strFut 
        : public FLTLformula,
          public binary
    {
    public:
        strFut(const strFut& c);
        strFut(formula *l, formula *r);
        ~strFut();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    /*Future box.*/
    class fbx 
        : public FLTLformula, 
          public unary 
    {
    public:
        fbx(const fbx& c);
        fbx(formula *f);
        ~fbx();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    /*Future diamond.*/
    class fdi 
        : public FLTLformula,
          public unary 
    {
    public:
        fdi(const fdi& c);
        fdi(formula *f);
        ~fdi();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    /*Rewarding proposition.*/
    class dollars 
        : public FLTLformula
    {
    public:
        dollars(const dollars& d);
        dollars();
        ~dollars();
        virtual bool operator==(formula& f) const;
    protected:
        void accept_() const;
    };

    class PLTLformula 
        : public formula
    {
    public:
        bool operator==(formula& f) const;
        
        formula* negNormalise(bool b = false) const;
        formula* copy() const;
        
        
        /*Simplified copy of $PLTLformula$.*/
        formula* simplify() const;
        
        /*String represented copy of formula.*/
        string* print() const;
        
        /*What assigned literal does this \class{PLTLformula}
         *correspond to if it appears as a component of a
         *\class{PLTLformula}, after that \class{formula} has been
         *regressed, where the regressed \class{formula} comprises
         *part of a \member{explicitDomainSpecification.startState}
         *labelling?*/
        virtual assLiteral* startStateAssignment()const = 0;
               
        /*Size of the formula.*/
        unsigned int size()const;
        
        /*Length of the formula.*/
        unsigned int length()const;
    };

    /*PLTL formula*/

    /*Start.*/
    class startStateProposition
        : public PLTLformula
    {
    public:
        startStateProposition();
        ~startStateProposition();
        bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };
    
    /*Previously.*/
    class prv 
        : public PLTLformula,
          public unary 
    {
    public:
        prv(const prv& c);
        prv(formula *f);
        ~prv();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };

    /*Disjunction of the previously operator.*/
    class prvDisj
        : public PLTLformula,
          public summaryUnary
    {
    public:
        prvDisj(const prvDisj& c);
        prvDisj(formula *f, unsigned int depth);
        ~prvDisj();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };

    /*Conjunction of the previously operator.*/
    class prvConj
        : public PLTLformula,
          public summaryUnary
    {
    public:
        prvConj(const prvConj& c);
        prvConj(formula *f, unsigned int depth);
        ~prvConj();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };
    
    /*Nested previously operator.*/
    class prvNest
        : public PLTLformula,
          public summaryUnary
    {
    public:
        prvNest(const prvNest& c);
        prvNest(formula *f, unsigned int depth);
        ~prvNest();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };

    /*Since.*/
    class snc 
        : public PLTLformula,
          public binary
    {
    public:
        snc(const snc& c);
        snc(formula *l, formula *r);
        ~snc();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };

    /*Past box.*/
    class pbx 
        : public PLTLformula, 
          public unary 
    {
    public:
        pbx(const pbx& c);
        pbx(formula *f);
        ~pbx();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };

    /*Past diamond.*/
    class pdi 
        : public PLTLformula,
          public unary 
    {
    public:
        pdi(const pdi& c);
        pdi(formula *f);
        ~pdi();
        virtual bool operator==(formula& f) const;
        assLiteral* startStateAssignment()const;
    protected:
        void accept_() const;
    };
}
#endif
