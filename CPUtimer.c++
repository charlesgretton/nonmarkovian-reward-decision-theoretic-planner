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

/* CPUtimer.c++
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"
#include <sys/times.h>
#include <time.h>
#include <unistd.h>

class CPUtimer : public CommandListener
{
private:
    struct tms start, stop;
};

#endif
#include "CPUtimer.h++"
#include "Registry.h++"
#include "Utils.h++"

PUBLIC CPUtimer::CPUtimer()
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("startCPUtimer", this, 0);
    reg->setFunction("stopCPUtimer", this, 0);
    reg->setFunction("readCPUtimer", this, 0);
}

PUBLIC CPUtimer::~CPUtimer()
{
    Registry *reg = Registry::getInstance();
    reg->unregister("startCPUtimer", this);
    reg->unregister("stopCPUtimer", this);
    reg->unregister("readCPUtimer", this);
}

void CPUtimer::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "startCPUtimer") {
        times(&start);
    } else if (command == "stopCPUtimer") {
        times(&stop);
    } else if (command == "readCPUtimer") {
        double value = double(stop.tms_utime - start.tms_utime + stop.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
        commandResult = Utils::doubleToString(value);
    }
}

string CPUtimer::shortHelp(const string &command) const
{
    if (command == "startCPUtimer")
        return shortHelpLine(command, "Starts a CPU-time timer from 0.");
    else if (command == "stopCPUtimer")
        return shortHelpLine(command, "Stops CPU-time timer.");
    else if (command == "readCPUtimer")
        return shortHelpLine(command, "Returns time elapsed.");
    return "";
}

string CPUtimer::longHelp(const string &command) const
{
    return shortHelp(command) + "\n";
}
