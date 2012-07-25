// -*- C++ -*-
// Declaration of classes for reaction rate constants.
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
#ifndef CH_K_H
#define CH_K_H 1

#include <string>
#include "constant.h"
#include "except.h"
#include "parameter.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// rate constant with no temperature dependence
class k
{
protected:
  // for derived class stringify()
  par_expression* k0;		// rate _constant_

private:
  // prevent copy construction and assignment
  k(const k&);
  k& operator=(const k&);
public:
  // ctor: no default
  // ctor: parameter owned by someone else
  explicit k(par_expression* k0_);
  // dtor: delete k0
  virtual ~k();

  // create a rate constant of the given type with the given par_expressions
  static k* new_k(const CH_STD::string& type,
		  const par_expression_seq& par_exps)
    throw (bad_input, bad_value); // this, k_lfer()
  // both of these functions return value of k0 (independent of T)
  double get_k() const;
  virtual double get_k(double T, double R = constant::r) const;

  // virtual function for output
  virtual CH_STD::string stringify() const;
}; // end class k

// use a typedef so we can distinguish between the base class and
// the rate constant which is independent of temperature
typedef k k_constant;

// rate constant with Arrhenius temperature dependence
class k_arrhenius : public k
{
  par_expression* ea;		// activation energy

private:
  // prevent copy sontruction and assignment
  k_arrhenius(const k_arrhenius&);
  k_arrhenius& operator=(const k_arrhenius&);
public:
  // ctor: no default
  // ctor: must create parameters yourself
  k_arrhenius(par_expression* k0_, par_expression* ea_);
  // dtor: delete ea
  virtual ~k_arrhenius();

  // return value of rate constant at T
  virtual double get_k(double T, double R = constant::r ) const;

  // virtual function for output
  virtual CH_STD::string stringify() const;
}; // end class k_arrhenius

// rate constant using linear free energy relationship (lfer)
// this could be done using par_expressions, but this class provides
// additional checks to make sure thermodynamic consistency is
// maintained
class k_lfer : public k
{
  par_expression* e0;		// intrinsic activation barrier
  par_expression* gamma;	// transfer coefficient
  par_expression* delH;		// enthalpy change upon reaction

private:
  // prevent copy construction and assignment
  k_lfer(const k_lfer&);
  k_lfer& operator=(const k_lfer&);
public:
  // ctor: no default
  // ctor: must create par_expressions yourself
  k_lfer(par_expression* k0_, par_expression* e0_, par_expression* gamma_,
	 par_expression* delH_)
    throw (bad_value); // this
  // dtor: delete the par_expression pointers
  virtual ~k_lfer();

  // return value of rate constant at T
  virtual double get_k(double T, double R = constant::r ) const;

  // virtual function for output
  virtual CH_STD::string stringify() const;
}; // end class k_lfer

CH_END_NAMESPACE

#endif // not CH_K_H

/* $Id: k.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
