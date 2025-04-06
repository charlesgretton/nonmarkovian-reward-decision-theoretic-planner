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
 * This module is concerned with domain state representation and
 * supporting types.
 **/
#ifndef STATES
#define STATES

#include"SpecificationTypes.h++"
#include"PhaseI.h++"

using namespace std;

namespace MDP
{
    /*\class{State} equality checking using \method{State.operator==()}.*/
	class StateEqual : public binary_function<State const*, State const*, bool>
	{
	public:
	    bool operator()(State const*, State const*) const;
        bool operator()(const State&, const State&) const;
	};

    /*\class{State} greater than or equal to checking using
     *only the value associated with the state.*/
	class StateValueGeq : public binary_function<State const*, State const*, bool>
	{
	public:
	    bool operator()(State const*, State const*) const;
        bool operator()(const State&, const State&) const;
	};

    /*\class{State} possibility checking using \method{State.isPossible()}.*/
    class IsPossible : public unary_function<State const*, bool>
    {
    public:
        bool operator()(State const*) const;
    };

    /*see \class{IsPossible}.*/
    class PairIsPossible : public unary_function<const TransitionPair&, bool>
    {
    public:
        bool operator()(const TransitionPair&) const;
    };

/****************************************** State */
    
    /*A state is vertex characterised by a set of domain propositions
     *\member{propositions}, a \member{reward}, its possibility
     *\member{possible} and transitions \member{stms}.
     *
     *States are also provided with a \member{value} to be used by
     *clients as they wish. A \class{State}s value is initialised to
     *the \class{State} \member{reward} when the reward is calculated
     *(see \method{calculateReward}).
     *
     *\textbf{NOTE:}The primary intent of a \class{State} is the
     *encapsulation of a proposition set thus the behaviour of some of
     *the functions and methods are unintuitive.
     *
     *Another way that we view a state is as a container of
     *transitional information relating states to states via
     *transitions. The traversal operations supported by the
     *\class{State} support this functionality.
     **/
    class State
    {
    public:
        /*States are characterised by the argument set of propositions
         *which hold when a domain is in them. States are assumed to
         *be possible. A states \member{colouring} is initially $0$.*/
        State(const DomainSpecification::PropositionSet&);

        /*All members less \member{stms} and
         *\member{value}\footnote{Which is initialised to the
         *\argument{State}s reward.} are copied on construction. A
         *states \member{colouring}, even on copying, is initially
         *$0$.*/
        State(const State&);

        /*See descendents.*/
        virtual ~State();

        /*Is this state possible?*/
        bool isPossible()const;

        /*Calculate how possible this state is, based on transitional
         *information. If no transitions are possible then this state
         *is marked as impossible.*/
        void calculatePossibility();
        
        /*Equality of state is \emph{only} based on propositional
         *equality, how \member{possible} the state is and the
         *\member{reward} (\textbf{Not the \member{value}}) associated
         *with the state.*/
        virtual bool operator==(const State&)const;

        /*Inequality of state is \emph{only} based on propositional
         *inequality, possibility and that of the reward.*/
        virtual bool operator<(const State&)const;

        /*Endows this state with transitional information by
         *assigning the \argument{StateTransitionMatrices} to
         *\member{stms}.*/
        void addSuccessors(const StateTransitionMatrices&);

        /*Get the set of state characterising propositions.*/
        const DomainSpecification::PropositionSet& getPropositions() const;

        /*Get the set of domain propositions which do not characterise
         *this state.*/
        DomainSpecification::PropositionSet getNonPropositions(const DomainSpecification&);

        /*Add a proposition to the set which characterises this state.*/
        void addProposition(proposition);

        /*Remove a proposition from the set which characterises this state.*/
        void removeProposition(proposition);
        
        /*Copy this state (see \function{State(const State\&)}).*/
        virtual State* copy() const;

        /*Both the \member{propositions} and the \member{stms} are
         *re--initialised.*/
        virtual void clear();

        /*How does this state look as a string?*/
        virtual string toString(bool identify = false) const;

        /*Propositions true in the current state.*/
        string propositionsToString() const;
        
        /*For use with descendents. In this case returns true without
         *altering any class members.*/
        virtual bool calculateReward(BuildFilter<formula>* = 0);

        /*What is the reward associated with this state?*/
        double getReward() const;

        /*Set the reward associated with this state.*/
        void setReward(double);
        
        /*Returns the set of all possible successors.*/
        StateSet getSuccessors() const;

        /*Remove all actions that are impossible (lead to an
         *impossible state).*/
        void removeImpossibleActions();

        /*This state remains possible, however all the actions are
          removed.*/
        void removeAllActions();
        
        /*Associate a value with this state.*/
        void setValue(double);

        /*What is the value associated with this state?*/
        double getValue() const;

        /*Approximately the amount of memory taken by this
         *\class{State}. The result is a number of Kb.*/
        virtual unsigned int memory()const;
        
        /*Transition traversal.*/
        
        typedef StateTransitionMatrices::const_iterator const_iterator;
        typedef StateTransitionMatrices::iterator iterator;
        
        inline const_iterator begin()const{return stms.begin();}
        inline const_iterator end()const{return stms.end();}
        inline iterator begin(){return stms.begin();}
        inline iterator end(){return stms.end();}

        inline iterator find(action& act){return stms.find(act);}
        inline const_iterator find(action& act)const{return stms.find(act);}
        
        /*The\argument{int} argument is the colour that this state is
         *set to (see \member{colouring}).*/
        void setColour(int);

        /*What colour is this state?*/
        int getColour();
    protected:
        /*Set of state characterising propositions.*/
        DomainSpecification::PropositionSet propositions;

        /*Corresponding reward.*/
        double reward;

        /*A "value" associated with the state, often the expected
         *future reward. This is initialised at the same time as the
         *\member{reward}. This includes when a call to
         *\function{setReward()} is made and thus on construction.*/
        double value;
        
        /*Is this state possible given a domain specification?*/
        bool possible;

        /*State transitional information.*/
        StateTransitionMatrices stms;
    private:
        /*Many graph algorithms require that states have a colouring;
         *this integer has been added to support this (see
         *\class{LAO}).*/
        int colouring;

        /*Has the transition information been explicitly emptied?*/
        bool stmsEmptied;
        
        /*Transition description.*/
        string transitionsString()const;
    };

/****************************************** eState */
    
    /*This encapsulates extended states. \class{eState}s are domain
     *states, \class{State}s, augmented with a \member{rewardSpecification}.*/
    class eState : public State
    {
    public:
        /*Sequence traversal is not initiated/executed on
         *construction, see \method{calculateReward()}. Thus reward
         *information is not calculated.*/
        eState(DomainSpecification::PropositionSet&, RewardSpecification&);

        /*Sequence traversal is not initiated on construction, see
         *\method{calculateReward()}. Thus reward information is not
         *calculated.*/
        eState(const eState&);

        /*Destruction commits to the following:
         *
         *\begin{itemize}
         *
         *\item Heap taken by \member{rewardSpecification} is freed.
         *
         *\end{itemize}*/
        ~eState();
        
        /*State type equivalence is verified prior to
         *\method{operator==(const eState\&)} equivalence call.*/
        bool operator==(const State&) const;

        /*Equality is based on the \member{rewardSpecification} and the
         *equality of \parent{State} member variables (see
         *\method{State.operator==()}).*/
        bool operator==(const eState&) const;
        
        /*Inequality is based on the contents of the
         *\member{rewardSpecification} and the inequality of
         *\parent{State} member variables (see
         *\method{State.operator==()}).*/
        bool operator<(const eState& s) const;
        
        /*Same as \function{eState(const eState\&)} copy
         *construction.*/
        virtual State* copy() const;

        /*Same as parent clearing. This does not clear the cloned
         *reward specification. On the contrary, this is kept for the
         *purpose of traversal by a call to \method{calculateReward}.
         *
         *\textbf{NOTE:}The need to keep the reward information is so
         *that during state based expansion, states can be copied to
         *create children.  The predecessor reward is then useful in
         *determining the truth of reward elements associated with a
         *successor and this state.*/
        virtual void clear();

        /*How does this state look as a string?*/
        virtual string toString(bool identify = false)const;

        /*The argument should be $true$ if the state has a predecessor
         *when the call is made and $false$ otherwise.*/
        virtual void hasPredecessor(bool);
        
        /*Adjusts \member{rewardSpecification} through a sequence
         *traversal over \parent{State} \member{propositions}, updates
         *the reward associated with this \class{eState} and returns
         *whether or not this \class{eState} \member{isPossible()}.
         *
         *The following assertions must be satisfied:
         *
         *\begin{itemize}
         *
         *\item The \member{rewardSpecification} is equal to that of the
         *predecessor in an action graph (NFSA).
         *
         *\end{itemize}
         *
         *The \argument{BuildFilter} is ignored in this case as the
         *state has no basis (see \class{basedExpandedState}) on which
         *to filter.*/
        bool calculateReward(BuildFilter<formula>* = 0);

        /*\method{calculateReward()} is called. Child reward
         *generation can be more complicated and useful (see
         *\class{basedExpandedState}).*/
        virtual void generateReward();

        /*Obtain a constant handle to the states reward.*/
        RewardSpecification const* getRewardSpecification() const;

        /*Approximately the amount of memory taken by this
         *\class{State}. The result is a number of Kb.*/
        unsigned int memory()const;
    protected:
        /*Reward formulae which this e-state is annotated with.*/
        RewardSpecification* rewardSpecification;
    };
    
    /*An expanded state that is based on a labelled set.*/
    class basedExpandedState : public eState
    {
    public:
        /*Construction takes the set of state characterising
         *propositions \argument{PropositionSet}, the reward
         *specification of the states predecessor
         *\argument{predecessorRewardSpecification} and the
         *\argument{labelSet}. The \class{RewardSpecification} are
         *copied via a \method{RewardSpecification.copy()} call.*/
        basedExpandedState(DomainSpecification::PropositionSet&,
                           RewardSpecification& predecessorRewardSpecification,
                           RewardSpecification& labelSet);

        /*\member{labelSet} is \method{RewardSpecification.copy()}'d in
         *addition to \parent{eState} copy construction*/
        basedExpandedState(const basedExpandedState&);

        /*Same as \parent{eState} copy construction given the first
         *argument. The second argument is assigned to
         *\member{labelSet}.
         *
         *\textbf{NOTE:} The second argument shall be deleted on a
         *\method{~basedExpandedState()} call.
         **/
        basedExpandedState(const eState&, RewardSpecification* labelSet);

        /*\begin{itemize}
         *
         *\item Heap taken by \member{labelSet} is freed.
         *
         *\end{itemize}*/
        ~basedExpandedState();

        /*Same as \function{basedExpandedState(const
         *basedExpandedState\&)} copy construction.*/
        State* copy()const;
         
        /*If the \argument{bool} is $false$ then the state is a start
         *state and the \member{rewardSpecification} is deleted for future
         *calculation with a call to \method{calculateReward()}. This
         *method has two purposes in the context of a
         *\class{basedExpandedState}
         *
         *\begin{enumerate}
         *
         *\item To free heap space allocated to the
         *\member{rewardSpecification} without destroying the state.
         *
         *\item To ensure that reward calculation traversal is aware
         *that there is no past associated with this state.
         *
         *\end{enumerate}
         **/
        void hasPredecessor(bool);
        
        /*Same as \parent{eState}, however type information is
         *kept. In this case, if an \argument{BuildFilter} is provided
         *the traversal executes the desired filtration. Filtration in
         *the case that this state has no predecessor or in the atoms
         *case is provided for if no filtration is requested.*/
        bool calculateReward(BuildFilter<formula>* = 0);

        /*This function calculates the \member{rewardSpecification}
         *from the \member{labelSet} given the state characterising
         *propositions \member{propositions} without any
         *adjustments. The resulting \member{rewardSpecification}
         *is a traversal \footnote{probably regression, thus no
         *exceptions are caught} of the \member{labelSet}.*/
        void generateReward();
        
        /*What is the length of the annotation label?*/
        unsigned int getLabelLength() const;

        /*Get the reward specification that labels this state.*/
        RewardSpecification* getLabelSet();

        /*Configure the reward specification that labels this state.*/
        void setLabelSet(const RewardSpecification&);
        
        /*Add elements from this states reward specification to the label set
         *of the argument specification.*/
        void updatePredecessorReward(basedExpandedState&) const;

        /*Approximately the amount of memory taken by this
         *\class{basedExpandedState}. The result is a number of Kb.*/
        unsigned int memory()const;
    protected:
        /*Reward formulae which comprises this states labelled set.*/
        RewardSpecification* labelSet;
    };
}
#endif
