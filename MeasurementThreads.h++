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
 * This module contains the implementation of accounting
 * threads\footnote{Or interval threads, because they report
 * accounting information at intervals.} (see \module{Thread} and
 * \module{StateBasedSolutionWrapper}).
 * */

#ifndef MEASUREMENT_THREAD
#define MEASUREMENT_THREAD

#include"Thread.h++"
#include"domainSpecification_Anytime_or_Explicit.h++"
#include"Algorithm.h++"

namespace MDP
{
    /*This measurement thread measures the memory usage of the
     *\member{explicitDomSpec} which is an
     *\class{explicitDomainSpecification}. This measurement thread also
     *provides a \member{getPeak()} query which provides the peak
     *memory usage of the \member{explicitDomSpec}.*/
    class ExpansionMemory : public Thread
    {
    public:
        /*Construct a memory measurement thread, where the first
         *argument is a delay in seconds between reportage and the
         *second argument the domain which this thread shall
         *measure.*/
        ExpansionMemory(int, explicitDomainSpecification*);
        
        ~ExpansionMemory();
        
        /*Report \member{explicitDomSpec} memory usage to the output
         *stream.*/
        static void execute();
        
        /*An \class{explicitDomainSpecification} which this is
         *measuring the memory usage of.*/
        static explicitDomainSpecification* explicitDomSpec;

        /*Largest memory usage to date.*/
        unsigned int getPeak()const;
        
        /*Largest memory usage to date.*/
        static unsigned int peak;
    };

    /*This measurement thread measures the state size (number of
     *states) of the \member{explicitDomSpec} which is an
     *\class{explicitDomainSpecification}.*/
    class ExpansionStates : public Thread
    {
    public:
        /*Construct a state size measurement thread where, the first
         *argument is a delay in seconds between reportage and the
         *second argument the domain which this thread shall measure.*/
        ExpansionStates(int, explicitDomainSpecification*);
        
        ~ExpansionStates();
        
        /*Report \member{explicitDomSpec} state size to the output
         *stream.*/
        static void execute();
        
        /*An \class{explicitDomainSpecification} which this is
         *measuring the memory usage of.*/
        static explicitDomainSpecification* explicitDomSpec;
    };

    /*This measurement thread measures the value of the
     *\member{algorithm} policy. This is achieved by polling the
     *\member{Algorithm}s \member{Algorithm.getError()}.*/
    class PolicyValue : public Thread
    {
    public:
        /*Construct a policy measurement thread where, the first
         *argument is a delay in seconds between reportage and the
         *second argument the \class{Algorithm} which this thread
         *shall measure.*/
        PolicyValue(int, Algorithm*);
        
        ~PolicyValue();
        
        /*Report policy error to the output stream.*/
        static void execute();
        
        /*An \class{explicitDomainSpecification} which this is
         *measuring the memory usage of.*/
        static Algorithm* algorithm;
    };
}
#endif
