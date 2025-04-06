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
 * Solution algorithms all have a common ancestor which is provided by
 * this module.
 * */

#ifndef ALGORITHM_HPP
#define ALGORITHM_HPP

#include "SpecificationTypes.h++"
#include "AlgorithmTypes.h++"

using namespace MDP;

namespace MDP 
{
    /*An \namespace{MDP} algorithm is the encapsulation of a solution
     *method (\cite{dean, hansen, boutilier00, feng}).
     *
     *Derivative algorithms are assumed to be iterative, as they shall
     *involve or be characterised by iteration. Their execution
     *comprises a number of dynamic programming steps which are in
     *turn initiated via a call to \method{operator()()}.*/
    class Algorithm
    {
    public:
        /*Perform an iteration of the algorithm, and return $true$ if no
         *more iterations need to be done.*/
        virtual bool operator()() = 0;

        /*Acceptable error. Interpretation is left to derivatives.*/
        virtual void setEpsilon(const double);
        
        /*Acceptable error. Interpretation is left to derivatives.*/
        double getEpsilon()const;
        
        /*Discount factor (in some documents this is denoted with
         *beta).*/
        virtual void setGamma(const double);

        /*Discount factor (in some documents this is denoted with
         *beta).*/
        double getGamma()const;
    protected:
        /*Specification of a domain for which a solution/policy is to
         *be computed by this algorithm.*/
        DomainSpecification* domSpec;

        /*Discount factor (in some documents this is denoted with
         *beta).*/
        double gamma;

        /*Acceptable error. Interpretation is left to derivatives.*/
        double epsilon;

        /*Error between some current policy and the optimal policy.*/
        double error;
        
        /*Default discount factor.*/
        static const double defaultGamma = 0.95;
        
        /*Default acceptable error*/
        static const double defaultEspilon = 0.001;
    public:
        /*Default algorithm construction. Structured solution methods
         *sometimes ignore the parent construction functionality.*/
        Algorithm(){}

        /*Algorithm construction which accounts for algorithm
         *parameters.*/
        Algorithm(DomainSpecification &domSpec,
                  double gamma = defaultGamma,
                  double epsilon = defaultEspilon);

        /*Virtual for children.*/
        virtual ~Algorithm();

        /*Error between the current policy and the optimal policy.*/
        virtual double getError()const;
    };
};

#endif
