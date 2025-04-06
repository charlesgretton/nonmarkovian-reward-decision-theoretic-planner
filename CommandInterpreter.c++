
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

/* CommandInterpreter.c++
 *
 * \paragraph{:Purpose:}
 *
 * Provides a command line interface for the user to interact with the
 * system.
 */

#include "common.h++"
#ifdef HEADER

#include "CommandListener.h++"

#include <cstdio>
#include <stack>
#include <string>
using namespace std;

#include "domainSpecification.h++"
using namespace MDP;

class ExtADD;

class CommandInterpreter : public CommandListener
{
public:
    static CommandInterpreter *getInstance()
    {
        if (!instance)
            instance = new CommandInterpreter;
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
    string history_file;
    bool quit;

    struct InputFile 
    {
        string filename;
        FILE *file;
        int lineNumber;
        int offset;
    };
    
    stack<struct InputFile> inputFiles;

    /*Application domain specification
     *(\module{domainSpecification}).
     *
     *\textbf{NOTE:} Changes in the application domain specification
     *do not reflect a state change in this interpreter.*/
    mutable DomainSpecification *domSpec;

    /* Automatic constraint (for structured algorithms) */
    ExtADD *automaticConstraint;
};

#endif

#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cerrno>

extern "C" {
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>
}
    
#include <vector>
#include <iostream>
#include <string>
using namespace std;

#include "Registry.h++"
#include "CommandInterpreter.h++"
#include "commandParser.h++"
#include "Utils.h++"
#include "ExtADD.h++"

extern HIST_ENTRY **history_list();

extern int commandyyparse();

static void interrupt(int sig)
{
    CommandInterpreter::getInstance()->setInterrupted(true);
}

static char *commandGenerator(const char *text, int state)
{
    return Registry::getInstance()->commandGenerator(text, state);
}

static char **commandCompletion(const char *text, int start, int end)
{
    return rl_completion_matches(text, commandGenerator);
}

PRIVATE STATIC CommandInterpreter *CommandInterpreter::instance = 0;

/*This has the effect of deleting any previous \member{domSpec} and
 *replacing it with a "clean slate" specification.*/
PRIVATE void CommandInterpreter::configureDomainSpecification()
{
    if(0 != automaticConstraint) {
        delete automaticConstraint;
        automaticConstraint = NULL;
    }
    if(0 != domSpec) {
        delete domSpec;
        domSpec = NULL;
    }
    
    domSpec = new DomainSpecification();
    automaticConstraint = new ExtADD(domSpec->getActionSpecification()->getManager().addOne());
}

PRIVATE CommandInterpreter::CommandInterpreter()
    : quit(false)
{
#ifndef __CSIGNAL__
    sigset(SIGTSTP, &interrupt);
#else
    signal(SIGTSTP, &interrupt);
#endif
    
    //Setup the initial application domain specification.
    domSpec = 0;
    automaticConstraint = 0;
    configureDomainSpecification();

    using_history();

    rl_completion_append_character = '\0';
    rl_attempted_completion_function = &commandCompletion;

    history_file = string(getenv("HOME")) + "/.nmrdpp.history";

    Registry::getInstance()->setFunction("quit", this, 0);
    Registry::getInstance()->setFunction("exit", this, 0);
    Registry::getInstance()->setFunction("help", this, 1);
    Registry::getInstance()->setFunction("include", this, 1);
    Registry::getInstance()->setFunction("continue", this, 0);
    Registry::getInstance()->setFunction("clear", this, 0);
    Registry::getInstance()->setFunction("stop", this, 0);

    read_history(history_file.c_str());
}

PUBLIC VIRTUAL CommandInterpreter::~CommandInterpreter()
{
    signal(SIGTSTP, SIG_DFL);
    
    write_history(history_file.c_str());

    Registry::getInstance()->unregister("quit", this);
    Registry::getInstance()->unregister("exit", this);
    Registry::getInstance()->unregister("help", this);
    Registry::getInstance()->unregister("include", this);
    Registry::getInstance()->unregister("continue", this);
    Registry::getInstance()->unregister("clear", this);
    Registry::getInstance()->unregister("stop", this);

    if(0 != automaticConstraint) {
        delete automaticConstraint;
        automaticConstraint = NULL;
    }
    if(0 != domSpec) {
        delete domSpec;
        domSpec = NULL;
    }
    domSpec = NULL;
}

PUBLIC void CommandInterpreter::start()
{
    commandyyparse();
}

PUBLIC string CommandInterpreter::longHelp(const string &command) const
{
    if (command == "quit" || command == "exit")
        return shortHelp(command) + "\n";
    else if (command == "help")
        return shortHelp(command) + "\n" + Utils::wordWrapString(helpIndentSize,
            "Without a parameter - displays list of commands and a terse "
            "help string for each.  Variables, and their values are also displayed.\n\n"
            "Given a parameter - displays a possibly more verbose help on the "
            "given command.");
    else if (command == "include")
        return shortHelp(command) + "\n"
            "  Returns to current file or command line when done.\n";
    else if (command == "continue")
        return shortHelp(command) + "\n";
    else if (command == "stop")
        return shortHelp(command) + "\n";
    else
        return shortHelp(command);
}

PUBLIC string CommandInterpreter::shortHelp(const string &command) const
{
    if (command == "quit" || command == "exit")
        return shortHelpLine(command, "Exit from application.");
    else if (command == "help")
        return shortHelpLine("help [commandname]", "Display help message for a command (optional).");
    else if (command == "include")
        return shortHelpLine("include(filename)", "Read input from specified file.");
    else if (command == "continue")
        return shortHelpLine(command, "Continue execution of interrupted operation.");
    else if (command == "stop")
        return shortHelpLine(command, "Terminate interrupted operation.");
    else if (command == "clear")
        return shortHelpLine(command, "Clear the current domain.");
    return "";
}

PUBLIC void CommandInterpreter::commandIssued(const string &command, const vector<string> &parameters)
{
    commandResult = "";
    if (command == "quit" || command == "exit") {
        quit = true;
    } else if (command == "help") {
        commandResult = Registry::getInstance()->help(parameters[0]);
    } else if (command == "include") {
        include(parameters[0]);
    } else if (command == "continue") {
            Registry::getInstance()->resume();
    } else if (command == "stop") {
        Registry::getInstance()->stop();
    }
    else if (command == "clear")
    {
        configureDomainSpecification();
    }   
}

PUBLIC int CommandInterpreter::getLine(string &result)
{
    char *line;

    if (quit)
        return 1;

    if (inputFiles.empty() || Registry::getInstance()->isInterrupted()) {
        /* interactive input */
        line = readline("> ");
        if (!line) {
            cout << "\n";
            return 1;
        }
        if (strlen(line) > 0)
            add_history(line);

        result = string(line) + "\n";
        free(line);
        line = NULL;
    } else {
        /* input from file */
        inputFiles.top().lineNumber++;
        char ch;
        FILE *from = inputFiles.top().file;
        while (true) {
            // We keep track of our offset in the input file, since
            // somehow something is trashing it (seems to happen when
            // we call the other parser - ie loadWorld.  This is an
            // ugly hack, and should be removed when whatever is
            // causing this problem is found and fixed
            fseek(inputFiles.top().file, inputFiles.top().offset, SEEK_SET);
            ch = fgetc(from);
            inputFiles.top().offset = ftell(inputFiles.top().file);
            if (ch == EOF) {
                fclose(from);
                inputFiles.pop();
                break;
            }
            if (ch != '\r')
                result += ch;
            if (ch == '\n')
                break;
        }
    }

    return 0;
}

PUBLIC void CommandInterpreter::output(const string &filename, const string &data, bool append) const
{
    int fd;
    int flags = O_WRONLY | O_CREAT;

    if (append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    fd = open(filename.c_str(), flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        perror("error opening output file");
        return;
    }
    write(fd, data.c_str(), data.length());
    write(fd, "\n", 1);
    close(fd);
}

PUBLIC void CommandInterpreter::shell(const string &command, const string &data) const
{
    int chStdin[2];
    int chpid;

    pipe(chStdin);

    chpid = fork();

    if (!chpid) {
        signal(SIGTSTP, SIG_IGN);
            
        close(chStdin[1]);
        close(0);
        dup(chStdin[0]);
        system(command.c_str());
        exit(0);
    }
    close(chStdin[0]);
    write(chStdin[1], data.c_str(), data.length());
    write(chStdin[1], "\n", 1);
    close(chStdin[1]);
    waitpid(chpid, 0, 0);
}

PUBLIC FILE *CommandInterpreter::nullInput() const
{
    static FILE *devNull;
    
    if (!devNull)
        devNull = fopen("/dev/null", "r");
    return devNull;
}

PUBLIC void CommandInterpreter::setInterrupted(bool newVal)
{
    if (newVal)
        Registry::getInstance()->interrupt();
}

PUBLIC bool CommandInterpreter::isInterrupted()
{
   return Registry::getInstance()->isInterrupted();
}

/*Get the current \member{domSpec}.*/
PUBLIC DomainSpecification *CommandInterpreter::getDomSpec() const
{
    return domSpec;
}

PRIVATE void CommandInterpreter::include(const string &filename)
{
    InputFile newFile;
    newFile.file = fopen(filename.c_str(), "r");
    if (newFile.file == NULL) {
        cerr << "Error opening \""
             << filename << "\": "
             << Utils::errorToString(errno) << endl;
    } else {
        newFile.lineNumber = 0;
        newFile.filename = filename;
        newFile.offset = 0;
        inputFiles.push(newFile);
    }
}

PUBLIC void CommandInterpreter::loadFile(const string &filename)
{
    include(filename);
}

/* Report an error on the line last read */
PUBLIC void CommandInterpreter::reportError(const string& error)
{
    if (inputFiles.empty() || Registry::getInstance()->isInterrupted()) {
        /* interactive input */
        cerr << error << endl;
    } else {
        /* input from file */
        cerr << inputFiles.top().filename
             << ":" << inputFiles.top().lineNumber
             << ": " << error << endl;
    }
}

PUBLIC ExtADD CommandInterpreter::getAutomaticConstraint() const
{
    return *automaticConstraint;
}
    
PUBLIC void CommandInterpreter::setAutomaticConstraint(ExtADD val)
{
    *automaticConstraint = val;
}
    
