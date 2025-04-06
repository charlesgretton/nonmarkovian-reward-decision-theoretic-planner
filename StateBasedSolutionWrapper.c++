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
 * This module comprises the user interface to the following
 * algorithms. The algorithms are indexed below by their command
 * names.
 *
 * \begin{itemize}
 *
 * \item{\textbf{LAO}:} Interface to the \class{LAO} \class{Algorithm}.
 *
 * \item{\textbf{expand}:} Expands the state space using the
 * \member{explicitDomainSpecification.expandFringe()} of the
 * \member{explicitDomSpec}.
 *
 * \item{\textbf{preprocess}:} The
 * \class{CommandInterpreter::getInstance()}
 * \member{CommandInterpreter.getDomSpec()} must be preprocessed
 * before $expansion$ or a heuristic search driven DP algorithm
 * ($LAO$) is called.
 *
 * \item{\textbf{valIt}:} Interface to the \class{ValueIteration}
 * \class{Algorithm}.
 *
 * \item{\textbf{polIt:}} Interface to the \class{PolicyIteration}
 * \class{Algorithm}.
 *
 * \item{\textbf{simplify}:} Removes all impossible states (including
 * states which lead to an impossibility) from the
 * \member{explicitDomSpec}.
 *
 * \end{itemize}
 *
 * Also included in this module is the user interface to the
 * accounting functionality that is provided for state based solution
 * algorithms (see
 * \method{StateBasedSolutionWrapper::StateBasedSolutionWrapper()}).
 *
 * The implementation of the interface is given in the
 * \class{StateBasedSolutionWrapper}.
 * */

#include"common.h++"
#ifdef HEADER

#include"CommandListener.h++"
#include"SpecificationTypes.h++"
#include"AlgorithmTypes.h++"
#include"Timer.h++"

/*Interval reportage inclusive.*/
#include"MeasurementThreads.h++"

using namespace MDP;

class StateBasedSolutionWrapper : public CommandListener
{
private:
    /*This wrapper provides an interface to search driven
     *\class{PolicyIteration}.*/
    LAO<PolicyIteration>* laoPi;
    
    /*This wrapper provides an interface to search driven
     *\class{ValueIteration}.*/
    LAO<ValueIteration>* laoVi;

    /*This wrapper provides an interface to \class{PolicyIteration}
     *\class{Algorithm}.*/
    PolicyIteration* polIt;
 
    /*This wrapper provides an interface to \class{ValueIteration}
     *\class{Algorithm}.*/
    ValueIteration* valIt;
    
    /*An \class{explicitDomainSpecification} based on the
     *\class{CommandInterpreter}s \member{domSpec}.*/
    explicitDomainSpecification* explicitDomSpec;

    /*Utility for estimating the algorithms execution time.*/
    Timer timer;

    /*Number of iterations of any of the current solution algorithms
     *that have been performed.*/
    unsigned int iterations;

    /*\class{Thread} which accounts for memory usage during NMRDP
     *expansion.*/
    ExpansionMemory* expansionMemory;

    /*\class{Thread} which accounts for domain state quantity during
     *NMRDP expansion.*/
    ExpansionStates* expansionStates;

    /*If this integer has a non--zero value then when a solution
     *algorithm is requested the \member{valueDifference} is
     *executed. This member variable has a dual use as its value is
     *the time interval, in seconds, at which the policy accounting
     *should be executed.*/
    int policyAccounting;
    
    /*\class{Thread} which accounts for policy value during
     *solution algorithm execution.*/
    PolicyValue* valueDifference;

    /*Time that the \member{timer} reported when it was last reset.*/
    double timeCache;

    /*Handle to what this interface believes to be the applications
     *domain specification. If the application specification changes
     *then \ember{domSpec} becomes inconsistent and is updated.*/
    DomainSpecification* domSpec;

    /*Shall we always simplify the domain during
     *expansion?
     *
     *If true, simplification will remove states which are impossible
     *to reach according to control knowledge.*/
    bool alwaysSimplify;
};

#endif
#include"StateBasedSolutionWrapper.h++"
#include"Registry.h++"
#include"CommandInterpreter.h++"
#include"Utils.h++"

#include"domainSpecification_Anytime_or_Explicit.h++"
#include"Preprocessors.h++"
#include"PhaseI.h++"
#include"LAO.h++"
#include"ValueIteration.h++"
#include"PolicyIteration.h++"

#include<sstream>
#include<cstdlib>

using namespace std;

/*Assumptions satisfied through construction include:
 *
 *\begin{itemize}
 *
 *\item No heuristic search algorithm, \member{laoPi} and
 *\member{laoVi}, is configured. These are initialised to $0$.
 *
 *\item The explicit domain specification \member{explicitDomSpec} is
 *not built, thus it is initially always $0$.
 *
 *\item The registry instance \class{Registry::getInstance()} is
 *initialised with the functionality provided by this
 *\class{CommandListener}.
 *
 *\item \member{polIt} and \member{valIt} are both initialised to $0$.
 *
 *\item All pointers to accounting threads are initialised to $0$.
 *
 *\item \member{policyAccounting} is initialised to $false == 0$.
 *
 *\item the \member{timeCache} is zero initialised.
 *
 *\item The \member{domSpec} is zero initialised.
 *
 *\item By default, the domain is not simplified during expansion.
 *\end{itemize}
 **/
PUBLIC StateBasedSolutionWrapper::StateBasedSolutionWrapper()
    :laoPi(0),
     laoVi(0),
     polIt(0),
     valIt(0),
     explicitDomSpec(0),
     expansionMemory(0),
     expansionStates(0),
     policyAccounting(0),
     valueDifference(0),
     timeCache(0),
     domSpec(0),
     alwaysSimplify(false)
{
    Registry *reg = Registry::getInstance();
    
    reg->setFunction("LAO", this, 3);
    reg->setFunction("LAO", this, 2);
    reg->setFunction("expand", this, 0);
    reg->setFunction("preprocess", this, 1);
    reg->setFunction("valIt", this, 2);
    reg->setFunction("polIt", this, 1);
    reg->setFunction("simplify", this, 0);
    reg->setFunction("alwaysSimplify", this, 1);

    reg->setFunction("valueDifferenceAtInterval", this, 1);
    reg->setFunction("valueDifference", this, 0);
    reg->setFunction("printDomain", this, 1);
    reg->setFunction("iterationCount", this, 0);
    reg->setFunction("iterationCount", this, 1);
    reg->setFunction("longestLabel", this, 0);
    reg->setFunction("averageLabelSize", this, 0);
    reg->setFunction("domainStateSize", this, 0);
    reg->setFunction("domainMemorySize", this, 0);
    reg->setFunction("expansionMemory", this, 1);
    reg->setFunction("expansionStates", this, 1);
    reg->setFunction("expansionPeak", this, 0);
    reg->setFunction("reportTime", this, 0);
    reg->setFunction("rewardDuplication", this, 0);
    reg->setFunction("getPolicy", this, 0);
    reg->setFunction("clear", this, 0);
}

/*Members \member{polIt}, \member{valIt}, \member{laoPi},
 *\member{laoVi}, \member{explicitDomSpec} are deleted. Accounting
 *threads are also deleted. All the interface functionality is
 *\function{unregister}ed from the \class{Registry::getInstance()}.*/
PUBLIC StateBasedSolutionWrapper::~StateBasedSolutionWrapper()
{
    Registry *reg = Registry::getInstance();
    
    reg->unregister("LAO", this);
    reg->unregister("LAO", this);
    reg->unregister("expand", this);
    reg->unregister("preprocess", this);
    reg->unregister("valIt", this);
    reg->unregister("polIt", this);
    reg->unregister("simplify", this);
    reg->unregister("alwaysSimplify", this);

    reg->unregister("valueDifferenceAtInterval", this);
    reg->unregister("valueDifference", this);
    reg->unregister("printDomain", this);
    reg->unregister("iterationCount", this);
    reg->unregister("iterationCount", this);
    reg->unregister("longestLabel", this);
    reg->unregister("averageLabelSize", this);
    reg->unregister("domainStateSize", this);
    reg->unregister("domainMemorySize", this);
    reg->unregister("expansionMemory", this);
    reg->unregister("expansionStates", this);
    reg->unregister("expansionPeak", this);
    reg->unregister("reportTime", this);
    reg->unregister("rewardDuplication", this);
    reg->unregister("getPolicy", this);
    reg->unregister("clear", this);
    
    /*Both Bellman backup algorithms are deleted.*/
    /*Delete any configured heuristic solution algorithms.*/
    clearSolutionAlgorithms();
    
    /*Delete the expanded domain specification. This shall not
      destruct the specification from which it was created (see
      \method{commandIssued(\ldots)}).*/
    delete explicitDomSpec;

    /*We no longer require the thread objects.*/
    clearMeasurementThreads();
}

/*Delete and nullify any wrapper solution algorithms.*/
PRIVATE void StateBasedSolutionWrapper::clearSolutionAlgorithms()
{
    delete valIt;
    delete polIt;
    delete laoVi;
    delete laoPi;
    
    valIt = 0;
    polIt = 0;
    laoVi = 0;
    laoPi = 0;
}

/*Delete and nullify any measurment thread objects.*/
PRIVATE void StateBasedSolutionWrapper::clearMeasurementThreads()
{
    delete expansionMemory;
    delete expansionStates;
    delete valueDifference;

    expansionMemory = 0;
    expansionStates = 0;
    valueDifference = 0; 
}


/*See \parent{CommandListener}.*/
bool StateBasedSolutionWrapper::commandIteration(const string &command)
{
    /*Has the algorithm completed its task?*/
    bool stoppingCondition = true;

    /*Start timing\footnote{we do not require flawless accuracy}.*/
    startTimer();
        
    if (command == "LAO")
    {
        /*The \member{valueDifference} is only alive and non--null when a
          solution algorithm is present/requested.*/
        if(0 != valueDifference && !valueDifference->isAlive())
            valueDifference->Create();
    
        /*Start memory and state accounting threads if this is not
         *already done.*/
        if(0 != expansionMemory && !expansionMemory->isAlive())
            expansionMemory->Create();
        if(0 != expansionStates && !expansionStates->isAlive())
            expansionStates->Create();
        
        if(0 != laoVi)
            stoppingCondition = (*laoVi)();
        else if(0 != laoPi)
            stoppingCondition =  (*laoPi)();

        /*Destroy all accounting threads if expansion and solution is
         *complete.*/
        if(0 != expansionMemory && stoppingCondition)
        {
            expansionMemory->Destroy();
            expansionMemory = 0;
        }
        if(0 != expansionStates && stoppingCondition)
        {
            expansionStates->Destroy();
            expansionStates = 0;
        }
        
        /*Ensure that the \member{valueDifference} does not recommence until
          the user requests this.*/
        if(0 != valueDifference && stoppingCondition)
        {
            valueDifference->Destroy();
            valueDifference = 0;
        }
    }
    else if(command == "expand" && 0 != explicitDomSpec)
    {
        /*Start memory and state accounting threads if this is not
          already done.*/
        if(0 != expansionMemory && !expansionMemory->isAlive())
            expansionMemory->Create();
        if(0 != expansionStates && !expansionStates->isAlive())
            expansionStates->Create();
        
        stoppingCondition =  explicitDomSpec->expandFringe();

        /*If we are required to always simplify the domain.*/
        if(alwaysSimplify)
            explicitDomSpec->removeImpossibleStates();
        
        /*Destroy all accounting threads if expansion is complete.*/
        if(0 != expansionMemory && stoppingCondition)
        {
            expansionMemory->Destroy();
            expansionMemory = 0;
        }
        if(0 != expansionStates && stoppingCondition)
        {
            expansionStates->Destroy();
            expansionStates = 0;
        }
            
    }
    else if(command == "simplify" && 0 != explicitDomSpec)
    {
        explicitDomSpec->removeImpossibleStates();
    }
    else if(command == "polIt" && 0 != polIt)
    {
        /*The \member{valueDifference} is only alive and non--null when a
          solution algorithm is present/requested.*/
        if(0 != valueDifference && !valueDifference->isAlive())
            valueDifference->Create();
        
        stoppingCondition = (*polIt)();
        
        /*Ensure that the \member{valueDifference} does not recommence until
          the user requests this.*/
        if(0 != valueDifference && stoppingCondition)
        {
            valueDifference->Destroy();
            valueDifference = 0;
        }
    }
    else if(command == "valIt" && 0 != valIt)
    {
        /*The \member{valueDifference} is only alive and non--null when a
          solution algorithm is present/requested.*/
        if(0 != valueDifference && !valueDifference->isAlive())
            valueDifference->Create();
        
        stoppingCondition = (*valIt)();
        
        /*Ensure that the \member{valueDifference} does not recommence until
          the user requests this.*/
        if(0 != valueDifference && stoppingCondition)
        {
            valueDifference->Destroy();
            valueDifference = 0;
        }
    }
    stopTimer();

    
    /*We have done one more iteration of the solution algorithm.*/
    iterations++;
    return stoppingCondition;
}

PRIVATE void StateBasedSolutionWrapper::configureExplicitDomainSpecification()
{
    if(0 != explicitDomSpec)
        delete explicitDomSpec;
    explicitDomSpec = 0;
    
    /*Get the interpreters domain specification.*/
    CommandInterpreter* ci = CommandInterpreter::getInstance();
    ci->getDomSpec()->getRewardSpecification()->normalise();
    ci->getDomSpec()->getRewardSpecification()->simplify();
        //ci->getDomSpec()->getRewardSpecification()->checkDuplication();
    domSpec = ci->getDomSpec();
    explicitDomSpec = new explicitDomainSpecification(*ci->getDomSpec());
}

/*See \parent{CommandListener}.*/
void StateBasedSolutionWrapper::commandIssued(const string &command, const vector<string> &parameters)
{
    /*Assume the command is not going to be accepted.*/
    acceptLastCommand = false;
    
    ostringstream streamCommandResult;
    commandResult = "";

    /*Where a command is issued the timer is reset.*/
    resetTimer();

    /*Commands*/
    
    /*If the explicit domain specification has not yet been
      constructed.*/
    if(0 == explicitDomSpec)
        configureExplicitDomainSpecification();

    if (command == "simplify" || command == "expand")
    {
        acceptLastCommand = true;
    }
    else if (command == "LAO")
    {
        clearSolutionAlgorithms();

        double gamma = atof(parameters[1].c_str());
        double epsilon;
        if(2 <= parameters.size())
            epsilon = atof(parameters[2].c_str());
        else
            epsilon = 1.0;
            
        if("valIt" == parameters[0])
        {
            laoVi = new LAO<ValueIteration>(*explicitDomSpec, gamma, epsilon, alwaysSimplify);
            acceptLastCommand = true;
        }
        else if("polIt" == parameters[0])
        {
            laoPi = new LAO<PolicyIteration>(*explicitDomSpec, gamma, epsilon, alwaysSimplify);
            acceptLastCommand = true;
        }
    }
    else if(command == "rewardDuplication")
    {
        streamCommandResult<<explicitDomSpec->getRewardSpecification()
            ->checkDuplication();
        acceptLastCommand = true;
    }
    else if(command == "preprocess")/*Execute this step*/
    {
        /*Algorithm used for preprocessing the \class{CommandInterpreter}s
         *\member{domSpec} in building the \member{explicitDomSpec}.*/
        Preprocessor* preprocessor = 0;
        PhaseI* phaseI = 0;
        
        if(parameters[0] == "pltl"
           && !dynamic_cast<FLTLrewardSpecification*>
           (explicitDomSpec->getRewardSpecification()))
        {
            phaseI = new Subformulas();
            
            /*Make the preprocessor.*/
            preprocessor = new PhaseIPreprocessor(phaseI);
            acceptLastCommand = true;
        }
        else if(parameters[0] == "mPltl"
                && !dynamic_cast<FLTLrewardSpecification*>
                (explicitDomSpec->getRewardSpecification()))
        {
            phaseI = new Minimal();
            
            /*Make the preprocessor.*/
            preprocessor = new MinimalPhaseIPreprocessor(phaseI);
            acceptLastCommand = true;
        } else if((parameters[0] == "" || parameters[0] == "fltl")
                  && dynamic_cast<FLTLrewardSpecification*>
                  (explicitDomSpec->getRewardSpecification())){ 
            preprocessor = new Preprocessor();
            acceptLastCommand = true;
        }

        if(!acceptLastCommand && (parameters[0] == "" ||
                                  parameters[0] == "fltl" ||
                                  parameters[0] == "pltl" ||
                                  parameters[0] == "mPltl"))
        {
            cerr<<"Preprocessor failed due to incompatible domain."<<endl;
        }
        
        /*If a preprocessor was configured.*/
        if(acceptLastCommand)
        {
            /*Report domain memory during preprocessing if requested.*/
            if(0 != expansionMemory && !expansionMemory->isAlive())
                expansionMemory->Create();
            startTimer();
            explicitDomSpec->preprocess(*preprocessor);
            stopTimer();
            
            /*Cease reporting domain memory usage.*/
            if(0 != expansionMemory && expansionMemory->isAlive())
                expansionMemory->Destroy();
            
            delete preprocessor;
            delete phaseI;
        }
    }
    else if(command == "polIt")
    {
        clearSolutionAlgorithms();

        double gamma = atof(parameters[0].c_str());
        polIt = new PolicyIteration(*explicitDomSpec, gamma, 0.1);
        acceptLastCommand = true;
    }
    else if(command == "valIt")
    {
        clearSolutionAlgorithms();
        
        double gamma = atof(parameters[0].c_str());
        double epsilon = atof(parameters[1].c_str());
        valIt = new ValueIteration(*explicitDomSpec, gamma, epsilon);
        acceptLastCommand = true;
    }
    else if(command == "alwaysSimplify")/*Simplify during expansion?*/
    {
        if("true" == parameters[0])
            alwaysSimplify = true;
        else
            alwaysSimplify = false;
        acceptLastCommand = true;
    }
    
    /*Queries*/
    else if(command == "printDomain")
    {
        if(parameters[0] == "NMRDP")
        {    
            cout<<"The current domain is as follows :: \n";
            dynamic_cast<DomainSpecification*>(explicitDomSpec)
                ->printDomain();
            acceptLastCommand = true;
        }
        else if(parameters[0] == "" || parameters[0] == "XMDP"
                || parameters[0] == "MDP")
        {        
            streamCommandResult<<"The current explicit domain is as follows :: \n";
            streamCommandResult<<explicitDomSpec->toString(true)<<endl;
            acceptLastCommand = true;
        }
    }
    else if(command == "valueDifferenceAtInterval")
    {
        /*Trigger policy accounting.*/
        policyAccounting = atoi(parameters[0].c_str());
        acceptLastCommand = true;
    }
    else if(command == "valueDifference")
    {
        streamCommandResult<<"The supremum norm of the current less the previous value vector is :: ";
            
        if(laoPi != 0)streamCommandResult<<laoPi->getError()<<endl;
        
        else if(laoVi != 0)streamCommandResult<<laoVi->getError()<<endl;
        
        else if(valIt != 0)streamCommandResult<<valIt->getError()<<endl;
        
        else if(polIt != 0)streamCommandResult<<polIt->getError()<<endl;
        
        streamCommandResult<<"\n";
        acceptLastCommand = true;
    }
    else if(command == "iterationCount")
    {
        if(parameters.size() == 0 || parameters[0] == "")
        {    
            streamCommandResult<<"Iteration number: "<<iterations<<endl;
            acceptLastCommand = true;
        }
    }
    else if(command == "longestLabel")
    {
        streamCommandResult<<"The longest label is comprised of "
             <<explicitDomSpec->longestLabel()
             <<" operators.\n";
        acceptLastCommand = true;
    }
    else if(command == "averageLabelSize")
    {
        streamCommandResult<<"The average label is comprised of "
                           <<explicitDomSpec->averageLabelSize()
                           <<" operators.\n";
        acceptLastCommand = true;
    }
    else if(command == "domainMemorySize")
    {
         streamCommandResult<<"The domain is currently "
                            <<explicitDomSpec->memory() / 1000
                            <<" Kb in size.\n";
         acceptLastCommand = true;
    }
    else if(command == "domainStateSize")
    {
         streamCommandResult<<"The domain is currently comprised of "
                            <<explicitDomSpec->numberOfStates()
                            <<" e-states.\n";
         acceptLastCommand = true;
    }
    else if(command == "expansionMemory")
    {
        /*Inform the threads of the domain they shall have to watch.*/
        if(0 != expansionMemory)
            delete expansionMemory;
        
        expansionMemory = new ExpansionMemory(atoi(parameters[0].c_str())
                                              ,explicitDomSpec);
        acceptLastCommand = true;
    }
    else if(command == "expansionStates")
    {
        /*Inform the threads of the domain they shall have to watch.*/
        if(0 != expansionStates)
            delete expansionStates;
        
        expansionStates = new ExpansionStates(atoi(parameters[0].c_str())
                                              ,explicitDomSpec);
        acceptLastCommand = true;
    }
    else if (command == "expansionPeak")
    {
        if(0 == expansionMemory)
            streamCommandResult<<"\"expansionMemory\" must be or have been active\n"
                               <<"for peak memory figures to be available.\n";
        else
            streamCommandResult<<"Peak memory usage was "
                               <<expansionMemory->getPeak()
                               <<" Kb.\n";
        acceptLastCommand = true;
    }
    else if(command == "reportTime")
    {
        streamCommandResult<<"Time spent executing the last command was approximately "
                           <<timeCache
                           <<" seconds.\n";
        acceptLastCommand = true;
    }
    else if(command == "getPolicy")
    {
        streamCommandResult<<explicitDomSpec->policyToString();
        acceptLastCommand = true;
    }
    else if(command == "clear")
    {
        clearSolutionAlgorithms();
        
        delete  explicitDomSpec;
        explicitDomSpec = 0;
        
        clearMeasurementThreads();
        
        domSpec = 0;
        
        acceptLastCommand = true;
    }
    
    /*If policy accounting has been requested.*/
    if(0 != policyAccounting)
    {
        /*Inform the threads of the domain they shall have to watch.*/
        if(0 != valueDifference)
        {
            delete valueDifference;
            valueDifference = 0;
        }
        
        Algorithm* algorithm = 0;
        if(laoPi != 0)algorithm = dynamic_cast<Algorithm*>(laoPi);
        else if(laoVi != 0)algorithm = dynamic_cast<Algorithm*>(laoVi);
        else if(valIt != 0)algorithm = dynamic_cast<Algorithm*>(valIt);
        else if(polIt != 0)algorithm = dynamic_cast<Algorithm*>(polIt);   
        
        /*If an algorithm has been requested.*/
        if(0 != algorithm)
            valueDifference = new PolicyValue(policyAccounting
                                          ,algorithm);
    }

    commandResult += streamCommandResult.str();
    
    /*Where a command is issued no iterations of the desired algorithm
      have been performed.*/
    iterations = 0;
}

/*See \parent{CommandListener}.*/
string StateBasedSolutionWrapper::shortHelp(const string &command) const
{
    string result = "";
    
    if(command == "LAO")
        result = shortHelpLine(command, string("Executes the LAO* algorithm on the current domain.\n"));
    else if(command == "expand")
        result = shortHelpLine(command, "Expands the NMRDP that was last \"preprocessed\" into\n")
            + shortHelpLine("", "its corresponding XMDP.\n");
    else if(command == "preprocess")
        result = shortHelpLine(command, string("Preprocess the NMRDP. Executing this command prepares the domain\n")
            + shortHelpLine("", "for state space expansion.\n"));
    else if(command == "valIt")
        result = shortHelpLine(command, string("Execute value iteration on the expanded domain.\n"));
    else if(command == "polIt")
        result = shortHelpLine(command, string("Execute policy iteration on the expanded domain.\n"));
    else if(command == "simplify")
        result = shortHelpLine(command, string("Simplify the expanded domain. This removes states which are\n")
            + shortHelpLine("", "impossible and predecessors of those which are impossible.\n"));
    else if(command == "alwaysSimplify")
        result = shortHelpLine(command, string("Simplify the expanded domain during expansion. By executing this\n")
            + shortHelpLine("", "command before the \"expand\" command, expansion shall consider\n")
            + shortHelpLine("", "control knowledge (currently $FLTL only).\n"));
    else if(command == "valueDifferenceAtInterval")
        result = shortHelpLine(command, string("Every $n$ second interval, where $n$ is the integer argument\n")
            + shortHelpLine("", "to this command, report the supremum norm of the current less the\n")
            + shortHelpLine("", "previous value vector. n = 0 turns accounting off.\n"));
    else if(command == "valueDifference")
        result = shortHelpLine(command, string("Reports the supremum norm between the current less the previous\n")
            + shortHelpLine("", "value vector.\n"));
    else if(command == "printDomain")
        result = shortHelpLine(command, string("Returns a textual description of the current XMDP or NMRDP domain\n")
            + shortHelpLine("", "depending on whether the argument is \"MDP\" or \"NMRDP\" respectively.\n"));
    else if(command == "iterationCount")
        result = shortHelpLine(command, string("Reports the number of iterations of the last executed solution\n")
            + shortHelpLine("", "algorithm.\n"));
    else if(command == "longestLabel")
        result = shortHelpLine(command, string("Reports the operator length of the longest domain label.\n"));
    else if(command == "averageLabelSize")
        result = shortHelpLine(command, string("Reports the average operator length of the domain's labels.\n"));
    else if(command == "domainStateSize")
        result = shortHelpLine(command, string("Reports the number of states in the XMDP.\n"));
    else if(command == "domainMemorySize")
        result = shortHelpLine(command, string("Reports the number of Kb used by the current XMDP."));
    else if(command == "expansionMemory")
        result = shortHelpLine(command, string("Reports the number of Kb used by the current XMDP at $n$\n")
            + shortHelpLine("", "second intervals where $n$ is an unsigned integer argument.\n"));
    else if(command == "expansionStates")
        result = shortHelpLine(command, string("Reports the number of states in the current XMDP at $n$\n")
            + shortHelpLine("", "second intervals where $n$ is an unsigned integer argument.\n"));
    else if(command == "expansionPeak")
        result = shortHelpLine(command, string("Reports the peak memory used during expansion. This command\n")
            + shortHelpLine("", "requires that \"expansionMemory\" was activated.\n"));
    else if(command == "reportTime")
        result = shortHelpLine(command, string("Reports the time taken for the last (state based) command to execute")
                               + shortHelpLine("", "(see startCPUtimer).\n"));
    else if(command == "rewardDuplication")
        result = shortHelpLine(command, string("Are any elements in the reward specification duplicates?\n"));
    else if(command == "getPolicy")
        result = shortHelpLine(command, string("Reports the policy generated by the last executed solution\n")
            + shortHelpLine("", "algorithm.\n"));
    else if(command == "clear")
        result = shortHelpLine(command, string("Any accounting over the old domain is lost. Threads such as\n"))
            +shortHelpLine("", string("those initiated by commands \"expansionMemory\",  \"expansionStates\"\n"))
            +shortHelpLine("", string("and \"valueDifferenceAtInterval\" are deleted, as the domain on which they\n"))
            +shortHelpLine("", string("were accounting has been deleted.\n"));
    
    return result;
}

string StateBasedSolutionWrapper::longHelp(const string &command) const
{
    string result = shortHelp(command) + "\n" + "";
    
    if(command == "preprocess")
        result += Utils::wordWrapString(helpIndentSize, "\t arg1 :: Expansion type, either \"pltl\", \"mPltl\" or "
                                        "\"fltl\" (in the case the domain has an FLTL reward specification).\n\n"
                                        "In the \"pltl\" case, preprocessing generates the set of subformulae of "
                                        "domain reward elements for the purpose of history determination during "
                                        "expansion. In the \"mPltl\" case, preprocessing generates the minimum set "
                                        "of PLTL formula for determining histories during expansion. The null  "
                                        "argument or \"\", means that no preprocessing is achieved, however the  "
                                        "domain is prepared for expansion. This last argument should only be used  "
                                        "in the case where the reward specification is given in FLTL.\n");

    else if(command == "LAO")
        result += Utils::wordWrapString(helpIndentSize, "\t arg1 :: Algorithm used for Bellman backup (valIt, polIt).\n\n"
                                        "\t arg2 :: Discount factor (gamma).\n\n"
                                        "\t arg3 :: Value iteration shall terminate when the computed value "
                                        "function is within $arg3/2$ of the optimal. This argument is optional if "
                                        "not required by the $arg1$.\n\n");
    else if(command == "valIt")
        result += Utils::wordWrapString(helpIndentSize, "\t arg1 :: Discount factor (gamma).\n"
                                        "\t arg2 :: Value iteration shall terminate when the computed value "
                                        "function is within $arg2/2$ of the optimal.\n");
    else if(command == "polIt")
        result += Utils::wordWrapString(helpIndentSize, "\t arg1 :: Discount factor (gamma).\n\n");
    
    return result;
}


/*Timer access functions.*/

/*see \member{Timer.stop()}.*/
PUBLIC void StateBasedSolutionWrapper::stopTimer()
{
    timer.stop();
}

/*see \member{Timer.start()}.*/
PUBLIC void StateBasedSolutionWrapper::startTimer()
{
    timer.start();
}

/*Caches the time via \member{getTimerTime} before the \member{timer}
 *is reset see \member{Timer.reset()}.*/
PUBLIC void StateBasedSolutionWrapper::resetTimer()
{
    /*Cache the last time calculated.*/
    timeCache = getTimerTime();

    /*Reset the timer.*/
    timer.reset();
}

/*see \member{Timer.read()}.*/
PUBLIC double StateBasedSolutionWrapper::getTimerTime()const
{
    return timer.read();
}

/*Number of iterations of any of the current solution algorithm that
 *have been performed.*/
PUBLIC unsigned int StateBasedSolutionWrapper::getIterations()const
{
    return iterations;
}
