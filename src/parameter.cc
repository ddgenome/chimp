// Functions for creating, inspecting, and manipulating parameters.
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

#include <cmath>
#include "t_string.h"
#include "parameter.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initialize parameter static members, allow multiple instances with same name
unique parameter::name_list("par", true);

// parameter methods
// ctor: (default) create name for parameter
// ctor: defaults value_ = 0.0e0
parameter::parameter(double value_)
  : name(name_list.insert()), value(value_)
{}

// ctor: with name given
// ctor: defaults value_ = 0.0e0
parameter::parameter(const CH_STD::string& name_, double value_)
  : name(name_), value(value_)
{
  name_list.insert(name);
}

// dtor: erase name from list of used names
parameter::~parameter()
{
  name_list.remove(name);
}

// parameter public methods
// return the name of parameter
CH_STD::string
parameter::get_name() const
{
  return name;
}

// return value of the parameter
double
parameter::get_value() const
{
  return value;
}

// set the value and return old value
double
parameter::set_value(double value_)
{
  double old(value);
  value = value_;
  return old;
}

// return name of parameter for stringification
CH_STD::string
parameter::stringify() const
{
  return name;
}

// opt_parameter methods
// ctor: (default) optional value and bounds
// ctor: defaults value_ = 0.0e0, lower_ = -DBL_MAX,
// ctor:          upper_ = DBL_MAX
opt_parameter::opt_parameter(double value_, double lower_,
			     double upper_)
  throw(bad_value) // set_value()
  : parameter(value_), orig_value(value_), lower(lower_),
    upper(upper_)
{
  // calling set_value() will check to make sure lower_ <= value_ <= upper_
  set_value(value_);
}

// ctor: name and optional value and bounds
// ctor: defaults value_ = 0.0e0, lower_ = -DBL_MAX,
// ctor:          upper_ = DBL_MAX
opt_parameter::opt_parameter(const CH_STD::string& name_, double value_,
			     double lower_, double upper_)
  throw(bad_value) // set_value()
  : parameter(name_, value_), orig_value(value_), lower(lower_),
    upper(upper_)
{
  // calling set_value() will check to make sure _lower <= value_ <= _upper
  set_value(value_);
}

// dtor: do nothing
opt_parameter::~opt_parameter()
{}

// opt_parameter public methods
// return original value of the parameter
double
opt_parameter::get_original_value() const
{
  return orig_value;
}

// return the LOWER bound on the parameter
double
opt_parameter::get_lower_bound() const
{
  return lower;
}

// return the UPPER bound on the parameter
double
opt_parameter::get_upper_bound() const
{
  return upper;
}

// change the value of the parameter, return old value
double
opt_parameter::set_value(double value_)
  throw(bad_value) // this
{
  // make sure value is within the bounds
  if (value_ < get_lower_bound())
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":opt_parameter::set_value(): " + stringify() + "'s "
		      "value (" + t_string(value_) + ") is less than "
		      "lower bound");
    }
  else if (value_ > get_upper_bound())
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":opt_parameter::set_value(): " + stringify() + "'s "
		      "value (" + t_string(value_) + ") is greater than "
		      "upper bound");
    }
  return parameter::set_value(value_);
}

// change the value, setting to the bound if the desired value is outside
// the bound, return value used
double
opt_parameter::set_value_bounds(double value_)
{
  // check if value_ is outside of bounds
  if (value_ < lower)
    {
      parameter::set_value(lower);
    }
  else if (value_ > upper)
    {
      parameter::set_value(upper);
    }
  else // value_ is within bounds
    {
      parameter::set_value(value_);
    }
  return get_value(); // return the value used
}

// log_parameter methods
// ctor: (default) create name for parameter
// ctor: defaults value_ = 1.0e0, lower_ = 0.0e0, upper_ = DBL_MAX
log_parameter::log_parameter(double value_, double lower_,
			     double upper_)
  throw(bad_value) // set_value()
  : opt_parameter(value_, lower_, upper_), sign(1), log_value(0.0e0)
{
  // set the value to log(value) and make sure it is not zero
  set_value(value_);
}

// ctor: with name given
// ctor: defaults value_ = 1.0e0, lower_ = 0.0e0, upper_ = DBL_MAX
log_parameter::log_parameter(const CH_STD::string& name_, double value_,
			     double lower_, double upper_)
  throw(bad_value) // set_value()
  : opt_parameter(name_, value_, lower_, upper_), sign(1), log_value(0.0e0)
{
  // set the value to log(value_) and make sure it is not zero
  set_value(value_);
}

// dtor: do nothing
log_parameter::~log_parameter()
{}

// log_parameter private methods
// safely return log(_value) and set sign to sign of _value
CH_STD::pair<int,double>
log_parameter::take_log(double _value)
  throw(bad_value) // this
{
  if (_value > 0.0e0)
    {
      return CH_STD::make_pair(1, log(_value));
    }
  else if (_value < 0.0e0)
    {
      return CH_STD::make_pair(-1, log(-_value));
    }
  else // cannot take log of zero
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":log_parameter::take_log(): log value cannot be zero");
    }
}

// log_parameter public methods
// return value of the parameter (must exponentiate)
double
log_parameter::get_value() const
{
  return opt_parameter::get_value();
}

// return the integer sign and log value of the parameter
CH_STD::pair<int,double>
log_parameter::get_log_value() const
{
  return CH_STD::make_pair(sign, log_value);
}

// change the value, return old value (keep track of log's and exp's)
double
log_parameter::set_value(double value_)
  throw(bad_value) // take_log(), opt_parameter::set_value()
{
  // take the log and save the result
  CH_STD::pair<int,double> log_pair(take_log(value_));
  // set the actual value, make sure value in bounds
  double old(opt_parameter::set_value(value_));
  sign = log_pair.first;	// set the value of the sign
  log_value = log_pair.second;	// set log_value to log(|value|)
  // return old value
  return old;
}

// change the value, setting to the bound if the desired value is outside
// the bound, return value used
double
log_parameter::set_value_bounds(double value_)
  throw(bad_value) // take_log()
{
  // set value and save the value used
  double new_value(opt_parameter::set_value_bounds(value_));
  // take the log and save pair for assignment
  CH_STD::pair<int,double> log_pair(take_log(new_value));
  // update instance variables
  sign = log_pair.first;
  log_value = log_pair.second;
  return new_value; // return actual new value
}

// change the value using the log value, returning the old log value
// defaults sign_ = 1
CH_STD::pair<int,double>
log_parameter::set_log_value(double log_value_, int sign_)
  throw(bad_value) // opt_parameter::set_value()
{
  // set actual value and make sure it is in the bounds
  opt_parameter::set_value(sign_ * exp(log_value_));
  // set sign and log_value and return the old values
  CH_STD::pair<int,double> old(CH_STD::make_pair(sign, log_value));
  sign = sign_;
  log_value = log_value_;
  return old;
}

// change value using the given log value, if value is out of bounds, set to
// the bound, return the new log value
// defaults sign_ = 1
CH_STD::pair<int,double>
log_parameter::set_log_value_bounds(double log_value_, int sign_)
{
  // set the value safely, retain the new value
  double new_value(opt_parameter::set_value_bounds(sign_ * exp(log_value_)));
  CH_STD::pair<int,double> new_log(take_log(new_value));
  sign = new_log.first;
  log_value = new_log.second;
  return new_log;
}

// par_expression methods
// ctor: do nothing
par_expression::par_expression()
{}

// dtor: do nothing
par_expression::~par_expression()
{}

// par_single methods
// ctor: user-supplied parameter
par_single::par_single(parameter* par_)
  : par(par_)
{}

// dtor: do nothing
par_single::~par_single()
{}

// public methods
// return value of par
double
par_single::get_value() const
{
  return par->get_value();
}

// return a string representation of the parameter expression
CH_STD::string
par_single::stringify() const
{
  return par->stringify();
}

// par_minus methods
// ctor: user-supplied parameter
par_minus::par_minus(par_expression* positive_)
  : positive(positive_)
{}

// dtor: delete positive
par_minus::~par_minus()
{
  delete positive;
  positive = 0;
}

// public methods
// return negated value of positive
double
par_minus::get_value() const
{
  return - positive->get_value();
}

// return a string representation of the parameter expression
CH_STD::string
par_minus::stringify() const
{
  return "-" + positive->stringify();
}

// par_sum methods
// ctor: user-supplied expressions
par_sum::par_sum(par_expression* left_, par_expression* right_)
  : left(left_), right(right_)
{}

// dtor: delete left and right
par_sum::~par_sum()
{
  delete left;
  left = 0;
  delete right;
  right = 0;
}

// public methods
// return sum of the values of the two expressions
double
par_sum::get_value() const
{
  return left->get_value() + right->get_value();
}

// return a string representation of the parameter expression
CH_STD::string
par_sum::stringify() const
{
  return "(" + left->stringify() + " + " + right->stringify() + ")";
}

// par_difference methods
// ctor: user-supplied expressions
par_difference::par_difference(par_expression* left_, par_expression* right_)
  : left(left_), right(right_)
{}

// dtor: delete left and right
par_difference::~par_difference()
{
  delete left;
  left = 0;
  delete right;
  right = 0;
}

// public methods
// return difference of the values of the two expressions
double
par_difference::get_value() const
{
  return left->get_value() - right->get_value();
}

// return a string representation of the parameter expression
CH_STD::string
par_difference::stringify() const
{
  return "(" + left->stringify() + " - " + right->stringify() + ")";
}

// par_product methods
// ctor: user-supplied expressions
par_product::par_product(par_expression* left_, par_expression* right_)
  : left(left_), right(right_)
{}

// dtor: delete left and right
par_product::~par_product()
{
  delete left;
  left = 0;
  delete right;
  right = 0;
}

// public methods
// return product of the values of the two expressions
double
par_product::get_value() const
{
  return left->get_value() * right->get_value();
}

// return a string representation of the parameter expression
CH_STD::string
par_product::stringify() const
{
  return left->stringify() + " * " + right->stringify();
}

// par_ratio methods
// ctor: user-supplied expressions
par_ratio::par_ratio(par_expression* numerator_, par_expression* denominator_)
  : numerator(numerator_), denominator(denominator_)
{}

// dtor: delete numerator and denominator
par_ratio::~par_ratio()
{
  delete numerator;
  numerator = 0;
  delete denominator;
  denominator = 0;
}

// public methods
// return ratio of the values of the two expressions
double
par_ratio::get_value() const
  throw(bad_value) // this
{
  // see if denominator is zero
  double r(denominator->get_value());
  if (r == 0.0e0)
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":par_ratio::get_value(): expression denominator (" +
		      stringify() + ") is zero (0.0e0)");
    }
  return numerator->get_value() / r;
}

// return a string representation of the parameter expression
CH_STD::string
par_ratio::stringify() const
{
  return numerator->stringify() + " / " + denominator->stringify();
}

// par_pow methods
// ctor: user-supplied base and exponent
par_pow::par_pow(par_expression* base_, par_expression* exponent_)
  : base(base_), exponent(exponent_)
{}

// dtor: do nothing
par_pow::~par_pow()
{
  delete base;
  base = 0;
  delete exponent;
  exponent = 0;
}

// return (base)^(exponent), check to make sure values are ok
double
par_pow::get_value() const
  throw(bad_value) // this
{
  double b(base->get_value());
  double e(exponent->get_value());
  // check to make sure we are not dividing by zero
  if (CH_STD::fabs(b) < DBL_EPSILON && e <= 0.0e0)
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":par_pow::get_value(): in expression (" + stringify() +
		      ") you are trying to raise zero to a negative power("
		      + t_string(e) + ")");
    }
  // check if raising a negative number to a non-integral power
  double i(0.0e0);
  if (b < 0.0e0 && CH_STD::fabs(CH_STD::modf(e, &i)) > DBL_EPSILON)
    {
      // throw exception
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":par_pow::get_value(): in expression (" + stringify() +
		      ") you are trying to raise a negative number (" +
		      t_string(b) + ") to a non-integral(" + t_string(e) +
		      "power");
    }
  return CH_STD::pow(b, e);
};

// return a string representation of the parameter expression
CH_STD::string
par_pow::stringify() const
{
  return "(" + base->stringify() + ")^(" + exponent->stringify() + ")";
}

CH_END_NAMESPACE

/* $Id: parameter.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
