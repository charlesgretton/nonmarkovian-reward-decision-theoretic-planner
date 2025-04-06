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
 * This module is purely architectural. It provides the basis for an
 * adaptation of the common visitor pattern to c++. The composites for
 * which the visitation is provided are \class{formula}s.
 *
 * The architecture includes a common \class{Builder} concept, which 
 * has been extended to support template element construction and 
 * filtration. The visitation is encapsulated into three distinct
 * components, the \class{PCvisitor}, \class{FLTLvisitor} and
 * \class{PLTLvisitor}, the latter two, being children of the 
 * \parent{PCvisitor}.
 *
 * \paragraph{:Architectural Notes:}
 *
 * When examining the means by which we achieved this architecture it
 * is important to take the following points into consideration.
 *
 * \begin{quote}
 * Most C++ compilers (G++ included) do not yet implement
 * export, which is necessary for separate compilation of
 * template declarations and definitions. Without export, a
 * template definition must be in scope to be used. The obvious
 * workaround is simply to place all definitions in the header
 * itself. Alternatively, the compilation unit containing template
 * definitions may be included from the header.
 * \end{quote}
 *
 * \emph{http://gcc.gnu.org/bugs.html$\#$known}
 *
 * \begin{quote}
 * $[14.6]$ specifies how names are looked up inside a template. G++
 * does not do this correctly, but for most templates this will not be
 * noticeable.
 * \end{quote}
 *
 * \emph{http://gcc.gnu.org/bugs.html$\#$known}
 *
 * 
 * \textbf{NOTE:} One may think it strange that pure virtual functions
 * are re-qualified in derivations of $PCvisitor$. This is due to the
 * scope of C++ overload resolution $[13.3/1996/draft]$.
 * */

#ifndef FORMULA_VISITATION
#define FORMULA_VISITATION

#include"formulaTypes.h++"
#include<cassert>
#include<iostream>

namespace Formula
{          
    /*Template argument to ignorant visitors.*/
    class NA{};

    /*\class{Builder}s that need to filter elements should do so with
     *a subclass of the \class{BuildFilter}.*/
    template<typename T>
    class BuildFilter
    {
    public:
        /*Apply the filter. Some filters only act minimally, ie: when they
         *are given their \argument{lastChance}.*/
        virtual T* operator()(T*, bool lastChance = false)const = 0;
    };

    /*A filter that does nothing.*/
    template<typename T>
    class NullFilter : public BuildFilter<T>
    {
    public:
        T* operator()(T* t, bool lastChance)const{return t;}
    };
    
    /*Visitation builder that builds an item by traversal shall be a
     *derivative of a \class{Builder}.*/
    template<typename T>
    class Builder
    {
    public:
        virtual ~Builder() {}

        /*Obtain the item that was built by derivative.*/
        virtual T* getBuild() const
            {
                return item;
            }

        /*Initialise the build.*/
        virtual T* initBuild() = 0;

        virtual void setBuildFilter(BuildFilter<T> const* buildFilter)
            {
                this->buildFilter = buildFilter;
            } 
    protected:
        /*Item that is built. This is by no means the property of a
         *single instance of any object. Thus the state of a builder
         *is not considered changed solely due to a change in what 
         *is being built.*/
        mutable T* item;

        /*Filter that may be used during build.*/
        BuildFilter<T> const* buildFilter;
    };

    /*Virtual morphing visitor which is able to morph, via delegation,
     *into a \child{FLTLvisitor} or \child{PLTLvisitor}.*/
    class PCvisitor
    {
    public:
        /*Initialise visitor assigning a default behaviour mode.*/
        PCvisitor(){wasVirtualVisit = false;}

        /*Method which attempts to correct a situation arising from
         *virtual visitation. It is up to derivatives to define full
         *behavioural correction.*/
        virtual void repass(const formula &c){wasVirtualVisit = false;}

        /*Was the last attempted visitation unsuccessful?*/
        bool needRepass(){return wasVirtualVisit;}

        virtual void visit(const conj&) = 0;
        virtual void visit(const disj&) = 0;
        virtual void visit(const literal&) = 0;
        virtual void visit(const assLiteral&) = 0;
        virtual void visit(const iff&) = 0;
        virtual void visit(const imp&) = 0;
        virtual void visit(const lnot&) = 0;

        /*This function implements a virtual visitation pass. In the
         *case when it is called, the caller may choose to re-attempt a
         *visitation using \method{repass()}.*/
        template<class C>
        void visit(C c){wasVirtualVisit = true;}
    protected:
        virtual void accept_(formula const* f)const;
    private:
        /*Has an un--corrected virtual visitation occurred?*/
        bool wasVirtualVisit;
    };

    /*Charles::We want the visitor to be a template argument so we can make a
      fltl and pltl visitor.*/
    
    class FLTLvisitor : virtual public PCvisitor
    {
    public:
        virtual void visit(const conj&) = 0;
        virtual void visit(const disj&) = 0;
        virtual void visit(const literal&) = 0;
        virtual void visit(const assLiteral&) = 0;
        virtual void visit(const iff&) = 0;
        virtual void visit(const imp&) = 0;
        virtual void visit(const lnot&) = 0;

        virtual void visit(const nxt&) = 0;
        virtual void visit(const fut&) = 0;
        virtual void visit(const strFut&) = 0;
        virtual void visit(const fbx&) = 0;
        virtual void visit(const fdi&) = 0;
        virtual void visit(const dollars&) = 0;
        
        virtual void visit(const nxtDisj&) = 0;
        virtual void visit(const nxtConj&) = 0;
        virtual void visit(const nxtNest&) = 0;

        template<class C>
        void visit(C c){cout<<"Error: Non-existent FLTL visitation attempted.\n";}
    protected:
        virtual void accept_(formula const* f)const;
    };
    
    class PLTLvisitor : virtual public PCvisitor
    {
    public:
        virtual void visit(const conj&) = 0;
        virtual void visit(const disj&) = 0;
        virtual void visit(const literal&) = 0;
        virtual void visit(const assLiteral&) = 0;
        virtual void visit(const iff&) = 0;
        virtual void visit(const imp&) = 0;
        virtual void visit(const lnot&) = 0;

        virtual void visit(const startStateProposition&) = 0;
        virtual void visit(const prv&) = 0;
        virtual void visit(const snc&) = 0;
        virtual void visit(const pbx&) = 0;
        virtual void visit(const pdi&) = 0;

        virtual void visit(const prvConj&) = 0;
        virtual void visit(const prvDisj&) = 0;
        virtual void visit(const prvNest&) = 0;
        
        template<class C>
        void visit(C c){cout<<"Error: Non-existent PLTL visitation attempted.\n";}
    protected:
        virtual void accept_(formula const* f)const;
    };

    /*Interface to \class{PCvisitor}, \class{FLTLvisitor} or
     *\class{PLTLvisitor} visitor instances.*/
    class Visitor
    {
    public:
        Visitor(){pcV = 0;fltlV = 0;pltlV = 0;}

        void setVisitor(PCvisitor const* v)const
            {
                pcV = const_cast<PCvisitor*>(v);
                fltlV = 0;
                pltlV = 0;
            }
        void setVisitor(FLTLvisitor const* v)const
            {
                pcV = 0;
                fltlV = const_cast<FLTLvisitor*>(v);
                pltlV = 0;
            }
        void setVisitor(PLTLvisitor const* v)const
            {
                pcV = 0;
                fltlV = 0;
                pltlV = const_cast<PLTLvisitor*>(v);
            }

        /*Interface to visitor instances. When formula traversal
         *begins, if an appropriate visitor doesn't exist it is
         *created.*/
        template<typename C>
        void visit(C const *c) const
            {
                if(0 != pcV)/*Alternate visitation is possible if $c$ is
                              not a pc formula.*/
                {
                    /*Attempt a visitation.*/
                    pcV->visit(*c);

                    /*Make copy of formula in the case of a repass.*/
                    if(pcV->needRepass())
                    {
                        C tmp = *c;

                        /*In the case of unexpected children, (If traversal
                          was assumed to be over a pc structure and the
                          structure contains an fltl element perhaps) then
                          a repass must be made.*/
                        pcV->repass(tmp);
                    }
                }
                else if(0 != fltlV)/*Error if $c$ not fltl or pc formula.*/
                    fltlV->visit(*c);
                else if(0 != pltlV)/*Error if $c$ not pltl or pc formula.*/
                    pltlV->visit(*c);
            }
    private:
        mutable PCvisitor *pcV;
        mutable FLTLvisitor *fltlV;
        mutable PLTLvisitor *pltlV;
    };
}
#endif
