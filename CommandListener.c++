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

/* CommandListener.c++
 *
 * \paragraph{:Purpose:}
 *
 * Provides an interface for classes that can respond to function
 * calls from the command line.
 */

#include "common.h++"
#ifdef HEADER

#include <string>
#include <vector>
using namespace std;

class CommandListener
{
public:
    /* Called when a command is issued - only trivial work should be
     * done here.  The main work should be done in \function{commandIteration} */
    virtual void commandIssued(const string &command, const vector<string> &parameters) = 0;

    /* Return a short help message of the form provided by
     * shortHelpLine.
     */
    virtual string shortHelp(const string &command) const = 0;
    
protected:
    string commandResult;
    
    const static int helpIndentSize = 2;

    /*Was the last command issued, via \method{commandIssued()},
     *accepted? This shall be true until proven false.*/
    bool acceptLastCommand;
};

#endif
#include "CommandListener.h++"

#include <algorithm>
using namespace std;

PUBLIC CommandListener::CommandListener()
    :acceptLastCommand(true)
{}

PUBLIC VIRTUAL CommandListener::~CommandListener()
{
}

PUBLIC STATIC string CommandListener::shortHelpLine(const string &s1, const string &s2)
{
    return s1 + string(max(23 - int(s1.length()), 1), ' ') + s2;
}

/* Returns true if we are done.  If we return true, we are done, and
 * expect not to be called again.  If we return false, we will
 * probably be called again provided the user does not cancel the
 * command - in which case endCommand will be called (it will be
 * called when we return true also).
 */
PUBLIC VIRTUAL bool CommandListener::commandIteration(const string& command)
{
    return commandIteration();
}

/* This interface is depreciated */
PUBLIC VIRTUAL bool CommandListener::commandIteration()
{
    return true;
}

/* Allow clean up of any resources allocated during the execution of
 * the command.  This will be called either when commandIteration
 * returns true, or when the command is cancelled (presumably due to
 * user interaction).
 */
PUBLIC VIRTUAL void CommandListener::endCommand(const string& command)
{
}

/* Returns the result of the last command executed.  Will only be
 * called after we return true from commandIteration.
 */
PUBLIC VIRTUAL string CommandListener::getCommandResult(const string& command) const
{
    return getCommandResult();
}

/* This interface is depreciated */
PUBLIC VIRTUAL const string &CommandListener::getCommandResult() const
{
    return commandResult;
}

/* Return a more detailed help message formatted with newlines. */
PUBLIC VIRTUAL string CommandListener::longHelp(const string &command) const
{
    return shortHelp(command) + "\n";
}

/*Was the last command issued, via \method{commandIssued()},
 *accepted?*/
PUBLIC bool CommandListener::acceptedLastCommandIssued()
{
    return acceptLastCommand;
}

