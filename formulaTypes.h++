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

#ifndef FORMULA_TYPES
#define FORMULA_TYPES

namespace Formula
{
    /*Index type for operator types. For instance Classes
      \class{binary} and \class{unary}.*/
    typedef unsigned short OperatorIndexType;
    
    class unary;
    class binary;

    class formula;
    class FLTLformu;
    class PLTLformulala;
    class PCformula;

    /*Formula for which the root operator is a PC operator.*/

    class literal;
    class assLiteral;
    class conj;
    class disj;
    class iff;
    class imp;
    class lnot;

    /*Formula for which the root operator is an FLTL operator.*/

    class nxt;
    class nxtDisj;
    class nxtConj;
    class nxtNest;
    class fut;
    class strFut;
    class fbx;
    class fdi;
    class dollars;

  /*Formula for which the root operator is an PLTL operator.*/

    class startStateProposition;
    class prv;
    class prvDisj;
    class prvConj;
    class prvNest;
    class snc;
    class pbx;
    class pdi;
}

#endif
