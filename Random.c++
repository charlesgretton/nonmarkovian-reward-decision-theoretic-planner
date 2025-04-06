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

/* Random.c++
 */

#include "common.h++"
#ifdef HEADER

class Random
{
private:
};

#endif
#include "Random.h++"
#include <cstdlib>
#include <math.h>

PUBLIC STATIC int Random::randomInt(int lower, int upper)
{
    return int(floor(randomProb() * (upper - lower + 1)) + lower);
}

/* Produce a random number in the range [0..1) */
PUBLIC STATIC double Random::randomProb()
{
    return double(random()) / (RAND_MAX - 1);
}

PUBLIC bool Random::randomBool()
{
    return rand() > RAND_MAX / 2;
}
