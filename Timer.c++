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

/* Timer.c++
 *
 * \paragraph{:Purpose:}
 *
 * Provides a timer that acts somewhat like a stop-watch.
 */

#include "common.h++"
#ifdef HEADER

#include<ctime>

class Timer
{
private:
    clock_t startTime, finishTime;
    double time;
};

#endif

#include"Timer.h++"

PUBLIC Timer::Timer()
{
    reset();
}

PUBLIC void Timer::start()
{
    startTime = clock();
}

PUBLIC void Timer::reset()
{
    time = 0.0;
}

PUBLIC void Timer::stop()
{
    finishTime = clock();
    time += ((double)(finishTime - startTime)/CLOCKS_PER_SEC);
}

PUBLIC double Timer::read()const
{
    return time;
}
