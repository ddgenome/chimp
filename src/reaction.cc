// Functions for the manipulation of chemical reactions.
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

#include "reaction.h"
#include <cfloat>
#include <cmath>
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initialize static instance variables
quantity::type model_reaction::amount_type(quantity::Econcentration);

// stoiciometric coefficient class methods
// ctor: (default) set coefficient to given value
// ctor: default coefficient_ = 0.0e0
stoichiometric::stoichiometric(double coefficient_)
  : coefficient(coefficient_), power(coefficient_), power_set(false)
{}

// ctor: copy
stoichiometric::stoichiometric(const stoichiometric& original)
  : coefficient(original.coefficient), power(original.power),
    power_set(original.power_set)
{}

// dtor: do nothing
stoichiometric::~stoichiometric()
{}

// stoichiometric public methods
// return current value of stoichiometric coefficient
double
stoichiometric::get_coefficient() const
{
  return coefficient;
}

// return current value of exponent in rate expression
double
stoichiometric::get_power() const
{
  return power;
}

// return whether power was explicitly set
bool
stoichiometric::is_power_set() const
{
  return power_set;
}

// set the coefficient value, return old value
double
stoichiometric::set_coefficient(double coefficient_)
{
  double old(coefficient_);
  coefficient = coefficient_;
  return old;
}

// additive assignment operator
double
stoichiometric::increment(const stoichiometric& delta)
  throw (bad_input)
{
  // increment the coefficient
  coefficient += delta.coefficient;
  // deal with power situation
  if (power_set && delta.power_set)
    {
      // can't set power twice for same species
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":stoichiometric::increment(): trying to increment one "
		      "stoichiometric object with another when both have the "
		      "power explicitly set");
    }
  else if (delta.power_set)
    {
      power = delta.power;
    }
  else
    {
      power = coefficient;
    }
  return coefficient;
}

// additive assignment operator
double
stoichiometric::operator+=(double increment)
{
  coefficient += increment;
  if (!power_set)
    {
      power = coefficient;
    }
  return coefficient;
}

// set exponent in rate expression explicitly, return POWER
double
stoichiometric::set_power(double power_)
{
  power_set = true;
  return power = power_;
}

// reaction class methods
// ctor: optionally reversible reaction
// ctor: defaults k_reverse_ = 0, own_k_ = true
reaction::reaction(k* k_forward_, k* k_reverse_, bool own_k_)
  : reactants(), products(), net(), k_forward(k_forward_),
    k_reverse(k_reverse_), own_k(own_k_)
{}

// dtor: delete rate constants
reaction::~reaction()
{
  if (own_k)
    {
      // delete checks for null pointer
      delete k_forward;
      delete k_reverse;
    }
}

// protected methods
// ctor: make a new (model compatible) reaction from and old one and a map
reaction::reaction(const reaction& original, const species2model& s2m)
  throw (bad_pointer)
  : k_forward(original.k_forward), k_reverse(original.k_reverse), own_k(false)
{
  for (stoich_map_citer it = original.reactants.begin();
       it != original.reactants.end(); it++)
    {
      species2model_citer s2m_pair(s2m.find(it->first));
      // make sure species pair exists and model_species is not NULL
      if (s2m_pair == s2m.end() || s2m_pair->second == 0)
	{
	  throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":reaction::reaction(): invalid species* to "
			    "model_species* mapping for species " +
			    it->first->stringify());
	}
      // add model_species with same stoichiometric coeff
      add_reactant(s2m_pair->second, it->second);
    }
  for (stoich_map_citer it = original.products.begin();
       it != original.products.end(); it++)
    {
      species2model_citer s2m_pair(s2m.find(it->first));
      // make sure species pair exists and model_species is not NULL
      if (s2m_pair == s2m.end() || s2m_pair->second == 0)
	{
	  throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":reaction::reaction(): invalid species* to "
			    "model_species* mapping for species " +
			    it->first->stringify());
	}
      // add model_species with same stoichiometric coeff
      add_product(s2m_pair->second, it->second);
    }
}

// return value of forward rate constant at temperature T
double
reaction::get_forward_k(double T, double R) const
  throw (bad_pointer)
{
  // make sure forward rate constant was set
  if (k_forward == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":reaction::get_forward_k(): forward rate constant "
			"pointer points to null");
    }
  return k_forward->get_k(T, R);
}

// return value of reverse rate constant at temperature T
double
reaction::get_reverse_k(double T, double R) const
{
  if (k_reverse == 0)
    {
      return 0.0e0;
    }
  // else
  return k_reverse->get_k(T, R);
}

// public methods
// return rate constant pointers, release ownership
CH_STD::pair<k*,k*>
reaction::get_rate_constants()
{
  own_k = false;
  return CH_STD::make_pair(k_forward, k_reverse);
}

// add species to reactant list, return COEFFICIENT after incrementing by COEFF
// defaults coeff = 1.0e0
double
reaction::add_reactant(species* reactant, double coeff)
{
  // if map[foo] does not exist, it is created with value of zero
  // reactants have negative term in balance equation
  net[reactant] += -coeff;
  // use positive coefficients in reactants stoich_map
  return reactants[reactant] += coeff;
}

// add species to product list, return COEFFICIENT after incrementing by COEFF
// defaults coeff = 1.0e0
double
reaction::add_product(species* product, double coeff)
{
  // if map[foo] does not exist, it is created
  // increment stoichiometric coefficient in the net and products maps
  net[product] += coeff;
  return products[product] += coeff;
}

// add species to reactants, return COEFFICIENT after incrementing with COEFF
double
reaction::add_reactant(species* reactant, const stoichiometric& coeff)
  throw (bad_input)
{
  // don't worry about net's power
  net[reactant] += -coeff.get_coefficient();
  return reactants[reactant].increment(coeff);
}

// add species to products, return COEFFICIENT after incrementing with COEFF
double
reaction::add_product(species* product, const stoichiometric& coeff)
  throw (bad_input)
{
  // don't worry about net's power
  net[product] += coeff.get_coefficient();
  return products[product].increment(coeff);
}

// return constant reference to the reactant stoich_map
const stoich_map&
reaction::get_reactants() const
{
  return reactants;
}

// return constant reference to the product stoich_map
const stoich_map&
reaction::get_products() const
{
  return products;
}

// return constant reference to the net coefficient stoich_map
const stoich_map&
reaction::get_net_coefficients()
{
  return net;
}

// return whether reaction is reversible (k_reverse != 0)
bool
reaction::is_reversible() const
{
  if (k_reverse == 0)
    {
      return false;
    }
  // else
  return true;
}

// return a string description of the reaction (no newline)
CH_STD::string
reaction::stringify() const
{
  CH_STD::string rxn_string;
  bool strung_one = false;
  for (stoich_map_citer it = reactants.begin(); it != reactants.end();
       it++)
    {
      if (strung_one)
	{
	  rxn_string += " + ";
	}
      if (it->second.get_coefficient() == 1.0e0)
	{
	  rxn_string += it->first->get_name();
	  strung_one = true;
	}
      else if (it->second.get_coefficient() != 0.0e0)
	{
	  rxn_string += concatenate(it->second.get_coefficient(),
				    " " + it->first->get_name());
	  strung_one = true;
	}
      if (it->second.get_power() != it->second.get_coefficient())
	{
	  rxn_string += concatenate("^", it->second.get_power());
	}
    }
  if (k_reverse == 0) // no reverse reaction
    {
      rxn_string += " -> " + k_forward->stringify() + " ";
    }
  else
    {
      rxn_string += " <- " + k_reverse->stringify() + " -> " +
	k_forward->stringify() + " ";
    }
  strung_one = false;
  for (stoich_map_citer it = products.begin(); it != products.end();
       it++)
    {
      if (strung_one)
	{
	  rxn_string += " + ";
	}
      if (it->second.get_coefficient() == 1.0e0)
	{
	  rxn_string += it->first->get_name();
	  strung_one = true;
	}
      else if (it->second.get_coefficient() != 0.0e0)
	{
	  rxn_string += concatenate(it->second.get_coefficient(),
				    " " + it->first->get_name());
	  strung_one = true;
	}
      if (it->second.get_power() != it->second.get_coefficient())
	{
	  rxn_string += concatenate("^", it->second.get_power());
	}
    }
  rxn_string += ";";
  return rxn_string;
}

// model_reaction methods
// ctor: create with given rate constants
// ctor: defaults k_reverse_ = 0
model_reaction::model_reaction(k* k_forward_, k* k_reverse_)
  : reaction(k_forward_, k_reverse_), reactant_seq(), product_seq(),
    seq_available(false)
{}

// ctor: create from a reaction
model_reaction::model_reaction(const reaction& original,
			       const species2model& s2m)
  throw (bad_pointer)
  : reaction(original, s2m), reactant_seq(), product_seq(),
    seq_available(false)
{
  // try to make the sequences
  create_species_seq();
}

// dtor: do nothing
model_reaction::~model_reaction()
{}

// model_reactions private methods
// create the product and reactant species sequences
void
model_reaction::create_species_seq()
{
  // where to store return value of create_species_seq()
  CH_STD::pair<bool,model_species::seq> bool_seq;
  // create reactant sequence
  bool_seq = create_species_seq(get_reactants());
  // see if it did it
  if (bool_seq.first)
    {
      // assign the reactant sequence
      reactant_seq = bool_seq.second;
      // get the product one
      bool_seq = create_species_seq(get_products());
      if (bool_seq.first)
	{
	  // success
	  product_seq = bool_seq.second;
	  // both successfule, so set boolean to true
	  seq_available = true;
	}
      else			// failure
	{
	  // clear everything out
	  reactant_seq.clear();
	  product_seq.clear();
	  seq_available = false;
	}
    }
  else				// failure
    {
      // clear everything out
      reactant_seq.clear();
      product_seq.clear();
      seq_available = false;
    }
  return;
}

// return a list of model species from the given stoich_map
CH_STD::pair<bool,model_species::seq>
model_reaction::create_species_seq(const stoich_map& coeff_species)
{
  // create sequence
  model_species::seq species_seq;
  // loop through the reactants
  for (stoich_map_citer it(coeff_species.begin()); it != coeff_species.end();
       ++it)
    {
      // make sure stoichiometric coefficient is close to an integer
      double dcoeff;
      if (CH_STD::fabs(CH_STD::modf(it->second.get_coefficient(), &dcoeff))
	  > 2.0e0 * DBL_EPSILON)
	{
	  // can't put a species in a fractional number of times
	  return CH_STD::make_pair(false, model_species::seq());
	}
      // cast the coeff to an int (make sure it is correct)
      int coeff = static_cast<int>(dcoeff + 1.0e-1);
      // insert the species the number of times it appears
      for (int count(0); count < coeff; ++count)
	{
	  // insert the species
	  species_seq.push_back(static_cast<model_species*>(it->first));
	}
    }
  return CH_STD::make_pair(true, species_seq);
}

// model_reaction public methods
// return current AMOUNT_TYPE
quantity::type
model_reaction::get_amount_type()
{
  return amount_type;
}

// set the amount_type, return old value
// defaults type_ = quantity::Econcentration
quantity::type
model_reaction::set_amount_type(quantity::type type_)
  throw (bad_type)
{
  // make sure type is valid
  if (!quantity::is_type(type_))
    {
      // throw exception
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":model_reaction::set_amount_type(): invalid "
		     "quantity::type");
    }
  quantity::type old(amount_type);
  amount_type = type_;
  return old;
}

// add species to reactants, return COEFFICIENT after += -COEFF
double
model_reaction::add_reactant(model_species* reactant, double coeff)
{
  return reaction::add_reactant(reactant, coeff);
}

// add species to products, return COEFFICIENT after += COEFF
double
model_reaction::add_product(model_species* product, double coeff)
{
  return reaction::add_product(product, coeff);
}

// return forward rate at temperature T
// defaults R = 8.314e-3 kJ/molK
double
model_reaction::get_forward_rate(double T, double R) const
  throw (bad_pointer, bad_type)
{
  double rate(get_forward_k(T, R));
  const stoich_map r(get_reactants());
  for (stoich_map_citer it = r.begin(); it != r.end(); it++)
    {
      rate *= pow(static_cast<model_species*>(it->first)->get_quantity(get_amount_type()),
		  it->second.get_power());
    }
  return rate;
}

// returns reverse rate at temperature T
// defaults R = 8.314e-3 kJ/molK
double
model_reaction::get_reverse_rate(double T, double R) const
  throw (bad_type)
{
  if (!is_reversible())
    {
      return 0.0e0;
    }
  // else
  double rate(get_reverse_k(T, R));
  const stoich_map p(get_products());
  for (stoich_map_citer it = p.begin(); it != p.end(); it++)
    {
      rate *= pow(static_cast<model_species*>(it->first)->get_quantity(get_amount_type()),
		  it->second.get_power());
    }
  return rate;
}

// return net rate (forward - reverse) at temperature T
// defaults R = 8.314e-3 kJ/molK
double
model_reaction::get_net_rate(double T, double R) const
  throw (bad_pointer, bad_type)
{
  return get_forward_rate(T, R) - get_reverse_rate(T, R);
}

// update derivatives of reactants and products using given rate
void
model_reaction::derivatives(double rate)
{
  const stoich_map n(get_net_coefficients());
  for (stoich_map_citer it = n.begin(); it != n.end(); it++)
    {
      static_cast<model_species*>(it->first)->add_to_derivative(it->second.get_coefficient() * rate);
    }
}

// calculate rate and update derivatives of reactants and products, return rate
// defaults R = 8.314e-3 kJ/molK
double
model_reaction::rate_derivatives(double T, double R)
  throw (bad_pointer, bad_type)
{
  double rate(get_net_rate(T, R));
  derivatives(rate);
  return rate;
}

// return a sequence containing the reactants
const model_species::seq&
model_reaction::get_reactant_seq()
  throw (bad_input)
{
  if (seq_available)
    {
      return reactant_seq;
    }
  // else try to make them
  create_species_seq();
  if (seq_available)
    {
      return reactant_seq;
    }
  // else
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  ":model_reaction::get_reactant_seq(): invalid (non-integer) "
		  "stoichiometric coefficient in reaction: " + stringify());
}

// return a pointer to a sequence containing the reactants, or zero if not
// available
const model_species::seq*
model_reaction::get_reactant_seq_ptr()
{
  if (seq_available)
    {
      return &reactant_seq;
    }
  // else try to make them
  create_species_seq();
  if (seq_available)
    {
      return &reactant_seq;
    }
  // else
  return 0;
}

// return a sequence containing the products
const model_species::seq&
model_reaction::get_product_seq()
  throw (bad_input)
{
  if (seq_available)
    {
      return product_seq;
    }
  // else try to make them
  create_species_seq();
  if (seq_available)
    {
      return product_seq;
    }
  // else
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  ":model_reaction::get_product_seq(): invalid (non-integer) "
		  "stoichiometric coefficient in reaction: " + stringify());
}

// return a pointer to sequence containing the products, or zero if not
// available
const model_species::seq*
model_reaction::get_product_seq_ptr()
{
  if (seq_available)
    {
      return &product_seq;
    }
  // else try to make them
  create_species_seq();
  if (seq_available)
    {
      return &product_seq;
    }
  // else
  return 0;
}

// for kmc: return the forward rate, only multiplying the rate constant
// and gas phase species
double
model_reaction::get_fluid_forward_rate(double T, double R) const
  throw (bad_pointer, bad_type)
{
  double rate(get_forward_k(T, R));
  const stoich_map r(get_reactants());
  for (stoich_map_citer it = r.begin(); it != r.end(); it++)
    {
      if (it->first->get_surface_coordination() < 1)
	{
	  rate *= pow(static_cast<model_species*>(it->first)->get_quantity(get_amount_type()), it->second.get_power());
	}
    }
  return rate;
}

// for kmc: return the reverse rate, only multiplying the rate constant
// and gas phase species
double
model_reaction::get_fluid_reverse_rate(double T, double R) const
  throw (bad_type)
{
  if (!is_reversible())
    {
      return 0.0e0;
    }
  // else
  double rate(get_reverse_k(T, R));
  const stoich_map p(get_products());
  for (stoich_map_citer it = p.begin(); it != p.end(); it++)
    {
      if (it->first->get_surface_coordination() < 1)
	{
	  rate *= pow(static_cast<model_species*>(it->first)->get_quantity(get_amount_type()), it->second.get_power());
	}
    }
  return rate;
}

CH_END_NAMESPACE

/* $Id: reaction.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
