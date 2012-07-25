// Plug-flow reactor solution methods.
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

#include "pfr.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// pfr methods
// ctor: (default) call flow_reactor ctor
pfr::pfr()
  : flow_reactor()
{}

// ctor: copy
pfr::pfr(const pfr& original)
  : flow_reactor(original)
{}

// dtor: do nothing
pfr::~pfr()
{}

// pfr public methods
// create a copy of this pfr, return pointer to it
reactor*
pfr::copy()
{
  return new pfr(*this);
}

double
pfr::reactor_eqn(model_species* species)
{
  // set up variables to reduce function calls
  double rate(species->get_derivative());
  double yprime(0.0e0);

  // FIXME: put in reactor design equations
  yprime = rate;

  // set the derivative to the calculated value
  species->set_derivative(yprime);
  // return the corrected derivative value
  return yprime;
}

// update an individual gas-phase species
void
pfr::kmc_step(model_species* msp, double dx, double T0, double T1)
{
  // FIXME: put the correct thing here
  return;
}

CH_END_NAMESPACE

/* $Id: pfr.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
