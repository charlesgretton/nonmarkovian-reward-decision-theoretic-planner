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

#include"MeasurementThreads.h++"

using namespace MDP;

/*Ensure all static variables are in scope.*/
extern explicitDomainSpecification* ExpansionMemory::explicitDomSpec;
extern unsigned int ExpansionMemory::peak;
extern explicitDomainSpecification* ExpansionStates::explicitDomSpec;
extern Algorithm* PolicyValue::algorithm;

/*******************************************************ExpansionMemory*/

ExpansionMemory::ExpansionMemory(int delay, explicitDomainSpecification* eds)
    :Thread(delay)
{
    explicitDomSpec = eds;
    peak = 0;
    
    stop_sleep_function tmp = new pair<int*, sleep_function >
        (&alive, sleep_function(delay, &execute));
    
    f_arg = static_cast<void*>(tmp);
}

ExpansionMemory::~ExpansionMemory()
{
    delete static_cast<stop_sleep_function>(f_arg);
}

void ExpansionMemory::execute()
{
    cout<<"Domain Size :: "
        <<explicitDomSpec->memory() / 1000
        <<" Kb.\n";

    /*Keep track of the largest memory usage, $peak$, to date.*/
    if((explicitDomSpec->memory() / 1000) > peak)
        peak = explicitDomSpec->memory() / 1000;
}

unsigned int ExpansionMemory::getPeak()const
{
    return peak;
}


/*******************************************************ExpansionMemory*/

ExpansionStates::ExpansionStates(int delay, explicitDomainSpecification* eds)
    :Thread(delay)
{
    explicitDomSpec = eds;
    
    stop_sleep_function tmp = new pair<int*, sleep_function >
        (&alive, sleep_function(delay, &execute));
    
    f_arg = static_cast<void*>(tmp);
}

ExpansionStates::~ExpansionStates()
{
    delete static_cast<stop_sleep_function>(f_arg);
}

void ExpansionStates::execute()
{
    cout<<"Domain size :: "
        <<explicitDomSpec->numberOfStates()
        <<" e-states.\n";
}

/*******************************************************PolicyValue*/

PolicyValue::PolicyValue(int delay, Algorithm* alg)
    :Thread(delay)
{
    algorithm = alg;
    
    stop_sleep_function tmp = new pair<int*, sleep_function >
        (&alive, sleep_function(delay, &execute));
    
    f_arg = static_cast<void*>(tmp);
}

PolicyValue::~PolicyValue()
{
    delete static_cast<stop_sleep_function>(f_arg);
}

void PolicyValue::execute()
{
    cout<<"The supremum norm of the current less the previous value vector is :: "
        <<algorithm->getError()
        <<"\n";

}
