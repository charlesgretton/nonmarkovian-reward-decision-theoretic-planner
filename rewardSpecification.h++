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
 * Representation of and operations concerning NMRDPs reward
 * specifications.
 **/
#ifndef REWARD_SPEC
#define REWARD_SPEC

#include"formula.h++"
#include"formulaTreeCast.h++"

#include"RewardCalculation.h++"
#include"domainSpecification.h++"

#include<string>
#include<map>

using namespace Formula;

using namespace std;

namespace MDP
{ 
    class RewardSpecification
    {
    public:
        /*Storage type for reward formula and their associated
         *values. The structure can be be seen as a \class{formula}
         *\class{value/double} pair endowed with an equality test.*/
        struct FSpec
        {
            formula *form;
            double value;
            
            /*Equality is solely that of the members.*/
            bool operator==(const FSpec& fSpec)const;
        };

        typedef struct FSpec FSpec;

        /*Containment type for reward is a mapping from user defined
         *reward label string to an \struct{FSpec}.*/
        typedef map<string, FSpec> SpecificationContainer;

        /*Constant reward iterator.*/
        typedef SpecificationContainer::const_iterator crIterator;

        /*Reward iterator.*/
        typedef SpecificationContainer::iterator rIterator;

        /*Function object to test equality of a reward specification
         *element \struct{FSpec} \member{formula} and a
         *\class{formula}.*/
        template<typename F>
        class FormulaEqual : public binary_function<SpecificationContainer::value_type, F, bool>
        {
        public:
            bool operator()(const SpecificationContainer::value_type&,
                            const vector<formula const*>&) const;
            
            bool operator()(const SpecificationContainer::value_type&, const F&) const;
        };

        /*Contained equality. Same as \class{FormulaEqual} only
         *doesn't suffer from pure virtual dilemma that occurs when
         *formula type is unknown.*/
        class ContainedFormulaEqual : public binary_function<SpecificationContainer::value_type, vector<formula const*>, bool>
        {
        public:
            bool operator()(const SpecificationContainer::value_type&,
                            const vector<formula const*>&) const;
        };

        /*An exception type for reward abnormality.*/
        class RewardAbnormality
        {
        public:
            RewardAbnormality(rIterator &p);

            const string getMessage()const;
        private:
            const string message;
        };

        /*Traverse this specification using the argument visitation
         *sequence \argument{traverser}, over the state characterised
         *by the argument \argument{propositions}.*/
        template<class Traverser, typename Propositions>
        double sequenceTraversal(Traverser&,
                                 const Propositions&,
                                 const BuildFilter<formula>&);

        /*Traverse this specification over the state characterised by
         *the argument \argument{propositionSet} applying the
         *\argument{BuildFilter} to temporal components of the
         *result.*/
        virtual double sequenceTraversal(const DomainSpecification::PropositionSet&,
                                         const BuildFilter<formula>& = NullFilter<formula>());
        
        /*Reward specification construction provides the following
         *assumptions:
         *
         *\begin{itemize}
         *
         *\item That a type expansion is required until a formula
         *addition (see \method{addSpecComponent()}) suggests a type
         *(ie: \class{FLTLrewardSpecification} or
         *\class{PLTLrewardSpecification}).
         *
         *\end{itemize} */
        RewardSpecification();

        /*Generate a reward specification from the argument
         *specification.*/
        RewardSpecification(const RewardSpecification&);

        /*Free resources taken by the free store formulae.*/
        virtual ~RewardSpecification();
      
        /* Add a reward specification triple to the specification. The
         * argument \argument{formula} is not copied. If a triple with
         * the \argument{string} key already exists it is replaced.
         *
         * The following assumptions are satisfied by this method:
         *
         * \begin{itemize}
         *
         * \item A type change shall occur if the argument formula
         * provides details from which the reward specification type
         * can be inferred.
         *
         * \end{itemize}
         *
         * */
        virtual RewardSpecification* addSpecComponent(string, double, formula*);
    
        /*Does this instance of a specification have a type
         *expansion?*/
        bool typeExpansionPossible()const;

        /*Could a possible domain state be characterised by this
         *specification?
         *
         *\textbf{NOTE: } For instance an FLTL specification which
         *after progression contains a \class{assLiteral(false)} can
         *only annotate an impossible state.*/
        virtual bool isPossible()const;

        /*What type does this domain specification expand to?*/
        virtual RewardSpecification* expandsTo()const;

        /*Reward identified by \argument{label}.*/
        double getReward(const string&);
        
        typedef crIterator const_iterator;
        typedef rIterator iterator;
        
        const_iterator begin()const;
        
        const_iterator end()const;
        
        iterator begin();
        
        iterator end();

        /*Returns a reference to an \struct{FSpec} associated with the
         *argument key \argument{label} in this specification.*/
        FSpec& operator[](const string&);

        /*Remove reward specification element with the argument
         *\argument{label}. Formula deletion of the \struct{FSpec}
         *\member{form} does not occur.*/
        void erase(const string&);
        
        /*Check for duplicate reward formulae.*/
        string checkDuplication()const;

        /*Negation normalise all specification reward formulae.*/
        void normalise();

        /*Simplify normalise all specification reward formulae.*/
        void simplify();
        
        /*How does this specification appear as a string?*/
        string& toString()const;

        /*Copy this specification.*/
        virtual RewardSpecification* copy()const;
        
        /*Returns equality test of this and argument
         *\member{specificationContents}.
         *
         *Important:: Only the \member{specificationContents} is considered.*/
        bool operator==(const RewardSpecification&) const;

        /*Find the reward element labelled with \argument{string}.
         *
         *Delegates implementation to \member{specificationContents}.*/
        iterator find(const string&);
        
        /**Find the reward element labelled with \argument{string}.
         *
         *Delegates implementation to \member{specificationContents}.*/
        const_iterator find(const string&)const;

        /*How many elements is this reward specification comprised of?*/
        unsigned int size() const;

        /*Approximatly the amount of memory taken by this
         *specification. The result is a number of Kb.*/
        virtual unsigned int memory()const;
        
        /*Obtain a reward calculator suitable for calculating the
         *reward associated with an \class{eState} labelled with this
         *specification (assumed to only contain PC formulae).*/
        virtual RewardCalculator* getRewardCalculator()const;

        /*The sum of the reward associated with speciifcation reward elements.*/
        double getRewardSum()const;
    protected:
        /*Reward formulae and associated values are identified by their
         *string names.*/
        SpecificationContainer specificationContents;
	
        /*Is an expansion attempt required?*/
        bool expansionRequired;

        /*There are program elements that use the specification in a
         *string form. Because this string is expensive to calculate
         *it is cached mutably. The cache is used until the
         *specification is changed.*/
        mutable string* rewardCache;
        
        /*Calculate the reward associated with the traversal of the
         *specification component \argument{p}, where the traversal
         *tells us if it was \argument{rewarding} and provides the
         *traversed formula \argument{build}.*/
        virtual double getReward(bool rewarding, rIterator &p, formula *build)const;

        /*Clear the cached reward.*/
        void clearRewardCache();
    };

    class FLTLrewardSpecification : public RewardSpecification
    {
    public:
        /*Copy construction.*/
        FLTLrewardSpecification(const FLTLrewardSpecification& rs);

        /*Copy construction.*/
        FLTLrewardSpecification(const RewardSpecification& rs);

        
        /*Traverse this specification over the state characterised by
         *the argument \argument{propositionSet} applying the
         *\argument{buildFilter} to temporal components of the
         *result.*/
        double sequenceTraversal(const DomainSpecification::PropositionSet&,
                                 const BuildFilter<formula>& = NullFilter<formula>());

        /*Could a possible domain state be characterised by this
          specification?*/
        bool isPossible()const;

        /*Add a reward specification triple to the specification.*/
        RewardSpecification* addSpecComponent(string, double, formula*);

        /*What type does this domain specification expand to?*/
        RewardSpecification* expandsTo()const;

        /*see \member{FLTLrewardSpecification()}.*/
        RewardSpecification* copy()const;

        /*see \class{FLTLrewardCalculator}*/
        RewardCalculator* getRewardCalculator()const;
    protected:
        /*see \method{RewardSpecification::getReward()}*/
        double getReward(bool, rIterator&, formula*)const;
    };

    class PLTLrewardSpecification : public RewardSpecification
    {
    public:
        /*Copy construction.*/
        PLTLrewardSpecification(const PLTLrewardSpecification& rs);

        /*Copy construction.*/
        PLTLrewardSpecification(const RewardSpecification& rs);
       
        /*Add a reward specification triple to the specification.*/
        RewardSpecification* addSpecComponent(string, double, formula*);

        /*PLTL domain specifications are not expandable.*/
        RewardSpecification* expandsTo()const;

        /*Copy this specification.*/
        RewardSpecification* copy()const;
    };
}

#include"rewardSpecification_templates.h++"

#endif
