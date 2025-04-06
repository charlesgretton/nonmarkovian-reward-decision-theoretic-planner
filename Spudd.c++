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

/* Spudd.c++
 *
 * \paragraph{:Purpose:}
 *
 * Applies the Spudd algorithm to a domain specification, which must
 * already be Markovian.
 */

#include "common.h++"
#ifdef HEADER

#include "domainSpecification.h++"
#include "ADDreward.h++"
#include "ExtADD.h++"
#include "actionSpecification.h++"
#include "Algorithm.h++"
#include "Diagram.h++"
#include "Utils.h++"
#include "ActionADDmap.h++"

using namespace Formula;
using namespace MDP;

class Spudd : public Algorithm
{
private:
    ADDreward addReward;
    DomainSpecification& domSpec;
    ActionSpecification& actionSpec;
    Cudd& mgr;

    int numProps;
    double gamma;
    ExtADD reward, constraint;

    // run-state
    int iteration;
    double requiredDelta;
    ExtADD oldV;
    ExtADD v;
    ExtADD optimalValue; // valid when lastIteration
    ExtADD policy;
    bool lastIteration;
    bool optimalComputed;
    
    vector<set<action> > policyActions;
    action currentAction;

    ActionADDmap combinedDualActionDiagrams;

    bool hideAssumptions;

    vector<double> deltaHistory;
};

#endif
#include "Spudd.h++"
#include "Utils.h++"

#include <algorithm>
#include <set>
#include <vector>

PUBLIC Spudd::Spudd(DomainSpecification& domSpec, ExtADD automaticConstraint,
                    double gamma, double epsilon, bool hideAssumptions)
    : addReward(domSpec),
      domSpec(domSpec),
      actionSpec(*domSpec.getActionSpecification()),
      mgr(actionSpec.getManager()),
      gamma(gamma),
      reward(addReward.getReward()),
      constraint(addReward.getConstraint() & automaticConstraint),
      lastIteration(false),
      optimalComputed(false),
      combinedDualActionDiagrams(domSpec, constraint),
      hideAssumptions(hideAssumptions)
{        
    vector<proposition> props = domSpec.getPropositions();
    numProps = props.size();

#if 0
    Diagram act;
    act.setNodeNames(domSpec.getPropositions());
    act.setADD(combinedDualActionDiagrams["c"]);
    act.setTitle("c");
    Utils::displayDotString(act.toDotString());
#endif
    
    v = reward;

    iteration = 0;

    requiredDelta = epsilon * (1 - gamma) / (2 * gamma);
}

/*Recalculates the \member{requiredDelta} factor.*/
PUBLIC void Spudd::setEpsilon(const double eps)
{
    Algorithm::setEpsilon(eps);
    requiredDelta = epsilon * (1 - gamma) / (2 * gamma);
}

static Spudd *currentThis;

static DdNode *addToPolicyWrapper(DdManager * dd, DdNode ** f, DdNode ** g)
{
    return currentThis->addToPolicy(dd, f, g);
}

DdNode *Spudd::addToPolicy(DdManager * dd, DdNode ** f, DdNode ** g)
{
    DdNode *F, *G;
    F = *f;
    G = *g;

    if (Cudd_IsConstant(F) && Cudd_IsConstant(G)) {
        if (0.0 == Cudd_V(G)) {
            int gIdx = int(Cudd_V(F));

            if (policyActions[gIdx].count(currentAction) == 0) {
                set<action> newSet = policyActions[gIdx];
                newSet.insert(currentAction);

                vector<set<action> >::iterator existing = find(policyActions.begin(),
                                                               policyActions.end(),
                                                               newSet);
                int newIdx;
                if (existing == policyActions.end()) {
                    newIdx = policyActions.size();
                    policyActions.push_back(newSet);
                } else {
                    newIdx = existing - policyActions.begin();
                }
                    
                return Cudd_addConst(dd, newIdx);
            }
        }
        return F;
    }
    return NULL;
}

/* Get a vector containing the magnitude of the change between each of
 * the iterations performed by the algorithm. */
PUBLIC vector<double> Spudd::getDeltaHistory()
{
    return deltaHistory;
}


/* Get the current (optimal if we are done) policy
 */
PUBLIC Diagram Spudd::getPolicy()
{
    if (!optimalComputed) {
        if (lastIteration) {
            operator()();
        } else {
            lastIteration = true;
            optimalValue = v;
            v = oldV;
            operator()();
            v = optimalValue;
            lastIteration = false;
            optimalComputed = false;
        }
    }
    
    vector<string> actionLists;
    vector<set<action> >::const_iterator i;
    set<action>::const_iterator j;
    for (i = policyActions.begin(); i != policyActions.end(); ++i) {
        string tmp;
        if (i->empty()) {
            tmp = "__NOSHOW__";
        } else {
            for (j = i->begin(); j != i->end(); ++j) {
                if (!tmp.empty())
                    tmp += ",";
                tmp += *j;
            }
        }
        actionLists.push_back(tmp);
    }

    Diagram result;
    result.setNodeNames(domSpec.getPropositions());
    result.setLeafNames(actionLists);
    result.addPage(policy, "Optimal policy");

    return result;
}

/* Get the number of leaves in the optimal value ADD
 */
PUBLIC int Spudd::getOptValSize()
{
    return optimalValue.CountLeaves();
}

/* Get the density of the optimal value ADD
 */
PUBLIC double Spudd::getOptValDensity()
{
    return optimalValue.Density(domSpec.getPropositions().size());
}

/* Get the number of nodes in the optimal value ADD
 */
PUBLIC double Spudd::getOptValNodes()
{
    return optimalValue.dagSize();
}

/* Get the number of paths in the optimal value ADD
 */
PUBLIC double Spudd::getOptValPaths()
{
    return optimalValue.CountPath();
}

PUBLIC Diagram Spudd::getOptimalValue()
{
    Diagram result;
    result.setNodeNames(domSpec.getPropositions());
    result.addPage(optimalValue, "Expected value");

    return result;
}

/* Do an iteration of the algorithm - return true if we are done
 */
PUBLIC bool Spudd::operator()()
{
    if (optimalComputed)
        return true;
    
    double deltaMax;
    if (!lastIteration)
        iteration++;
    oldV = v;
    ExtADD vPrime = v.primeRecursive(numProps);

    v = mgr.addZero();
    if (lastIteration) {
        policyActions.clear();
        // let [0] represent an impossible case - ie, when a
        // constraint is violated
        policyActions.resize(1);
        policy = mgr.addZero();
    }

    map<action, ActionSpecification::CPTS*>::iterator i;
    for (i = actionSpec.begin(); i != actionSpec.end(); ++i) {
        ExtADD temp = vPrime;

        temp *= combinedDualActionDiagrams[i->first];
        
        // check which primed variables we have
        vector<int> primedVariables = temp.variablesUsed(numProps * 2);
        primedVariables.erase(remove_if(primedVariables.begin(), primedVariables.end(),
                                        bind2nd(less<int>(), numProps)),
                              primedVariables.end());

        temp = temp.sumOver(primedVariables);
        
        temp = temp * mgr.constant(gamma) + reward;
        if (lastIteration) {
            ExtADD diff = optimalValue - temp;
            currentAction = i->first;
            currentThis = this;

            /* We use a non-caching apply function because we are
             * carrying extra data - the `currentAction' variable
             * amongst others, so if caching is done we can
             * potentially have problems (which take ages to track
             * down). */
            policy = policy.applyNoCache(addToPolicyWrapper, diff);
        } else {
            v = v.Maximum(temp);
        }
    }

    ExtADD delta = v - oldV;
    ExtADD deltaMaxADD = delta.FindMax();
    deltaMax = deltaMaxADD.value();

    if (lastIteration) {
        if (hideAssumptions)
            policy = policy.filter(constraint);
        else
            policy = policy.constrain(constraint, numProps);
        optimalComputed = true;
        return true;
    }

    deltaHistory.push_back(deltaMax);

    if (deltaMax < requiredDelta) {
        cout << "converged after " << iteration << " iterations with deltaMax = " << deltaMax << endl;

        // We repeat more or less the same calculations on the last
        // iteration, but we don't do maximisation, since we already
        // know the maximum.  Instead we subtract our action values
        // from the known maximum, and where we get zeros, we set that
        // action in the optimal policy
        optimalValue = v;
        v = oldV;
        lastIteration = true;
        return false;
    }

    return false;
}

