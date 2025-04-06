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
 * \paragraph{Purpose}
 *
 * This module contains the implementations of visitation based formula
 * progression and regression.
 *
 * \bextbf{NOTE:} The build should be initialised between each
 * traversal for the purpose of reward initialisation.
 *
 * Any sequence traversal \class{Visitor} must be configured with a
 * \class{BuildFilter} prior to traversal. The reason for this is that
 * any temporal elements that appear in a specification resulting from
 * traversal are filtered. For details on how this is used during
 * Phase II of the annotated expansion algorithm see
 * \class{EntailmentFilter}.
 **/
#ifndef FORMULA_SEQUENCE_TRAVERSAL

#define FORMULA_SEQUENCE_TRAVERSAL

#include"formulaSimplifier.h++"
#include"formula.h++"
#include<cassert>
#include<iostream>
#include<string>
#include<vector>
#include<set>

namespace Formula
{   
    template<class C, 
        typename PropositionContainer = vector<string>*>
    class PCsequence_traversal : 
        public Builder<formula>,
        public virtual PCvisitor,
        public C
    {
    public:
        void visit(const conj&);
        void visit(const disj&);
        void visit(const literal&);
        void visit(const assLiteral&);
        void visit(const iff&);
        void visit(const imp&);
        void visit(const lnot&);

        /*Build initialisation in the case of sequence traversal
         *achieves the following:
         *
         *\begin{enumerate}
         *
         *\item Formula memorisation: After initialisation the first
         *visitation call commits to memory the formula whose
         *visitation was requested. Thus, the caller need not
         *request revisitation when necessary.
         *
         *\item Give reward: The sequence traversal in the case of
         *FLTL formula may find that without reward a progression
         *yields the formula $false$. In this case a repass must be
         *made when the caller queries the case for reward (see
         *\method{rewarding()}). Initialisation configures the
         *necessity of a repass.
         *
         *\end{enumerate} */
        virtual formula* initBuild();

        /*Set propositions which shall be assigned to $true$ during
          traversal.*/
        void setPropositionContainer(PropositionContainer* pc)
            {
                propositionContainer = pc;
                propositions = set<string>(pc->begin(), pc->end());
            }
        
        /*Did progression find the formula which was last traversed
         *rewarding?
         *
         *This method has the following effects:
         *
         *\begin{itemize}
         *
         *\item The formula obtained during traversal is simplified
         *using the boolean simplification provided by
         *\method{formula.simplify()}.
         *
         *\item A re--traversal is made in the case that reward my be
         *necessary.
         *
         *\end{itemize} */
        virtual bool rewarding() = 0;

        /*Configure the rewardability of the last formula traversed.*/
        void rewarding(bool b){reward = b;};
    protected:
        /*Formulae are simplified during sequence traversal.*/
        PCsimplifier<NA> simplifier;

        /*Reward is determined by traversal derivatives.*/
        bool reward;

        /*Propositions which shall be assigned to $true$ during
         *traversal.*/
        PropositionContainer *propositionContainer;
	
        /*Constant reference to a proposition.*/
        typename PropositionContainer::const_iterator cProposition;
	
        /*Formula whose visitation was requested by the non--visitor
         *caller.*/
        formula *visitationCache;

        /*Fast propositional search structure.*/
        set<string> propositions;
        
        /*Configures the \member{visitationCache}.*/
        void evaluate(formula const* f)
            {
                if(0 == visitationCache)
                    visitationCache = const_cast<formula*>(f);
            }
    };
    

    /*It is required that formulae intended for the purpose of control
     *knowledge contain no dollars propositionals.*/
    template<typename PropositionContainer>
    class FLTLprogression :
        public PCsequence_traversal<FLTLvisitor, PropositionContainer>
    {
    public:
        void visit(const nxt&);
        void visit(const fut&);
        /*We do not progress possible eventualities. A call to this
         *function yields an assertion violation.*/
        void visit(const strFut&);
        void visit(const fbx&);
        /*We do not progress possible eventualities. A call to this
         *function yields an assertion violation.*/
        void visit(const fdi&);
        void visit(const dollars&);
        
        void visit(const nxtDisj&);
        void visit(const nxtConj&);
        void visit(const nxtNest&);
        
        /*Was the reward formula progressed control knowledge?*/
        bool wasControl(){return control;}
	
        /*Was the progressed formula rewarding?*/
        bool rewarding();

        /*Parent and control initialisation. Progressed formulae are
         *considered to be control formula until traversal proves
         *otherwise.*/
        formula* initBuild()
            {
                control = true;
                return PCsequence_traversal<FLTLvisitor, PropositionContainer>::initBuild();
            }
    protected:
        /*Was the reward formula progress control knowledge?*/
        bool control;
	
        void accept_(formula* f)const {FLTLvisitor::accept_(f);}
    }; 
    
    template<typename PropositionContainer>
    class PLTLregression :
        public PCsequence_traversal<PLTLvisitor, PropositionContainer>
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

        bool rewarding();
    protected:
        void accept_(formula* f)const {PLTLvisitor::accept_(f);}
    };    
}

#include"formulaSequenceTraversal_templates.h++"

#endif
