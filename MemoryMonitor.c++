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

/* MemoryMonitor.c++
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"
#include "Thread.h++"

#include <cstdio>
using namespace std;

class MemoryMonitor : public CommandListener, public Thread
{
private:
    double initialMemory;
    double peakMemory;
};

#endif
#include "MemoryMonitor.h++"
#include "Registry.h++"
#include "Utils.h++"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

PUBLIC MemoryMonitor::MemoryMonitor()
{
    Registry *reg = Registry::getInstance();
    reg->setFunction("monitorMemory", this, 1);
    reg->setFunction("stopMonitoringMemory", this, 0);
    reg->setFunction("peakMemoryUsage", this, 0);
}

PUBLIC MemoryMonitor::~MemoryMonitor()
{
    Registry *reg = Registry::getInstance();
    reg->unregister("monitorMemory", this);
    reg->unregister("stopMonitoringMemory", this);
    reg->unregister("peakMemoryUsage", this);

    if (isAlive())
        Destroy();
}

static MemoryMonitor *currentThis = 0;

void MemoryMonitor::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "monitorMemory") {
        delay = atof(parameters[0].c_str());

        initialMemory = currentMemoryUsage();
        peakMemory = 0;
        
        stop_sleep_function tmp = new pair<int*, sleep_function >
            (&alive, sleep_function(delay, &executeWrapper));
        f_arg = static_cast<void*>(tmp);
        currentThis = this;

        Create();
    }
    if (command == "stopMonitoringMemory") {
        Destroy();
    }
    if (command == "peakMemoryUsage") {
        commandResult = Utils::doubleToString(peakMemory - initialMemory);
    }
}

PUBLIC double MemoryMonitor::currentMemoryUsage()
{
    char buf[10];
    char *space;
    int r;
    int statm_fd;

    statm_fd = open("/proc/self/statm", O_RDONLY);
    if (statm_fd > 0) {
        if ((r = read(statm_fd, buf, sizeof(buf) - 1)) > 0) {
            buf[r] = '\0';
            if ((space = strchr(buf, ' '))) {
                *space = '\0';
                close(statm_fd);
                return (atoi(buf) * getpagesize()) / (1024.0 * 1024.0);
            }
        }
        close(statm_fd);
    } else {
        perror("Error opening /proc/self/statm");
    }
    return 0;
}

PUBLIC void MemoryMonitor::execute()
{
    double usage;
    
    usage = currentMemoryUsage();
    peakMemory = usage > peakMemory ? usage : peakMemory;
}

PUBLIC STATIC void MemoryMonitor::executeWrapper()
{
    currentThis->execute();
}

PUBLIC string MemoryMonitor::shortHelp(const string &command) const
{
    if (command == "monitorMemory")
         return shortHelpLine(command + "(x)", "Commences periodically checking memory usage every x seconds.");
    if (command == "stopMonitoringMemory")
        return shortHelpLine(command, "Stop periodically checking memory usage.");
    if (command == "peakMemoryUsage")
        return shortHelpLine(command, "Get largest memory used between starting and now (or stopping) in MB.");
    return "";
}

PUBLIC string MemoryMonitor::longHelp(const string &command) const
{
    string result = shortHelp(command) + "\n" + "";
    if (command == "monitorMemory")
        return shortHelp(command) + "\n" + Utils::wordWrapString(
            helpIndentSize,
            "Starts a new thread which checks the memory usage.  See stopMonitoringMemory and peakMemoryUsage.  "
            "This command currently requires Linux. Behaviour on non-Linux platforms is undefined.");
    return result;
}
