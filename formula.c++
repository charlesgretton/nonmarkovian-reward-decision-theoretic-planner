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

#include"formula.h++"
#include"formulaNormaliser.h++"
#include"formulaCopier.h++"
#include"formulaSimplifier.h++"
#include"formulaPrinter.h++"
#include"formulaSize.h++"
#include"formulaLength.h++"

using namespace Formula;

// formula

bool formula::isNegNormal() const
{
    return negNormal;
}

void formula::setNegNormal(bool cn)
{
    negNormal = cn;
}

// unary

unary::unary(const unary& u)
{
    this->f = u[0]->copy();
}

unary::unary(){assert(0);}

unary::unary(formula* f_in)
{
    f = f_in;
}

unary::~unary()
{
    delete f;
}

formula* unary::operator[](OperatorIndexType i) const
{
    return f;
}


OperatorIndexType unary::begin()const
{
    return vBegin;
}

OperatorIndexType unary::end()const
{
    return vEnd;
}

// summaryUnary

summaryUnary::summaryUnary(const summaryUnary& u)
    :unary(u),
     depth(u.depth),
     MIN_DEPTH(u.MIN_DEPTH)
{}

summaryUnary::summaryUnary(){assert(0);}

summaryUnary::summaryUnary(formula* f_in, unsigned int depth, unsigned int MIN_DEPTH)
    :unary(f_in),
     depth(depth),
     MIN_DEPTH(MIN_DEPTH)
{}

bool summaryUnary::operator--()
{
    if(depth > MIN_DEPTH)
    {
        depth--;
        return true;
    }
    else
        return false;
}

unsigned int summaryUnary::getDepth() const
{
    return depth;
}

// binary

binary::binary(const binary& b)
{
    this->l = b[0]->copy();
    this->r = b[1]->copy();
}

binary::binary(){assert(0);}

binary::binary(formula* lft, formula* rgh)
{
    l = lft;
    r = rgh;
}

binary::~binary()
{
    delete l;
    delete r;
}

formula* binary::operator[](OperatorIndexType i) const
{
    if(i == 0)
        return l;
    else
        return r;
}

OperatorIndexType binary::begin()const
{
    return vBegin;
}

OperatorIndexType binary::end()const
{
    return vEnd;
}

/*******************************Visitor traversal interface methods.*/

template<class Return, class Visitor>
Return* formula::givenTraversal(Visitor &v) const
{
    v.initBuild();
    this->accept(&v);
    return v.getBuild();
}

formula* PCformula::negNormalise(bool carry) const
{
    PCnormaliser<NA> normaliser(carry);
    
    return givenTraversal<formula, PCnormaliser<NA> >(normaliser);
}

formula* FLTLformula::negNormalise(bool carry) const
{
    FLTLnormaliser normaliser(carry);
    
    return givenTraversal<formula, FLTLnormaliser >(normaliser);
}

formula* PLTLformula::negNormalise(bool carry) const
{
    PLTLnormaliser normaliser(carry);
    
    return givenTraversal<formula, PLTLnormaliser >(normaliser);
}


formula* PCformula::copy() const
{
    PCcopier<NA> copier;
    
    return givenTraversal<formula, PCcopier<NA> >(copier);
}

formula* FLTLformula::copy() const
{
    FLTLcopier copier;
    
    return givenTraversal<formula, FLTLcopier >(copier);
}

formula* PLTLformula::copy() const
{
    PLTLcopier copier;
    
    return givenTraversal<formula, PLTLcopier >(copier);
}

formula* PCformula::simplify() const
{
    PCsimplifier<NA> simplifier;
    
    return givenTraversal<formula, PCsimplifier<NA> >(simplifier);
}

formula* FLTLformula::simplify() const
{
    FLTLsimplifier simplifier;
    
    return givenTraversal<formula, FLTLsimplifier >(simplifier);
}

formula* PLTLformula::simplify() const
{
    PLTLsimplifier simplifier;
    
    return givenTraversal<formula, PLTLsimplifier >(simplifier);
}

string* PCformula::print() const
{
    PCprinter<NA> printer;
    
    return givenTraversal<string, PCprinter<NA> >(printer);
}

string* FLTLformula::print() const
{
    FLTLprinter printer;
    
    return givenTraversal<string, FLTLprinter >(printer);  
}

string* PLTLformula::print() const
{
    PLTLprinter printer;
    
    return givenTraversal<string, PLTLprinter >(printer);  
}

unsigned int PCformula::size() const
{
    PCsize<NA> size;
    
    unsigned int* tmp = givenTraversal<unsigned int, PCsize<NA> >(size);
    unsigned int result = *tmp;
    delete tmp;
    return result;
}

unsigned int FLTLformula::size() const
{
    FLTLsize size;
    
    unsigned int* tmp = givenTraversal<unsigned int, FLTLsize >(size);  
    unsigned int result = *tmp;
    delete tmp;
    return result;
}

unsigned int PLTLformula::size() const
{
    PLTLsize size;
    
    unsigned int* tmp = givenTraversal<unsigned int, PLTLsize >(size);
    unsigned int result = *tmp;
    delete tmp;
    return result;
}

unsigned int PCformula::length() const
{
    PClength<NA> length;
    
    unsigned int* tmp = givenTraversal<unsigned int, PClength<NA> >(length);
    unsigned int result = *tmp;
    delete tmp;
    return result;
}

unsigned int FLTLformula::length() const
{
    FLTLlength length;
    
    unsigned int* tmp = givenTraversal<unsigned int, FLTLlength >(length);  
    unsigned int result = *tmp;
    delete tmp;
    return result;
}

unsigned int PLTLformula::length() const
{
    PLTLlength length;
    
    unsigned int* tmp = givenTraversal<unsigned int, PLTLlength >(length);
    unsigned int result = *tmp;
    delete tmp;
    return result;
}

//  literal

literal::literal(string str)
{
    PCformula::formula::negNormal = true;
    id = str;
}

literal::literal(const literal& l)
{
    PCformula::formula::negNormal = true;
    id = l.getId();
}

literal::~literal(){}

const string& literal::getId() const {return id;}
void literal::setId(string& str){id = str;}
void literal::accept_() const {v->visit<literal>(this);}

//  assLiteral

assLiteral::assLiteral(const assLiteral& al)
{
    PCformula::formula::negNormal = true;
    assignment = al.getAssignment();
}

assLiteral::assLiteral(bool b)
{
    PCformula::formula::negNormal = true;
    assignment = b;
}

assLiteral::~assLiteral(){}

bool assLiteral::getAssignment() const {return assignment;}

void assLiteral::accept_() const {v->visit<assLiteral>(this);}

// conj

conj::conj(const conj& c):binaryCommutative(c){PCformula::formula::negNormal = c.isNegNormal();}
conj::conj(formula *l, formula *r):binaryCommutative(l, r){PCformula::formula::negNormal = (l->isNegNormal() && r->isNegNormal());}
conj::~conj(){}

void conj::accept_() const {v->visit<conj>(this);}

// disj

disj::disj(const disj& c):binaryCommutative(c){PCformula::formula::negNormal = c.isNegNormal();}
disj::disj(formula *l, formula *r):binaryCommutative(l, r){PCformula::formula::negNormal = l->isNegNormal() && r->isNegNormal();}
disj::~disj(){}

void disj::accept_() const {v->visit<disj>(this);}

// iff

iff::iff(const iff& c):binaryCommutative(c){PCformula::formula::negNormal = c.isNegNormal();}
iff::iff(formula *l, formula *r):binaryCommutative(l, r){PCformula::formula::negNormal = l->isNegNormal() && r->isNegNormal();}
iff::~iff(){}

void iff::accept_() const {v->visit<iff>(this);}

// imp

imp::imp(const imp& c):binary(c){PCformula::formula::negNormal = c.isNegNormal();}
imp::imp(formula *l, formula *r):binary(l, r){PCformula::formula::negNormal = l->isNegNormal() && r->isNegNormal();}
imp::~imp(){}

void imp::accept_() const {v->visit<imp>(this);}

// lnot

lnot::lnot(const lnot& c):unary(c){PCformula::formula::negNormal = c.isNegNormal();}
lnot::lnot(formula *f):unary(f){}
lnot::~lnot(){}

void lnot::accept_() const {v->visit<lnot>(this);}

// nxt

nxt::nxt(const nxt& c):unary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
nxt::nxt(formula *f):unary(f){FLTLformula::formula::negNormal = f->isNegNormal();}
nxt::~nxt(){}

void nxt::accept_() const {v->visit<nxt>(this);}

// nxtDisj

nxtDisj::nxtDisj(const nxtDisj& c):summaryUnary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
nxtDisj::nxtDisj(formula *f, unsigned int depth):summaryUnary(f, depth, 1){FLTLformula::formula::negNormal = f->isNegNormal();}
nxtDisj::~nxtDisj(){}

void nxtDisj::accept_() const {v->visit<nxtDisj>(this);}

// nxtConj

nxtConj::nxtConj(const nxtConj& c):summaryUnary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
nxtConj::nxtConj(formula *f, unsigned int depth):summaryUnary(f, depth, 2){FLTLformula::formula::negNormal = f->isNegNormal();}
nxtConj::~nxtConj(){}

void nxtConj::accept_() const {v->visit<nxtConj>(this);}

// nxtNest

nxtNest::nxtNest(const nxtNest& c):summaryUnary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
nxtNest::nxtNest(formula *f, unsigned int depth):summaryUnary(f, depth, 2){FLTLformula::formula::negNormal = f->isNegNormal();}
nxtNest::~nxtNest(){}

void nxtNest::accept_() const {v->visit<nxtNest>(this);}

// fut

fut::fut(const fut& c):binary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
fut::fut(formula *l, formula *r):binary(l, r){FLTLformula::formula::negNormal = l->isNegNormal() && r->isNegNormal();}
fut::~fut(){}

void fut::accept_() const {v->visit<fut>(this);}

// strFut

strFut::strFut(const strFut& c):binary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
strFut::strFut(formula *l, formula *r):binary(l, r){FLTLformula::formula::negNormal = l->isNegNormal() && r->isNegNormal();}
strFut::~strFut(){}

void strFut::accept_() const {v->visit<strFut>(this);}

// fbx

fbx::fbx(const fbx& c):unary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
fbx::fbx(formula *f):unary(f){FLTLformula::formula::negNormal = f->isNegNormal();}
fbx::~fbx(){}

void fbx::accept_() const {v->visit<fbx>(this);}

// fdi

fdi::fdi(const fdi& c):unary(c){FLTLformula::formula::negNormal = c.isNegNormal();}
fdi::fdi(formula *f):unary(f){FLTLformula::formula::negNormal = f->isNegNormal();}
fdi::~fdi(){}

void fdi::accept_() const {v->visit<fdi>(this);}

//  dollars

dollars::dollars(const dollars& d){FLTLformula::formula::negNormal = true;}
dollars::~dollars(){}
dollars::dollars(){FLTLformula::formula::negNormal = true;}

void dollars::accept_() const{v->visit<dollars>(this);}

//  startStateProposition

startStateProposition::startStateProposition(){}
    
startStateProposition::~startStateProposition(){}

assLiteral* startStateProposition::startStateAssignment()const
{
    return new assLiteral(true);
}

void startStateProposition::accept_() const
{v->visit<startStateProposition>(this);}

//  prv

prv::prv(const prv& c):unary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
prv::prv(formula *f):unary(f){PLTLformula::formula::negNormal = f->isNegNormal();}
prv::~prv(){}

assLiteral* prv::startStateAssignment()const
{
    return new assLiteral(false);
}

void prv::accept_() const {v->visit<prv>(this);}

//  prvDisj

prvDisj::prvDisj(const prvDisj& c):summaryUnary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
prvDisj::prvDisj(formula *f, unsigned int depth):summaryUnary(f, depth, 2){PLTLformula::formula::negNormal = f->isNegNormal();}
prvDisj::~prvDisj(){}

assLiteral* prvDisj::startStateAssignment()const
{
    return new assLiteral(false);
}

void prvDisj::accept_() const {v->visit<prvDisj>(this);}

//  prvConj

prvConj::prvConj(const prvConj& c):summaryUnary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
prvConj::prvConj(formula *f, unsigned int depth):summaryUnary(f, depth, 2){PLTLformula::formula::negNormal = f->isNegNormal();}
prvConj::~prvConj(){}

assLiteral* prvConj::startStateAssignment()const
{
    return new assLiteral(false);
}

void prvConj::accept_() const {v->visit<prvConj>(this);}

//  prvNest

prvNest::prvNest(const prvNest& c):summaryUnary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
prvNest::prvNest(formula *f, unsigned int depth):summaryUnary(f, depth, 1){PLTLformula::formula::negNormal = f->isNegNormal();}
prvNest::~prvNest(){}

assLiteral* prvNest::startStateAssignment()const
{
    return new assLiteral(false);
}

void prvNest::accept_() const {v->visit<prvNest>(this);}

// snc

snc::snc(const snc& c):binary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
snc::snc(formula *l, formula *r):binary(l, r){PCformula::formula::negNormal = l->isNegNormal() && r->isNegNormal();}
snc::~snc(){}

assLiteral* snc::startStateAssignment()const
{
    return new assLiteral(true);
}

void snc::accept_() const {v->visit<snc>(this);}

// pbx

pbx::pbx(const pbx& c):unary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
pbx::pbx(formula *f):unary(f){PLTLformula::formula::negNormal = f->isNegNormal();}
pbx::~pbx(){}

assLiteral* pbx::startStateAssignment()const
{
    return new assLiteral(true);
}

void pbx::accept_() const {v->visit<pbx>(this);}

// pdi

pdi::pdi(const pdi& c):unary(c){PLTLformula::formula::negNormal = c.isNegNormal();}
pdi::pdi(formula *f):unary(f){PLTLformula::formula::negNormal = f->isNegNormal();}
pdi::~pdi(){}

assLiteral* pdi::startStateAssignment()const
{
    return new assLiteral(false);
}

void pdi::accept_() const {v->visit<pdi>(this);}
