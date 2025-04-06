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

#include"Thread.h++"
#include "CommandInterpreter.h++"

#include<unistd.h>
#include<cassert>
#include<iostream>

using namespace std;

void* genericThread(void* arg)
{
    stop_sleep_function arguments
        = static_cast<stop_sleep_function>(arg);
    
    while(*arguments->first)/*Check*/
    {
        usleep(int(arguments->second.first * 1000000.0));/*Sleep*/

        /*Has the interface been paused?*/
        if(!CommandInterpreter::getInstance()->isInterrupted())
            arguments->second.second();/*Execute*/
        else/*If it has the thread shall have to be restarted.*/
            *arguments->first = false;
        
    }
    
    return 0;
}

Thread::Thread() :
    f(&genericThread), f_arg(0), alive(0), delay(0), thread(0)
{
    pthread_attr_init(&attr);
}

Thread::Thread(double delay) :
    f(&genericThread), f_arg(0), alive(0), delay(delay), thread(0)
{
    pthread_attr_init(&attr);
}

Thread::~Thread()
{
  if (alive)
  {
    Destroy();
    pthread_attr_destroy(&attr);
  }
}

int Thread::Create()
{
    alive = 1;
    
    if(pthread_create(&thread,0,f, f_arg))
        {
            cerr<<"Thread could not be created\n";assert(0);
        }
    
    return static_cast<int>(thread);
}

int Thread::Destroy()
{
    alive = 0;
    
    if(pthread_join(thread, 0))
        
        {
            cerr<<"Thread could not be destroyed\n";assert(0);
        }
    
    return alive;
}

bool Thread::isAlive()const
{
    return alive;
}

double Thread::getDelay()const
{
    return delay;
}

void Thread::setDelay(const double& d)
{
    delay = d;
}
