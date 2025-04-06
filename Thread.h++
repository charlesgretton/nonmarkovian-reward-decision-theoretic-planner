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

/* 
 * \paragraph{:Purpose:}
 *
 * The user shall be able to request that this program account for
 * algorithm activity. Accounting consists of a user, requesting that
 * for a defined interval of time, given in seconds, the program
 * inform them of the value of some application data (see
 * \module{MeasurementThreads}). The generic encapsulation of
 * necessary pthreads functionality that is required to achieve this
 * task is provided by this module.
 * */

#ifndef THREAD
#define THREAD

#include<map>
#include<pthread.h>

using namespace std;

/*A \class{pair}, where the first element is a period of $time$ to sleep
 *and the second a function that is called every time that $time$
 *elapses. */
typedef pair<double, void(*)()> sleep_function;

/*A \class{pair}, where the first element is a pointer to an integer
 *boolean and the second is a \class{sleep_function}*/
typedef pair<int*, sleep_function >* stop_sleep_function;

/*All application threads, excepting the main thread, have a
 *sleep--check--execute cycle. This function comprises the sleep--check
 *component of this functionality. The \argument{arg} provides
 *the execute step (see \class{stop_sleep_function}).*/
void* genericThread(void* arg);

/*A thread for our purposes is a reporting function that executes
 *independently of application algorithms (see \class{Algorithm} for
 *an example). This idea is what we encapsulated in the following
 *class.*/
class Thread
{
public:
    /*Create a thread. The thread does not begin execution until
     *\member{Create()} is called.*/
    Thread();

    /*Create a thread with a \member{delay} corresponding to the
     *\argument{delay}.*/
    Thread(double delay);
    
    /*\member{Destroy()} and then delete this thread. If the thread is
     *executing then it is asked to stop by a call to
     *\member{Destroy()}.*/
    ~Thread();

    /*Begin thread execution.*/
    int Create();

    /*Stop thread execution. This requests that this \class{Thread}
     *cease execution, however the calling thread shall pause until
     *this occurs.*/
    int Destroy();

    /*Is this thread alive?*/
    bool isAlive()const;

    /*Some $n$ second delay that can be used by children as an
     *argument to the thread that is created (see
     *\member{Create()}).*/
    double getDelay()const;

    /*Some $n$ second delay that can be used by children as an
     *argument to the thread that is created (see
     *\member{Create()}).*/
    void setDelay(const double&);
protected:
    /*Function that the thread calls when a call to \member{Create} is
     *made.*/
    void* (*f)(void*);

    /*Argument that should include a termination condition.*/
    void* f_arg;
    
    /*Is this thread alive?*/
    int alive;

    /*Some $n$ second delay that can be used by children as an
     *argument to the thread that is created (see \member{Create()}).*/
    double delay;
private:
    /*Ensure that a \class{Thread} cannot be copied.*/
    Thread& operator=(const Thread&);

    /*Posix thread.*/
    pthread_t thread;

    /*Thread attributes*/
    pthread_attr_t attr;
};
#endif
