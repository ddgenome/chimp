// Functions for creating, inspecting, and manipulating rate constants.
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

#include <cmath>		// exp() in Arrhenius relationship
#include "compare.h"
#include "k.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// k member functions
// ctor: parameter owned by someone else
k::k(par_expression* k0_)
  : k0(k0_)
{}

// dtor: delete k0
k::~k()
{
  delete k0;
}

// create a new rate constant of the given type and return pointer
k*
k::new_k(const CH_STD::string& type, const par_expression_seq& par_exps)
  throw (bad_input, bad_value)
{
  if (icompare(type, "k") == 0 || icompare(type, "k_constant") == 0)
    {
      if (par_exps.size() != 1)
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":k::new_k(): improper number of parameters "
			  "specified for rate constant of type k");
	}
      return new k(par_exps[0]);
    }
  else if (icompare(type, "k_arrhenius") == 0)
    {
      if (par_exps.size() != 2)
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":k::new_k(): improper number of parameters "
			  "specified for rate constant of type k_arrhenius");
	}
      return new k_arrhenius(par_exps[0], par_exps[1]);
    }
  else if (icompare(type, "k_lfer") == 0)
    {
      if (par_exps.size() != 4)
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":k::new_k(): improper number of parameters "
			  "specified for rate constant of type k_lfer");
	}
      return new k_lfer(par_exps[0], par_exps[1], par_exps[2], par_exps[3]);
    }
  // unknown rate constant type
  return 0;
}

// both of these functions return value of k0 (independent of T)
double
k::get_k() const
{
  return k0->get_value();
}

// default R = 8.314e-3 kJ/molK
double
k::get_k(double T, double R) const
{
  return k0->get_value();
}

// virtual function for proper printing
CH_STD::string
k::stringify() const
{
  return "k_constant(" + k0->stringify() + ")";
}

// k_arrhenius member functions
// ctor: must create parameters yourself
k_arrhenius::k_arrhenius(par_expression* k0_, par_expression* ea_)
  : k(k0_), ea(ea_)
{}

// dtor: delete ea
k_arrhenius::~k_arrhenius()
{
  delete ea;
}

// return the value of the rate constant at the given temperature
// defaults R = 8.314e-3 kJ/molK
double
k_arrhenius::get_k(double T, double R) const
{
  return k::get_k() * CH_STD::exp(-ea->get_value() / (R * T));
}

// virtual function for output
CH_STD::string
k_arrhenius::stringify() const
{
  return "k_arrhenius(" + k0->stringify() + ", " + ea->stringify() + ")";
}

// k_lfer member functions
// ctor: must create parameters yourself
k_lfer::k_lfer(par_expression* k0_, par_expression* e0_,
	       par_expression* gamma_, par_expression* delH_)
  throw (bad_value)
  : k(k0_), e0(e0_), gamma(gamma_), delH(delH_)
{
  // make sure gamma is between zero and one
  double g(gamma->get_value());
  if (g < 0.0e0 || g > 1.0e0)
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":k_lfer::k_lfer(): invalid transfer coefficient "
		      "value: " + t_string(gamma));
    }
}

// dtor: delete the par_expression pointers
k_lfer::~k_lfer()
{
  delete e0;
  delete gamma;
  delete delH;
}

// return value of rate constant at temperature T
// defaults R = 8.314 kJ/molK
double
k_lfer::get_k(double T, double R) const
{
  double Hrxn(delH->get_value());
  double ea(e0->get_value() +  gamma->get_value() * Hrxn);
  // make sure activation energy is valid
  ea = (ea < 0.0e0) ? 0.0e0 : ea;
  ea = (ea < Hrxn) ? Hrxn : ea;
  // calculate the rate constant
  return k::get_k() * CH_STD::exp(-ea / (R * T));
}

// virtual function for output
CH_STD::string
k_lfer::stringify() const
{
  return "k_lfer(" + k0->stringify() + ", " + e0->stringify() + ", "
    + gamma->stringify() + ", " + delH->stringify() + ")";
}

CH_END_NAMESPACE

/* $Id: k.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
