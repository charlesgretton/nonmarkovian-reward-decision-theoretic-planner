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
 * This module contains implementations of all \class{formula}
 * derivative equality test methods. The equality tests are based on
 * syntactic equivalence of formulae in their negation normal forms,
 * in the sense that:
 *
 * a and b == a and b
 *
 * Also considered is the associativity of the binary operators.
 *
 * \textbf{NOTE:} This module does \textbf{not} take validity into
 * account. Thus we have that:
 *
 * fbx( p imp q) imp (fbx p imp fbx q) == tt
 *
 * evaluates to false.
 *  */

#include"formula.h++"

using namespace Formula;

bool formula::operator==(formula& f) const
{
    if(this == &f)/*Formulae at the same address are equal.*/
        return true;
    
    /*If either of the comparison formulae are not negation normal*/
    if(!(this->isNegNormal()) || !(f.isNegNormal()))
	{
	    /*Are the formulae equal?*/
	    bool equal;

	    formula *tmp1 = this->negNormalise();
	    formula *tmp2 = f.negNormalise();
	    
	    equal = (*tmp1)==(*tmp2);

	    delete tmp1;
	    delete tmp2;
	    
	    return equal;/*return equality result*/
	}

    return false;
}

bool formula::operator==(formula* f) const
{
    return this->operator==(*f);
}

bool formula::operator!=(formula& f) const
{
    return !(this->operator==(f));
}

bool formula::operator!=(formula* f) const
{
    return this->operator!=(*f);
}        

bool unary::operator==(formula& f) const
{
    unary *tmp = dynamic_cast<unary*>(&f);

    if(0 == tmp)
        return false;

    return (*(this->f)) == (*tmp)[0];
}

bool summaryUnary::operator==(formula& f) const
{
    summaryUnary *tmp = dynamic_cast<summaryUnary*>(&f);

    if(0 == tmp)
        return false;
    
    return unary::operator==(f)   //(*(this->f)) == (*tmp)[0]
        && ( tmp->depth == this->depth )
        && ( tmp->MIN_DEPTH == this->MIN_DEPTH );
}

/*Argument formula is cast to parent $binary$. Boolean test on
 *subformula equality of argument formula.*/
bool binary::operator==(formula& f) const
{
    binary *tmp = dynamic_cast<binary*>(&f);

    if(0 == tmp)
        return false;

    return ((*l) == (*tmp)[0] && (*r) == (*tmp)[1]);
}

bool binaryCommutative::operator==(formula& f) const
{
    binaryCommutative *tmp = dynamic_cast<binaryCommutative*>(&f);

    if(0 == tmp)
        return false;

    return ((*l) == (*tmp)[1] && (*r) == (*tmp)[0]) 
        || (binary::operator==(f));
}

bool PCformula::operator==(formula& f) const
{
    return (0 != dynamic_cast<PCformula*>(&f));
}

bool literal::operator==(formula& f) const
{
    literal *tmp = dynamic_cast<literal*>(&f);
    
    if(0 == tmp)
        return false;
    
    return (tmp->getId() == id);
}

bool assLiteral::operator==(formula& f) const
{
    assLiteral *tmp = dynamic_cast<assLiteral*>(&f);
    
    if(0 == tmp)
        return false;
    
    return (tmp->getAssignment() == assignment);
}

bool conj::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PCformula::operator==(f))
	{
	    conj *tmp = dynamic_cast<conj*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binaryCommutative::operator==(f);
	}

    return false;
}

bool disj::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PCformula::operator==(f))
	{
	    disj *tmp = dynamic_cast<disj*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binaryCommutative::operator==(f);
	}

    return false;
}

bool iff::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PCformula::operator==(f))
	{
	    iff *tmp = dynamic_cast<iff*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binaryCommutative::operator==(f);
	}

    return false;
}

bool imp::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PCformula::operator==(f))
	{
	    imp *tmp = dynamic_cast<imp*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binary::operator==(f);
	}

    return false;
}

bool lnot::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PCformula::operator==(f))
	{
	    lnot *tmp = dynamic_cast<lnot*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

bool FLTLformula::operator==(formula& f) const
{
    return (0 != dynamic_cast<FLTLformula*>(&f));
}

bool nxt::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    nxt *tmp = dynamic_cast<nxt*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

bool nxtDisj::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    nxtDisj *tmp = dynamic_cast<nxtDisj*>(&f);
      
	    if(0 == tmp)
            return false;

	    return summaryUnary::operator==(f);
	}

    return false;
}

bool nxtConj::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    nxtConj *tmp = dynamic_cast<nxtConj*>(&f);
      
	    if(0 == tmp)
            return false;

	    return summaryUnary::operator==(f);
	}

    return false;
}

bool nxtNest::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    nxtNest *tmp = dynamic_cast<nxtNest*>(&f);
      
	    if(0 == tmp)
            return false;

	    return summaryUnary::operator==(f);
	}

    return false;
}

bool fut::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    fut *tmp = dynamic_cast<fut*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binary::operator==(f);
	}

    return false;
}

bool strFut::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    strFut *tmp = dynamic_cast<strFut*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binary::operator==(f);
	}

    return false;
}

bool fbx::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    fbx *tmp = dynamic_cast<fbx*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

bool fdi::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    fdi *tmp = dynamic_cast<fdi*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

bool dollars::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && FLTLformula::operator==(f))
	{
	    dollars *tmp = dynamic_cast<dollars*>(&f);
      
	    if(0 == tmp)
            return false;

	    return true;
	}

    return false;
}

/*PLTL*/

bool PLTLformula::operator==(formula& f) const
{
    return (0 != dynamic_cast<PLTLformula*>(&f));
}

bool startStateProposition::operator==(formula& f) const
{
    startStateProposition *tmp =
        dynamic_cast<startStateProposition*>(&f);
    
    if(0 == tmp)
        return false;
    else
        return true;
}


bool prv::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    prv *tmp = dynamic_cast<prv*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

bool prvDisj::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    prvDisj *tmp = dynamic_cast<prvDisj*>(&f);
      
	    if(0 == tmp)
            return false;

	    return summaryUnary::operator==(f);
	}

    return false;
}

bool prvConj::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    prvConj *tmp = dynamic_cast<prvConj*>(&f);
      
	    if(0 == tmp)
            return false;

	    return summaryUnary::operator==(f);
	}

    return false;
}

bool prvNest::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    prvNest *tmp = dynamic_cast<prvNest*>(&f);
      
	    if(0 == tmp)
            return false;

	    return summaryUnary::operator==(f);
	}

    return false;
}

bool snc::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    snc *tmp = dynamic_cast<snc*>(&f);
      
	    if(0 == tmp)
            return false;

	    return binary::operator==(f);
	}

    return false;
}

bool pbx::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    pbx *tmp = dynamic_cast<pbx*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

bool pdi::operator==(formula& f) const
{
    if(formula::operator==(f))
        return true;

    if(this->isNegNormal()
       && f.isNegNormal()
       && PLTLformula::operator==(f))
	{
	    pdi *tmp = dynamic_cast<pdi*>(&f);
      
	    if(0 == tmp)
            return false;

	    return unary::operator==(f);
	}

    return false;
}

