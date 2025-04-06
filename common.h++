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

/* This header is intended to be common throughout the system - ie,
 * have everything include it.
 */

#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
using namespace std;

#define TODO cerr << "TODO: " __FILE__ ": " << __LINE__ << endl

/* Ignore default macro - value is extracted and used in the
 * generation of the header. */
#define DEFVAL(x)

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL

#endif
