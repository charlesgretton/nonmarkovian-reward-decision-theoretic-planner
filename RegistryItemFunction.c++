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

/* RegistryItemFunction.c++
 *
 * \paragraph{:Purpose:}
 *
 * Holds information on a function that can be called from the command
 * line.
 */

#include "common.h++"
#ifdef HEADER

#include "RegistryItemFunction.h++"
#include "RegistryItem.h++"
#include "CommandListener.h++"

class RegistryItemFunction : public RegistryItem
{
private:
    string svalue;
    CommandListener *listener;
    int numParameters;
};

#endif
#include "RegistryItemFunction.h++"

PUBLIC RegistryItemFunction::RegistryItemFunction(CommandListener *l, int np)
    : listener(l),
      numParameters(np)
{
}

PUBLIC bool RegistryItemFunction::beginEvaluation(const string &command, const vector<string> &parameters)
{
    if (int(parameters.size()) > numParameters) {
        //cerr << tooManyParameters << " to " << command << " command.\n";
        return false;
    } else if (int(parameters.size()) < numParameters) {
        //cerr << tooFewParameters << " to " << command << " command.\n";
        return false;
    } else {
        listener->commandIssued(command, parameters);
        return listener->acceptedLastCommandIssued();
    }
}

PUBLIC bool RegistryItemFunction::evaluate(const string &command)
{
    return listener->commandIteration(command);
}

PUBLIC string RegistryItemFunction::evaluationResult(const string& command)
{
    return listener->getCommandResult(command);
}

PUBLIC void RegistryItemFunction::endEvaluation(const string& command)
{
    listener->endCommand(command);
}

PUBLIC string RegistryItemFunction::shortHelp(const string &command) const
{
    return listener->shortHelp(command);
}

PUBLIC string RegistryItemFunction::longHelp(const string &command) const
{
    return listener->longHelp(command);
}

PUBLIC CommandListener const* RegistryItemFunction::getListener() const
{
    return listener;
}

PUBLIC void RegistryItemFunction::setListener(CommandListener* cl)
{
    listener = cl;
}
