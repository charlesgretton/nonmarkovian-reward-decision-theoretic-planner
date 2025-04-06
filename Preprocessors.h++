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
 * This module contains the implementation of the types of domain
 * preprocessing available (see \class{explicitDomainSpecification}).
 * */
#ifndef PREPROCESSOR
#define PREPROCESSOR

#include"SpecificationTypes.h++"
#include"PhaseI.h++"

namespace MDP
{
    /*Calculate the reward associated with all members of the
     *\argument{eStateSet}.
     *
     *The result of the calculation is in the \return{eStateSet}.*/
    class Preprocessor
    {
    public:
        Preprocessor();
        virtual eStateSet operator()(eStateSet&,
                                     explicitDomainSpecification&) const;
    };

    /*PhaseI preprocessing required by the annotated expansion
     *algorithm, calculates \member{basedExpandedState.labelSet}
     *\footnote{Transformation from \class{eState} to
     *\class{basedExpandedState} is performed internally.} and the
     *\member{eState.rewardSpec} associated with the members of the
     *\argument{eStateSet}.
     *
     *The result of the calculation is in the \return{eStateSet}.
     **/
    class PhaseIPreprocessor : public Preprocessor
    {
    public:
        PhaseIPreprocessor(PhaseI const*);
        eStateSet operator()(eStateSet&, explicitDomainSpecification&) const;
    private:
        PhaseI const* phaseI;
    };

    /*Minimal PhaseI preprocessing (optional over
     *\class{PhaseIPreprocessor}) for use by the annotated expansion
     *algorithm. The following computation is achieved:
     *
     *\begin{itemize}
     *
     *\item The result from the \parent{PhaseIPreprocessor} is
     *obtained on a call to the \argument{eStateSet}. The
     *\argument{explicitDomainSpecification} is updated with this
     *result.
     *
     *\item The \argument{explicitDomainSpecification} is then
     *expanded without regression. Thus, only NMRDP states are
     *generated during the expansion.
     *
     *\item The NMRDP states then have their
     *\member{basedExpandedState.labelSet} calculated such that
     *elements from atoms from the regressed successors are included if
     *not already included.
     *
     *\end{itemize}
     *
     *The result of the calculation is in the \return{eStateSet}.
     **/
    class MinimalPhaseIPreprocessor : public PhaseIPreprocessor
    {
    public:
        MinimalPhaseIPreprocessor(PhaseI const*);
        eStateSet operator()(eStateSet&, explicitDomainSpecification&)const;
    };
}

#endif
