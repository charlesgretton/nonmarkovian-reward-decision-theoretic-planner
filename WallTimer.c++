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

/* WallTimer.c++
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"
#include <sys/time.h>
#include <time.h>

class WallTimer : public CommandListener
{
private:
    struct timeval start, stop;
};

#endif
#include "WallTimer.h++"
#include "Registry.h++"
#include "Utils.h++"

PUBLIC WallTimer::WallTimer()
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("startTimer", this, 0);
    reg->setFunction("stopTimer", this, 0);
    reg->setFunction("readTimer", this, 0);
}

PUBLIC WallTimer::~WallTimer()
{
    Registry *reg = Registry::getInstance();
    reg->unregister("startTimer");
    reg->unregister("stopTimer");
    reg->unregister("readTimer");
}

void WallTimer::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "startTimer") {
        gettimeofday(&start, 0);
    } else if (command == "stopTimer") {
        gettimeofday(&stop, 0);
    } else if (command == "readTimer") {
        long seconds = stop.tv_sec - start.tv_sec;
        long useconds = stop.tv_usec - start.tv_usec;
        commandResult = Utils::doubleToString(seconds + useconds / 1000000.0);
    }
}

string WallTimer::shortHelp(const string &command) const
{
    if (command == "startTimer")
        return shortHelpLine(command, "Starts a wall-time timer from 0.");
    else if (command == "stopTimer")
        return shortHelpLine(command, "Stops wall-time timer.");
    else if (command == "readTimer")
        return shortHelpLine(command, "Returns time elapsed.");
    return "";
}

string WallTimer::longHelp(const string &command) const
{
    return shortHelp(command) + "\n";
}
