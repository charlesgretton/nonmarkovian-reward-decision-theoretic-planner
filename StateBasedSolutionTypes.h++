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

#ifndef STATE_BASED_SOLUTION_TYPES
#define STATE_BASED_SOLUTION_TYPES

#include<mtl/matrix.h>
#include"mtl/mtl.h"
#include<mtl/utils.h>
#include"mtl/linalg_vec.h"

#include<mtl/lu.h>
#include<math.h>

#include"SpecificationTypes.h++"

namespace MDP
{
    template<class ExplicitAlgorithm> class LAO;
    
    /*For state based solution methods the dynamics of the system are
      encapsulated in the following matrix.*/
    typedef mtl::matrix<double,
        mtl::rectangle<>,
        mtl::compressed<>,
        mtl::row_major >::type ActionMatrix;
    
    /*Vector of values.*/
    typedef mtl::dense1D<double> ValueVector;
    
    /*Mapping from action names to action matrices.*/
    typedef std::map<action, ActionMatrix> ActionMatrices;
    
    typedef mtl::matrix<double,
        mtl::rectangle<>,
        mtl::dense<>,
        mtl::row_major>::type Matrix;
    
    typedef mtl::matrix<double,
        mtl::diagonal<>,
        mtl::packed<>,
        mtl::row_major>::type IdentMat;
    
    typedef std::map<action, Matrix> DenseActionMatrices; 
}

#endif
