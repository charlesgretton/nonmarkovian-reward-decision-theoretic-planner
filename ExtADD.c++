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

/* ExtADD.c++
 *
 * \paragraph{:Purpose:}
 *
 * Extends CUDD's ADD class to provide functionality that is either
 * missing from CUDD, or is specific to this application.
 */

#include "common.h++"
#ifdef HEADER

#include <vector>
#include <string>
#include <set>
using namespace std;

#include "include/cuddObj.hh"

#include "Utils.h++"
#include "ADDvisitor.h++"

class ExtADD : public ADD
{
public:
    ExtADD(const ADD& add)
    {
        ADD::operator=(add);
    }

    ExtADD()
    {
    }

    /* Provide missing const version */
    Cudd* manager() const
    {
        return (const_cast<ExtADD*> (this))->ADD::manager();
    }

    /* Provide missing const version */
    DdNode* getNode() const
    {
        return (const_cast<ExtADD *> (this))->ADD::getNode();
    }

    /* Provide missing const version */
    ExtADD ExistAbstract(ADD cube) const
    {
        return (const_cast<ExtADD *> (this))->ADD::ExistAbstract(cube);
    }

    /* Provide missing const version */
    ExtADD operator+(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator+(other);
    }

    /* Provide missing const version */
    ExtADD operator&(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator&(other);
    }

    /* Provide missing const version */
    ExtADD operator*(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator*(other);
    }

    /* Provide missing const version */
    ExtADD operator-(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator-(other);
    }

    /* Provide missing const version */
    ExtADD operator|(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator|(other);
    }
    /* Provide missing const version */
    bool operator==(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator==(other);
    }

    /* Provide missing const version */
    bool operator!=(ADD other) const
    {
        return (const_cast<ExtADD *> (this))->ADD::operator!=(other);
    }

    /* Return the true child of this node */
    ExtADD thenChild() const
    {
        return ADD(manager(), Cudd_T(getNode()));
    }

    /* Return the false child of this node */
    ExtADD elseChild() const
    {
        return ADD(manager(), Cudd_E(getNode()));
    }

    /* Is this node a leaf node? (a constant) */
    bool isConstant() const
    {
        return Cudd_IsConstant(getNode());
    }

    /* What is the value of this node? - must be a leaf node */
    double value() const
    {
        return Cudd_V(getNode());
    }

    /* Get CUDD's integer ID for the variable at this node - must not
     * be a leaf node. */
    int getIndex() const
    {
        return getNode()->index;
    }

    /* Returns an integer that uniquely identifies this node */
    int getID() const
    {
        // in order to get a unique ID for each node, we convert the
        // pointer to an integer
        return reinterpret_cast<int> (getNode());
    }

    /* Get the primed version of a variable - we need the number of
     * variables, since the primed versions follow the unprimed ones.
     */
    ExtADD prime(int numVars) const
    {
        assert(!isConstant());
        assert(getIndex() < numVars);
        return manager()->addVar(getIndex() + numVars);
    }

    /* Return the variable at this node as an ADD with 1 on the true
     * branch and 0 on the false branch. */
    ExtADD var() const
    {
        return manager()->addVar(getIndex());
    }

    /* Get the unprimed version of this node.  The node returned is
     * just a basic node with 1 on its true branch and 0 on its false
     * branch. */
    ExtADD unPrime(int numVars) const
    {
        assert(!isConstant());
        assert(getIndex() >= numVars && getIndex() < numVars * 2);
        return manager()->addVar(getIndex() - numVars);
    }

    /* Is this node primed? */
    bool isPrimed(int numVars) const
    {
        return getIndex() >= numVars;
    }
};

#endif
#include "ExtADD.h++"
#include <sstream>
using namespace std;

/* Apply the C-style binary ADD function recursively to the ADD rooted
 * at this node, and return the result. */
PUBLIC ExtADD ExtADD::apply(DdNode * (*op)(DdManager *, DdNode **, DdNode **),
                            const ExtADD& other)
{
    ADD result(manager(),
               Cudd_addApply(manager()->getManager(), op, getNode(), other.getNode()));
    return result;
}

#include <cuddInt.h>

static DdNode *cuddAddApplyRecurNoCache(
    DdManager * dd,
    DdNode * (*op)(DdManager *, DdNode **, DdNode **),
    DdNode * f,
    DdNode * g)
{
    DdNode *res,
	   *fv, *fvn, *gv, *gvn,
	   *T, *E;
    unsigned int ford, gord;
    unsigned int index;

    /* Check terminal cases.
     */
    statLine(dd);
    res = (*op)(dd,&f,&g);
    if (res != NULL) return(res);

    /* Recursive step. */
    ford = cuddI(dd,f->index);
    gord = cuddI(dd,g->index);
    if (ford <= gord) {
	index = f->index;
	fv = cuddT(f);
	fvn = cuddE(f);
    } else {
	index = g->index;
	fv = fvn = f;
    }
    if (gord <= ford) {
	gv = cuddT(g);
	gvn = cuddE(g);
    } else {
	gv = gvn = g;
    }

    T = cuddAddApplyRecurNoCache(dd,op,fv,gv);
    if (T == NULL) return(NULL);
    cuddRef(T);

    E = cuddAddApplyRecurNoCache(dd,op,fvn,gvn);
    if (E == NULL) {
	Cudd_RecursiveDeref(dd,T);
	return(NULL);
    }
    cuddRef(E);

    res = (T == E) ? T : cuddUniqueInter(dd,(int)index,T,E);
    if (res == NULL) {
	Cudd_RecursiveDeref(dd, T);
	Cudd_RecursiveDeref(dd, E);
	return(NULL);
    }
    cuddDeref(T);
    cuddDeref(E);

    return(res);

}

static DdNode *Cudd_addApplyNoCache(
    DdManager * dd,
    DdNode * (*op)(DdManager *, DdNode **, DdNode **),
    DdNode * f,
    DdNode * g)
{
    DdNode *res;

    do {
	dd->reordered = 0;
	res = cuddAddApplyRecurNoCache(dd,op,f,g);
    } while (dd->reordered == 1);
    return(res);

}

/* Apply the C-style binary ADD function recursively to the ADD rooted
 * at this node, and return the result.  This function does not use
 * Cudd's cache. */
PUBLIC ExtADD ExtADD::applyNoCache(DdNode * (*op)(DdManager *, DdNode **, DdNode **),
                                   const ExtADD& other)
{
    ADD result(manager(),
               Cudd_addApplyNoCache(manager()->getManager(), op, getNode(), other.getNode()));
    return result;
}


/* Apply the C-style monadic ADD function recursively to the ADD rooted
 * at this node, and return the result. */
PUBLIC ExtADD ExtADD::apply(DdNode * (*op)(DdManager *, DdNode *))
{
    ADD result(manager(),
               Cudd_addMonadicApply(manager()->getManager(), op, getNode()));
    return result;
}

/* Return an ADD the same as this one, but with all previously
 * unprimed variables, primed. */
PUBLIC ExtADD ExtADD::primeRecursive(int numVars) const
{
    if (isConstant()) {
        return *this;
    } else {
        ExtADD selfPrimed;
        if (isPrimed(numVars))
            selfPrimed = *this;
        else
            selfPrimed = prime(numVars);
        return (selfPrimed & thenChild().primeRecursive(numVars))
            + ((~selfPrimed) & elseChild().primeRecursive(numVars));
    }
}

/* Return an ADD the same as this one, but with all previously
 * primed variables, unprimed. */
PUBLIC ExtADD ExtADD::unprimeRecursive(int numVars) const
{
    if (isConstant()) {
        return *this;
    } else {
        ExtADD selfUnprimed;
        if (isPrimed(numVars))
            selfUnprimed = unPrime(numVars);
        else
            selfUnprimed = *this;
        return (selfUnprimed & thenChild().unprimeRecursive(numVars))
            + ((~selfUnprimed) & elseChild().unprimeRecursive(numVars));
    }
}

/* Sum over the variables indexed by `vars'
 */
PUBLIC ExtADD ExtADD::sumOver(const vector<int> vars) const
{
    ADD *sumOverVars = new ADD[vars.size()];

    for (int k = 0; k != int(vars.size()); k++) {
        sumOverVars[k] = manager()->addVar(vars[k]);
    }
    ADD cube = manager()->addComputeCube(sumOverVars, NULL, vars.size());
    delete[] sumOverVars;
    return ExistAbstract(cube);
}

/* Return a vector of the indices of the variables used in the ADD
 * rooted at this node. */
PUBLIC vector<int> ExtADD::variablesUsed(int numVars, bool invert DEFVAL(false)) const
{
    vector<int> result;
    int *supportIndex = Cudd_SupportIndex(manager()->getManager(), getNode());
    for (int i = 0; i < numVars; i++) {
        if ((supportIndex[i] != 0) == !invert)
            result.push_back(i);
    }

    return result;
}

/* The maximum value of leaf nodes in this ADD
 */
PUBLIC double ExtADD::maximumValue() const
{
    if (isConstant()) {
        return value();
    } else {
        return max(thenChild().maximumValue(), elseChild().maximumValue());
    }
}

/* Return an equivalent ADD with zeros maintained, and non-zeros
 * converted to 1s.
 */
PUBLIC ExtADD ExtADD::toZeroOne()
{
    if (isConstant()) {
        if (0.0 == value())
            return *this;
        else
            return manager()->addOne();
    } else {
        return (var() & thenChild().toZeroOne()) +
            ((~var()) & elseChild().toZeroOne());
    }
}

/* This is experimental and doesn't work. */
DdNode* ADDfilter(DdManager * dd, DdNode ** f, DdNode ** g)
{
    DdNode *F, *G;
    
    F = *f; G = *g;
    if (G == Cudd_ReadOne(dd)) {
        cout << "G == Cudd_ReadOne(dd))\n";
        return F;
    }
    if (G == Cudd_ReadZero(dd)) {
        cout << "G == Cudd_ReadZero(dd))\n";
        return G;
    }
#if 1
    if (Cudd_T(G) == Cudd_ReadZero(dd)) {
        if (Cudd_IsConstant(F)) {
            cout << "T(G) == 0, Const(F)\n";
            return F;
        } else {
            cout << "T(G) == 0, Node(F)\n";
            //return Cudd_addApply(dd, ADDfilter, Cudd_E(F), Cudd_E(G));
            return Cudd_E(F);
            //*f = Cudd_E(F);
            //*g = Cudd_E(G);
        }
    }
    if (Cudd_E(G) == Cudd_ReadZero(dd)) {
        if (Cudd_IsConstant(F)) {
            cout << "E(G) == 0, Const(F)\n";
            return F;
        } else {
            cout << "E(G) == 0, Node(F)\n";
            //return Cudd_addApply(dd, ADDfilter, Cudd_T(F), Cudd_T(G));
            return Cudd_T(F);
            //*f = Cudd_T(F);
            //*g = Cudd_T(G);
        }
    }
#endif
    return NULL;
}

/* Prune nodes where one branch or the other goes to 0
 */
PUBLIC ExtADD ExtADD::filter(const ADD& other)
{
    return apply(ADDfilter, other);
}

/* Goto 0 wherever `constraint' does, but without introducing any new
 * variables
 */
PUBLIC ExtADD ExtADD::constrain(const ExtADD& constraint, int numVars)
{
    vector<int> unusedVariables = variablesUsed(numVars, true);
    ExtADD tmp = constraint.sumOver(unusedVariables);
    tmp = tmp.toZeroOne();
    return (*this) * tmp;
}

/* Does this ADD contain only 0s and 1s?
 */
PUBLIC bool ExtADD::isZeroOne()
{
    if (isConstant()) {
        if (0.0 == value() || 1.0 == value())
            return true;
        else
            return false;
    } else {
        return thenChild().isZeroOne() && elseChild().isZeroOne();
    }
}

/* Counts the number of nodes
 */
PUBLIC int ExtADD::dagSize()
{
    return Cudd_DagSize(getNode());
}

double ExtADD::countPathsToNonZero()
{
    return Cudd_CountPathsToNonZero(getNode());
}

/* Produce a text representation of a tree that corresponds to this ADD
 */
PUBLIC string ExtADD::toTreeString(vector<string> names)
{
    ostringstream result;
    result << "(";
    if (isConstant()) {
        result << value();
    } else {
        result << names[getIndex()] << " "
               << thenChild().toTreeString(names) << " "
               << elseChild().toTreeString(names);
    }
    result << ")";
    return result.str();
}

/* Given a visitor, traverse all the internal and leaf nodes
 * rooted at this node. */
PUBLIC void ExtADD::traverse(ADDvisitor& visitor) const
{
    if (isConstant()) {
        visitor.visitLeaf(*this);
    } else {
        visitor.visitInternal(*this);
        thenChild().traverse(visitor);
        elseChild().traverse(visitor);
    }
}

/* Given a const visitor, traverse all the internal and leaf nodes
 * rooted at this node. */
PUBLIC void ExtADD::traverse(const ADDvisitor& visitor) const
{
    if (isConstant()) {
        visitor.visitLeaf(*this);
    } else {
        visitor.visitInternal(*this);
        thenChild().traverse(visitor);
        elseChild().traverse(visitor);
    }
}

