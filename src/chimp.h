// -*- C++ -*-
// CHIMP - CHIMP HIerarchical Modeling Program.
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
#ifndef CH_CH_H
#define CH_CH_H 1

#include <iostream>
#include <string>


// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// function declarations
// return the GNITS compliant version string
CH_STD::string version();
// print GNITS compliant usage to standard out (cout)
void usage(CH_STD::ostream& os, const CH_STD::string& program);
// return the GPL copyright stuff
CH_STD::string gpl_rant();

CH_END_NAMESPACE

#endif // not CH_CH_H

/* $Id: chimp.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
