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

#include"Algorithm.h++"
#include"domainSpecification.h++"

using namespace MDP;

Algorithm::Algorithm(DomainSpecification &domSpec,
                     double gamma,
                     double epsilon)
    : domSpec(&domSpec),
      gamma(gamma),
      epsilon(epsilon),
      error(0.0)
{}

Algorithm::~Algorithm()
{}

double Algorithm::getError()const
{
    return error;
}

void Algorithm::setEpsilon(const double eps)
{
    epsilon = eps;
}

double Algorithm::getEpsilon()const
{
    return epsilon;
}

void Algorithm::setGamma(const double gam)
{
    gamma = gam;
}

double Algorithm::getGamma()const
{
    return gamma;
}
