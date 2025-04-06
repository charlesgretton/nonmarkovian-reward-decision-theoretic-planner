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

/* RegistryItem.h++
 *
 * \paragraph{:Purpose:} Provide a common interface for function and
 * variables that can be evaluated via the command line.
 *
 * \paragraph{:Usage:} beginEvaluation will always be called first,
 * followed by evaluate, which is called zero or more times.  evaluate
 * however is not called after it has returned true.  evaluationResult
 * may then be called.  Finally endEvaluation is called.
 */
#include "RegistryItem.h++"

#ifdef HEADER

#include <vector>
#include <string>
using namespace std;

class RegistryItem
{
public:
    virtual string evaluationResult(const string &command) = 0;

    /* Provide a one-line short description of this command/variable -
     * no newline at end of string */
    virtual string shortHelp(const string &command) const = 0;

    /* Provide a multi-line help with usage info and any other
     * relevant notes - should have shortHelp as first line, then
     * subsequent lines indented by 2 spaces.  Each line (including
     * the last) should be terminated with a linefeed. */
    virtual string longHelp(const string &command) const = 0;
};

#endif
#include "common.h++"

PUBLIC VIRTUAL RegistryItem::~RegistryItem()
{
}

PROTECTED STATIC const string RegistryItem::tooFewParameters = "Error: Too few parameters";
PROTECTED STATIC const string RegistryItem::tooManyParameters = "Error: Too many parameters";

/* Perform any setup necessary for the evaluation - if the evaluation
 * requires no parameters, and no setup, then this need not be
 * overridden.  Returns false if there is an error.
 */
PUBLIC VIRTUAL bool RegistryItem::beginEvaluation(const string &command, const vector<string> &parameters)
{ 
    return true;
}

/* Perform an iteration of work evaluating this command - return true
 * when we are done.  If the work to be done is only a small amount
 * that can be done in either beginEvaluation or evaluationResult,
 * then this need not be overridden. */
PUBLIC VIRTUAL bool RegistryItem::evaluate(const string &command)
{
    return true;
}

/* Free any resources used during the evaluation, and optionally leave
 * things in a state where a new evaluation can be performed. */
PUBLIC VIRTUAL void RegistryItem::endEvaluation(const string &command)
{
}

