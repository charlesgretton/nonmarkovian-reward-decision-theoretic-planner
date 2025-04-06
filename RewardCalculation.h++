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
 * Classes which help \class{State}s reward calculation.
 **/

#ifndef REWARD_CALCULATION
#define REWARD_CALCULATION

#include"domainSpecification.h++"

namespace MDP
{
    class RewardCalculator
    {
    public:
        virtual double operator()
            (pair<RewardSpecification**, RewardSpecification*> rewardSpecs,
             const DomainSpecification::PropositionSet& propositions,
             BuildFilter<formula>* buildFilter = 0) = 0;
    };

    /*The \argument{*pair.first} is updated to reflect the reward
     *associated with an \class{eState} which had a parent with
     *\class{RewardSpecification} \argument{**pair.first} and an
     *annotation \argument{**pair.first}. The
     *\argument{PropositionSet} is taken to be the set of propositions
     *that hold true in the \class{State} corresponding to argument
     *annotation.*/
    class FLTLrewardCalculator : public RewardCalculator
    {
    public:
        double operator()
            (pair<RewardSpecification**, RewardSpecification*> rewardSpecs,
             const DomainSpecification::PropositionSet& propositions,
             BuildFilter<formula>* buildFilter = 0);
    };

    /*The \argument{*pair.first} is updated to reflect the reward
     *associated with an \class{eState} which had a parent with
     *\class{RewardSpecification} \argument{**pair.first} and an
     *annotation \argument{*pair.second}. The
     *\argument{PropositionSet} is taken to be the set of propositions
     *that hold true in the \class{State} corresponding to argument
     *annotation.*/
    class PLTLrewardCalculator : public RewardCalculator
    {
    public:
        double operator()
            (pair<RewardSpecification**, RewardSpecification*>,
             const DomainSpecification::PropositionSet&,
             BuildFilter<formula>* buildFilter = 0);
    };
}
#endif
