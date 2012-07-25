// -*- C++ -*-
// Class declarations for manipulation of parameters.
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
#ifndef CH_PARAMETER_H
#define CH_PARAMETER_H 1

#include <cfloat>		// DBL_MAX for bounds
#include <map>
#include <string>
#include <vector>
#include <utility>		// for pair<>
#include "except.h"
#include "unique.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class for constant parameters
class parameter
{
  CH_STD::string name;		// the name of the parameter
  double value;			// actual current value of parameter
  static unique name_list;	// keep track of all parameter names

private:
  // prevent copy construction and assignment
  parameter(const parameter&);
  parameter& operator=(const parameter&);
public:
  // ctor: (default) without name (a unique name is created)
  explicit parameter(double value_ = 0.0e0);
  // ctor: with name (not optional)
  explicit parameter(const CH_STD::string& name_, double value_ = 0.0e0);
  // dtor: delete name from list of used names
  virtual ~parameter();

  // return the name of parameter
  CH_STD::string get_name() const;
  // return value of the parameter
  virtual double get_value() const;
  // set the value of the parameter, return previous value
  double set_value(double value_);
  // return name of parameter
  CH_STD::string stringify() const;
}; // end class parameter

// set up containers and iterators for parameter
typedef CH_STD::vector<parameter*> parameter_seq;
typedef parameter_seq::iterator parameter_seq_iter;
typedef parameter_seq::const_iterator parameter_seq_citer;
typedef CH_STD::map<CH_STD::string,parameter*> parameter_map;
typedef parameter_map::iterator parameter_map_iter;
typedef parameter_map::const_iterator parameter_map_citer;

// class for optimizable parameters
class opt_parameter : public parameter
{
  double orig_value;		// original value of parameter
  double lower;			// lower bound for parameter
  double upper;			// upper bound for parameter

private:
  // prevent copy construction and assignment
  opt_parameter(const opt_parameter&);
  opt_parameter& operator=(const opt_parameter&);
public:
  // ctor: (default) optional value and bounds
  explicit opt_parameter(double value_ = 0.0e0, double lower_ = -DBL_MAX,
			 double upper_ = DBL_MAX)
    throw (bad_value); // set_value()
  // ctor: user-supplied name
  explicit opt_parameter(const CH_STD::string& name_, double value_ = 0.0e0,
			 double lower_ = -DBL_MAX, double upper_ = DBL_MAX)
    throw (bad_value); // set_value()
  // dtor: do nothing
  virtual ~opt_parameter();

  // return original value of the parameter
  double get_original_value() const;
  // return value of lower bound
  double get_lower_bound() const;
  // return value of upper bound
  double get_upper_bound() const;
  // change the value of the parameter, return old value
  virtual double set_value(double value_)
    throw (bad_value); // this
  // change the value, set to bound if value is out of bounds, return value used
  virtual double set_value_bounds(double value_);
}; // end class opt_parameter

// class for using the logarithm of the parameter (for optimization)
class log_parameter : public opt_parameter
{
  int sign;			// log of a negative number?  Why, yes.
  double log_value;		// log of parameter::value (for speed)

private:
  // prevent copy construction and assignment
  log_parameter(const log_parameter&);
  log_parameter& operator=(const log_parameter&);
  // safely return sign of value_ and log(value_)
  static CH_STD::pair<int,double> take_log(double value_)
    throw (bad_value); // this
public:
  // ctor: (default) optional value and bounds
  explicit log_parameter(double value_ = 1.0e0, double lower_ = 0.0e0,
			 double upper_ = DBL_MAX)
    throw (bad_value); // set_value()
  // ctor: with name and optional value and bounds
  explicit log_parameter(const CH_STD::string& name_, double value_ = 1.0e0,
			 double lower_ = 0.0e0, double upper_ = DBL_MAX)
    throw (bad_value); // set_value()
  // dtor: do nothing
  virtual ~log_parameter();

  // return value of the parameter
  virtual double get_value() const;
  // return the sign and log value of the parameter
  CH_STD::pair<int,double> get_log_value() const;
  // change the value of the parameter, return old value
  virtual double set_value(double value_)
    throw (bad_value); // this
  // change the value, set to bound if value is out of bounds, return old value
  virtual double set_value_bounds(double value_)
    throw (bad_value); // take_log()
  // set the log value of the parameter, return old log value
  CH_STD::pair<int,double> set_log_value(double log_value_, int sign_ = 1)
    throw (bad_value); // parameter::set_value()
  // set the log value of parameter if within bounds, return current log value
  CH_STD::pair<int,double> set_log_value_bounds(double log_value_,
                                                int sign_ = 1);
}; // end class log_parameter

// abstract base class for the creation of complex parameters
class par_expression
{
private:
  // prevent copy construction and assignment
  par_expression(const par_expression&);
  par_expression& operator=(const par_expression&);
public:
  // ctor: do nothing
  par_expression();
  // dtor: do nothing
  virtual ~par_expression();

  // return value of parameter expression
  virtual double get_value() const = 0;
  // return string of parameter expression
  virtual CH_STD::string stringify() const = 0;
}; // end class par_expression

// set up a sequence of par_expression's
typedef CH_STD::vector<par_expression*> par_expression_seq;
typedef par_expression_seq::iterator par_expression_seq_iter;
typedef par_expression_seq::const_iterator par_expression_seq_citer;

// class which hold a single par_expression
class par_single : public par_expression
{
  parameter* par;		// the base of parameter tree

private:
  // prevent copy construction and assignment
  par_single(const par_single&);
  par_single& operator=(const par_single&);
public:
  // ctor: user-supplied parameter
  explicit par_single(parameter* par_);
  // dtor: do nothing, do not own parameters
  virtual ~par_single();

  // return value of par
  virtual double get_value() const;
  // return string of parameter expression
  virtual CH_STD::string stringify() const;
}; // end class par_single

// class for unary negation of a parameter
class par_minus : public par_expression
{
  par_expression* positive;	// object of unary minus' action

private:
  // prevent copy construction and assignment
  par_minus(const par_minus&);
  par_minus& operator=(const par_minus&);
public:
  // ctor: user-supplied expression
  par_minus(par_expression* positive_);
  // dtor: delete positive
  virtual ~par_minus();

  // return negated value of positive
  virtual double get_value() const;
  // return string of parameter expression
  virtual CH_STD::string stringify() const;
}; // end par_minus

// class for a sum of two par_expression's
class par_sum : public par_expression
{
  par_expression* left;		// expression on lhs of plus sign
  par_expression* right;	// expression on rhs of plus sign

private:
  // prevent copy construction and assignment
  par_sum(const par_sum&);
  par_sum& operator=(const par_sum&);
public:
  // ctor: user-supplied expression
  par_sum(par_expression* left_, par_expression* right_);
  // dtor: delete left and right
  virtual ~par_sum();

  // return sum of the values of the two expressions
  virtual double get_value() const;
  // return string of parameter expression
  virtual CH_STD::string stringify() const;
}; // end class par_sum

// class for a difference of two par_expression's
class par_difference : public par_expression
{
  par_expression* left;		// expression on lhs of minus sign
  par_expression* right;	// expression on rhs of minus sign

private:
  // prevent copy construction and assignment
  par_difference(const par_difference&);
  par_difference& operator=(const par_difference&);
public:
  // ctor: user-supplied expression
  par_difference(par_expression* left_, par_expression* right_);
  // dtor: delete left and right
  virtual ~par_difference();

  // return difference of the values of the two expressions
  virtual double get_value() const;
  // return string of parameter expression
  virtual CH_STD::string stringify() const;
}; // end class par_difference

// class for a product of two par_expression's
class par_product : public par_expression
{
  par_expression* left;		// expression on lhs of mult sign
  par_expression* right;	// expression on rhs of mult sign

private:
  // prevent copy construction and assignment
  par_product(const par_product&);
  par_product& operator=(const par_product&);
public:
  // ctor: user-supplied expression
  par_product(par_expression* left_, par_expression* right_);
  // dtor: delete left and right
  virtual ~par_product();

  // return product of the values of the two expressions
  virtual double get_value() const;
  // return string of parameter expression
  virtual CH_STD::string stringify() const;
}; // end class par_product

// class for a ratio of two par_expression's
class par_ratio : public par_expression
{
  par_expression* numerator;	// expression on top of division sign
  par_expression* denominator;	// expression on bottom of division sign

private:
  // prevent copy construction and assignment
  par_ratio(const par_ratio&);
  par_ratio& operator=(const par_ratio&);
public:
  // ctor: user-supplied expression
  par_ratio(par_expression* numerator_, par_expression* denominator_);
  // dtor: delete numerator and denominator
  virtual ~par_ratio();

  // return ratio of the values of the two expressions
  virtual double get_value() const
    throw(bad_value); // this
  // return string of parameter expression
  virtual CH_STD::string stringify() const;
}; // end class par_ratio

// class to raise a parameter to the power given by the other parameter
class par_pow : public par_expression
{
  par_expression* base;		// base (root) of the power
  par_expression* exponent;	// exponent (power) of the expression

private:
  // prevent copy construction and assignment
  par_pow(const par_pow&);
  par_pow& operator=(const par_pow&);
public:
  // ctor: user-supplied base and exponent
  par_pow(par_expression* base_, par_expression* exponent_);
  // dtor: delete base and exponent
  ~par_pow();

  // return (base)^(exponent)
  virtual double get_value() const
    throw(bad_value); // this
  virtual CH_STD::string stringify() const;
}; // end class par_pow

CH_END_NAMESPACE

#endif // not CH_PARAMETER_H

/* $Id: parameter.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
