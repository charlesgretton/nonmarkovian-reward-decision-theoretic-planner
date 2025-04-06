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

/* Registry.c++
 *
 * \paragraph{:Purpose:}
 *
 * Acts as a repository for commands, functions and variables
 * available from the command line.  Modules that wish to provide
 * commands, register with the registry in order to receive call-backs
 * when their command or function is executed.
 *
 *
 * NOTE: This module also supports multicast.
 */

#include "common.h++"
#ifdef HEADER

#include "RegistryItemFunction.h++"
#include "RegistryItemString.h++"

#include <iostream>
#include <map>
#include <string>
using namespace std;

class Registry : public CommandListener
{
public:
    static Registry *getInstance()
    {
        if (!instance)
            instance = new Registry;
        return instance;
    }
    
    static void destroyInstance()
    {
        if (instance) {
            delete instance;
            instance = 0;
        }
    }
    
private:
    multimap<string, RegistryItem *> data;

    int len;
    map<string, RegistryItem *>::const_iterator listPosition;

    vector<pair<string, RegistryItem *> > jobs;

    bool interrupted;
};

#endif

#include<deque>
#include "Registry.h++"
#include "cassert"

PRIVATE STATIC Registry *Registry::instance = 0;

/* Prevent creation, since we are a singleton */
PRIVATE Registry::Registry()
    : interrupted(false)
{
    setFunction("jobs", this, 0);
}

PUBLIC Registry::~Registry()
{
    Registry::getInstance()->unregister("jobs", this);
    for (map<string, RegistryItem *>::iterator i = data.begin(); i != data.end(); i++) {
        delete i->second;
    }
}

/* Associate `value' with `key' */
PUBLIC void Registry::setString(const string &key, const string &value)
{
    if(data.find(key) != data.end())
    {
        delete data.find(key)->second;
        data.erase(key);
    }
    
    data.insert(pair<string, RegistryItem*>(key, new RegistryItemString(value)));
    // CHARLES :: Sat May 17 13:47:40 EST 2003 :: data[key] = new RegistryItemString(value);
}

/* Register `listener' for call-back when the command `key' is
 * executed.  The command expects the specified number of
 * parameters.
 *
 *IMPORTANT :: Keep in mind that the STL multimap is used as the
 *multicast datastructure, thus the order in which registration is
 *made is important.
 */
PUBLIC void Registry::setFunction(const string &key,
                           CommandListener *listener,
                           int numParameters)
{
    data.insert(pair<string, RegistryItem*>(key, new RegistryItemFunction(listener, numParameters)));
    // CHARLES :: Sat May 17 13:47:40 EST 2003 :: data[key] = new RegistryItemFunction(listener, numParameters);
}

/* Unregister the command (or value) associated with `key'.  This
 * should be called when a command listener is going out of scope, or
 * otherwise being destroyed in order to avoid the command listener
 * being called when it doesn't exist. All listeners are unregistered
 * if any listener makes a call.
 *
 * Charles :: This is now deprecated in favour of
 * \function{unregister(const string,CommandListener)}.
 */
PUBLIC void Registry::unregister(const string &key)
{
    for(multimap<string, RegistryItem*>::iterator p = data.lower_bound(key)
            ; (p != data.end()) && p->first <= key
            ; p++)
        delete p->second;
    
    data.erase(key);
    // CHARLES :: Sat May 17 13:47:40 EST 2003 :: delete data[key];
    // CHARLES :: Sat May 17 13:47:40 EST 2003 :: data.erase(data.find(key));
}

/* Unregister the command (or value) associated with `key'.  This
 * should be called when a command \argument{listener} is going out of
 * scope, or otherwise being destroyed in order to avoid the command
 * listener being called when it doesn't exist.
 */
PUBLIC void Registry::unregister(const string &key, CommandListener const* listener)
{
    deque<multimap<string, RegistryItem*>::iterator> toDelete;
        
    for(multimap<string, RegistryItem*>::iterator p = data.lower_bound(key)
            ; (p != data.end()) && p->first <= key
            ; p++)
        if(0 != dynamic_cast<RegistryItemFunction*>(p->second) &&
           dynamic_cast<RegistryItemFunction*>(p->second)->getListener() == listener)
        {
            toDelete.push_back(p);
            delete p->second;
        }
    

    while(!toDelete.empty())
    {        
        data.erase(toDelete.back());
        toDelete.pop_back();
    }
}

/* Get the value of a variable, or the value returned by a function.
 * If the function is suspended, then the value returned will be the
 * empty string, and the actual return value from the function when it
 * finally does finish will be returned by the continue command.
 */
PUBLIC string Registry::evaluate(const string &key, const vector<string> &parameters)
{
    string result;
    interrupted = false;

    /*A command was executed as a result of this call.*/
    bool success = false;
    
    if (data.count(key)) {
        for(multimap<string, RegistryItem*>::iterator p = data.lower_bound(key)
                ; (p != data.end()) && p->first <= key
                ; p++) {
            RegistryItem *currentJob = p->second;
            if (currentJob->beginEvaluation(key, parameters)) {
                pair<string, RegistryItem *> job(key, currentJob);
                jobs.push_back(job);
                result +=  executeCommand();
                success = true;
            } else {
                result += "";
            }
        }
    } else if (key == "false" || key == "true") {
        return key;
    } else {
        return "Undefined symbol `" + key + "'";
    }

    /*If no messages are provided and no function was called.*/
    if("" == result && !success)
        result = "Symbol "+ key +" is defined but usage was erroneous.";
    
    return result;
}

PRIVATE string Registry::executeCommand()
{
    assert(!jobs.empty());
    string result;
    while (true) {
        RegistryItem *currentJob = jobs[jobs.size() - 1].second;
        const string& currentCommand = jobs[jobs.size() - 1].first;
        if (currentJob->evaluate(currentCommand)) {
            result = currentJob->evaluationResult(currentCommand);
            currentJob->endEvaluation(currentCommand);

            jobs.pop_back();
            break;
        }
        if (interrupted)
            break;
    }
    return result;
}

/* Suspend the current command
 */
PUBLIC void Registry::interrupt()
{
    interrupted = true;
}

PUBLIC bool Registry::isInterrupted()
{
   return interrupted;
}

/* Resume a previously suspended command
 */
PUBLIC string Registry::resume()
{
    if (jobs.empty()) {
        cerr << "No job to resume.\n";
        return "";
    } else {
        interrupted = false;
        return executeCommand();
    }
}

/* Stop a previously suspended command
 */
PUBLIC void Registry::stop()
{
    if (jobs.empty()) {
        cerr << "No job to stop.\n";
    } else {
        RegistryItem *currentJob = jobs[jobs.size() - 1].second;
        string currentCommand = jobs[jobs.size() - 1].first;
        currentJob->endEvaluation(currentCommand);
        jobs.pop_back();
        cerr << currentCommand << " terminated.\n";
    }
}

PUBLIC string Registry::help(const string &about)
{
    string result;
    
    if (data.count(about))
    {
        string helpTmp = "";
        for(multimap<string, RegistryItem*>::iterator p = data.lower_bound(about)
                ; (p != data.end()) && p->first <= about
                ; p++)
        {
            string  helpLine = p->second->longHelp(about);
            
            /*Avoid giving the same help twice.*/
            if (helpTmp != helpLine)
                result +=  helpLine;
            helpTmp = helpLine;
        }
    }
     else {
        return "Undefined symbol `" + about + "'";
    }
    
    return result;
}

/* Produce a one line description of each command or variable.
 */
PUBLIC string Registry::helpSummary()
{
    string result;

    result += string("Below all commands are listed followed by a terse\n")
        +string("summary of their purpose. It is important to note\n")
        +string("that multiple application (NMRDPP) components are\n")
        +string("able to respond when a command is requested. In the\n")
        +string("following, a command is listed once for each application\n")
        +string("component that is a registered respondent.")
        +string("")
        +string("\n\n")
        +shortHelpLine("*COMMAND*", "*DESCRIPTION*\n")
        +shortHelpLine("_________", "______________________________\n\n");

    
    string helpTmp = "";
    for (multimap<string, RegistryItem *>::iterator i = data.begin(); i != data.end(); i++) {
        string helpLine = "";
        
        helpLine = i->second->shortHelp(i->first);

        /*If no help was provided.*/
        if (helpLine == "")
            helpLine = "---- No help available for " + i->first + " ----";

        /*Avoid giving the same help twice.*/
        if(helpLine != helpTmp)
            result += helpLine + "\n";
        helpTmp = helpLine;
    }
    
    return result;
}

PUBLIC char *Registry::commandGenerator(const string &text, int state)
{
    if (!state) {
        listPosition = data.begin();
        len = text.size();
    }

    while (listPosition != data.end()) {
        const char *name = listPosition->first.c_str();
        if (strncmp(name, text.c_str(), len) == 0) {
            listPosition++;        
            return strdup(name);
        }
        
        listPosition++;        
    }

    return ((char *)NULL);
}

PUBLIC string Registry::jobList()
{
    string result;
    vector<pair<string, RegistryItem *> >::const_iterator i;
    for (i = jobs.begin(); i != jobs.end(); i++) {
        result += i->first + "\n";
    }
    
    return result;
}

PUBLIC string Registry::longHelp(const string &command) const
{
    if (command == "jobs")
        return shortHelp(command) + "\n";
    return "";
}

PUBLIC string Registry::shortHelp(const string &command) const
{
    if (command == "jobs")
        return shortHelpLine(command, "Show list of commands currently executing.");
    return "";
}

PUBLIC void Registry::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "jobs")
        commandResult = Registry::getInstance()->jobList();
}

/* Does the registry have an entry for `key'?
 */
PUBLIC bool Registry::hasKey(const string& key)
{
    return data.count(key) > 0;
}

/* Get a boolean value from the registry entry `key', or return
 * `def' if `key' is not defined
 */
PUBLIC bool Registry::getBool(const string& key, bool def)
{
    if (hasKey(key)) {
        string tmp = evaluate(key, vector<string> ());
        if (tmp == "off" || tmp == "false")
            return false;
        if (tmp == "on" || tmp == "true")
            return true;
        cerr << key << " expected to have value on|off|true|false\n";
    }
    return def;
}

PUBLIC string Registry::getString(const string& key, const string& def)
{
    if (hasKey(key)) {
        return evaluate(key, vector<string> ());
    }
    return def;
}
