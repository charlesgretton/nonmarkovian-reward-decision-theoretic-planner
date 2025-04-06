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

#include"formulaUtilities.h++"

AccumulateFormula::AccumulateFormula()
{
    result = new RewardSpecification();
}

AccumulateFormula::AccumulateFormula(AccumulateFormula& accumulateFormula)
{
    result = accumulateFormula.result;
}

AccumulateFormula::AccumulateFormula(RewardSpecification* rewSpec)
{
    result = rewSpec;
}

void AccumulateFormula::operator()(double reward,
                                   formula const* form)
{
    /*Construct the key.*/
    string* key = form->print();

    RewardSpecification *tmp =
        result->addSpecComponent(*key, reward, form->copy());
    
    /*If a change was made to the reward specification.*/
    if(0 != tmp)
	{
	    /*Remove the old reward type.*/
	    if(result != tmp)
        {
            delete result;
            
            /*Keep the new reward type.*/
            result = tmp;
        }
	}
    
    delete key;
}

RewardSpecification* AccumulateFormula::getSpecification()
{
    return result;
}
