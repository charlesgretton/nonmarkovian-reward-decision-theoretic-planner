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

/* RegistryItemString.c++
 *
 * \paragraph{:Purpose:}
 *
 * Holds the value of a variable on the command line.
 */

#include "common.h++"
#ifdef HEADER

#include "RegistryItem.h++"

class RegistryItemString : public RegistryItem
{
private:
    string svalue;
};

#endif
#include "RegistryItemString.h++"
#include "CommandListener.h++"

PUBLIC RegistryItemString::RegistryItemString(const string &newVal)
    : svalue(newVal)
{
}

PUBLIC string RegistryItemString::evaluationResult(const string &command)
{
    return svalue;
}

PUBLIC string RegistryItemString::shortHelp(const string &command) const
{
    return CommandListener::shortHelpLine(command, "Variable (value = \"" + svalue + "\")");
}

PUBLIC string RegistryItemString::longHelp(const string &command) const
{
    return shortHelp(command) + "\n";
}
