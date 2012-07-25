// Assignment of useful constants.
// Copyright (C) 2004 David Dooling <banjo@users.sourceforge.net>
//
// This file is part of CHIMP.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "constant.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

const double constant::avogadro = 6.0221367e23; // [molecules / mole]
const double constant::pi = 3.1415926535897932e0; // [radians / 180deg]
const double constant::r = 8.314510e0; // [J/(mol K)] or [m^3*Pa/(mol*K)]
const double constant::k = 1.380657e-23; // [J/K]

CH_END_NAMESPACE

/* $Id: constant.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
