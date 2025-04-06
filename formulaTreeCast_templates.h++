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

#ifndef FORMULA_TREE_CAST
#define FORMULA_TREE_CAST

namespace Formula
{ 
    template<typename CastTo>
    bool formula_treeCast(formula *f)
    {
        /**/
        binary *binTmp;

        /**/
        unary *unTmp;
        
        /*If the argument formula is of the correct cast?*/
        if(0 != dynamic_cast<CastTo*>(f))
            return true;
        
        /*If the argument formula is a unary and its subformula is of the
          correct cast.*/
        else if(0 != (unTmp = dynamic_cast<unary*>(f)))
            return formula_treeCast<CastTo>(unTmp->operator[](0));
        /*If the argument formula is a binary and its subformula is of the
          correct cast.*/
        else if(0 != (binTmp = dynamic_cast<binary*>(f)))
	    {
            return formula_treeCast<CastTo>(binTmp->operator[](0)) || 
                formula_treeCast<CastTo>(binTmp->operator[](1));
	    }
	
        return false;
    }
}
#endif

