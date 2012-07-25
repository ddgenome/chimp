// -*- C++ -*-
// Description of plug-flow reactor solution class.
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
#ifndef CH_MODEL_PFR_H
#define CH_MODEL_PFR_H 1

#include "except.h"
#include "reactor.h"
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class for solution of plug-flow reactor problems
class pfr : public flow_reactor
{
public:
  // ctor: (default) call flow_reactor ctor
  pfr();
  // ctor: copy
  explicit pfr(const pfr& original);
  // dtor: do nothing
  virtual ~pfr();

  // create a copy of a pfr, return pointer to it
  virtual reactor* copy();
  // modify derivative according to the reactor design equations
  virtual double reactor_eqn(model_species* species);
  // update an individual gas-phase species
  virtual void kmc_step(model_species* msp, double dx, double T0, double T1);
}; // end class pfr

CH_END_NAMESPACE

#endif // not CH_MODEL_PFR_H

/* $Id: pfr.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
