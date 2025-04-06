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
 * This software system facilitates planning in completely observable
 * stochastic domains, where dynamics are modelled as a stochastic
 * automata and reward is non--Markovian. The system is implemented
 * using a range of experimental algorithms and frameworks (see
 * \module{Spudd}, \module{LAO}, etc\ldots), thus also facilitates
 * comparative experimentation in the field of planning.
 *
 * Problem domain reward is specified by a user in either
 * $\$FLTL$\cite{sylvie}(Future Linear Temporal Logic with Rewards) or
 * $PLTL$\cite{bacchus96}(Past Linear Temporal Logic).
 *
 * \paragraph{:Interface:}
 *
 * Information concerning the user interface is provided in the
 * documentation for \module{parser}, \module{commandParser} and
 * \module{CommandInterpreter}.
 **/

#include"Registry.h++"
#include"CommandInterpreter.h++"
#include"common.h++"

#include"SpuddWrapper.h++"
#include"StructuredPLTLtranslatorWrapper.h++"
#include"ParserWrapper.h++"
#include"WallTimer.h++"
#include"CPUtimer.h++"
#include"MemoryMonitor.h++"
#include"DomainInspector.h++"
#include"RandomDomainWrapper.h++"
#include"AutomaticConstraintGeneratorWrapper.h++"
#include"StructuredSolutionWrapper.h++"
#include"StateBasedSolutionWrapper.h++"

using namespace MDP;

// no-export

#include"main.h++"

int main(int argc, char **argv)
{
    ParserWrapper* pw = new ParserWrapper();

    SpuddWrapper* sw = new SpuddWrapper();
    
    WallTimer* wt = new WallTimer();
    CPUtimer* ct = new CPUtimer();
    MemoryMonitor* mm = new MemoryMonitor();
    RandomDomainWrapper* rdw = new RandomDomainWrapper();
    AutomaticConstraintGeneratorWrapper* acgw = new AutomaticConstraintGeneratorWrapper();
        
    StructuredPLTLtranslatorWrapper* strucPLTLwrap =
        new StructuredPLTLtranslatorWrapper();
    DomainInspector* di = new DomainInspector();

    CommandInterpreter* ci = CommandInterpreter::getInstance();

    /*Interface to state based expansion and solution algorithms.*/
    StateBasedSolutionWrapper* stateBasedSolutionWrapper
        = new StateBasedSolutionWrapper();

    StructuredSolutionWrapper* structuredSolutionWrapper
        = new StructuredSolutionWrapper();
    
    if (argc == 2) {
        ci->loadFile(argv[1]);
    }
    ci->start();

    delete acgw;
    delete di;
    delete rdw;
    delete mm;
    delete ct;
    delete wt;
    delete sw;
    delete strucPLTLwrap;
    delete pw;

    delete structuredSolutionWrapper;
    delete stateBasedSolutionWrapper;

    CommandInterpreter::destroyInstance();
    Registry::destroyInstance();

    return 0;
}

