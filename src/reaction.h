// -*- C++ -*-
// Class declarations for chemical reactions.
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
#ifndef CH_REACTION_H
#define CH_REACTION_H 1

#include <map>
#include <utility>
#include <vector>
#include "constant.h"
#include "except.h"
#include "k.h"
#include "quantity.h"
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class for stoichiometric coefficients
// use double for fractional coefficients
class stoichiometric
{
  double coefficient;		// value of coefficient
  double power;			// exponent in rate equation
  bool power_set;		// value of power set explicitly

private:
  // prevent assignment
  stoichiometric& operator=(const stoichiometric&);
public:
  // ctor: (default) initialize to given coefficient
  explicit stoichiometric(double coefficient_ = 0.0e0);
  // ctor: copy
  stoichiometric(const stoichiometric& original);
  // dtor: do nothing
  ~stoichiometric();

  // return current value of coefficient
  double get_coefficient() const;
  // return current value of exponent
  double get_power() const;
  // return whether power was explicitly set
  bool is_power_set() const;
  // set the coefficient, return old value
  double set_coefficient(double coefficient_);
  // increment assignment operator for coefficient
  double increment(const stoichiometric& delta)
    throw (bad_input); // this
  // increment assignment operator for coefficient
  double operator+=(double increment);
  // set exponent in rate expression, return POWER
  double set_power(double power_);
}; // end class stoichiometric

// set up species-stiochiometric coefficient map
typedef CH_STD::map<species*,stoichiometric> stoich_map;
typedef stoich_map::iterator stoich_map_iter;
typedef stoich_map::const_iterator stoich_map_citer;

// chemical reaction class
class reaction
{
public:
  // set up containers (sequence) and iterators for reaction
  typedef CH_STD::vector<reaction*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;

private:
  stoich_map reactants;		// map of reactants and stoichiometric coeffs
  stoich_map products;		// map of products and stoichiometric coeffs
  stoich_map net;		// map for net stoichiometric coeffs
  k* k_forward;			// rate constant for forward reaction
  k* k_reverse;			// rate constant for reverse reaction
  bool own_k;			// whether this reaction owns the k's

private:
  // prevent copy construction and assignment
  reaction(const reaction&);
  reaction& operator=(const reaction&);
protected:
  // ctor: make a new (model compatible) reaction from and old one and a map
  reaction(const reaction& reaction_, const species2model& s2m)
    throw (bad_pointer); // this
  // return forward rate constant
  double get_forward_k(double T, double R) const
    throw (bad_pointer); // this
  // return reverse rate constant
  double get_reverse_k(double T, double R) const;
public:
  // ctor: (default) create an empty reaction with the given rates
  reaction(k* k_forward_, k* k_reverse_ = 0, bool own_k_ = true);
  // dtor: delete rate constants
  ~reaction();

  // return rate constant pointers, release ownership
  CH_STD::pair<k*,k*> get_rate_constants();
  // add species to reactants, return COEFFICIENT after += -COEFF
  double add_reactant(species* reactant, double coeff = 1.0e0);
  // add species to products, return COEFFICIENT after += COEFF
  double add_product(species* product, double coeff = 1.0e0);
  // add species to reactants, return COEFFICIENT after incrementing with COEFF
  double add_reactant(species* reactant, const stoichiometric& coeff)
    throw (bad_input); // stoichiometric::increment()
  // add species to products, return COEFFICIENT after incrementing with COEFF
  double add_product(species* product, const stoichiometric& coeff)
    throw (bad_input); // stoichiometric::increment()
  // return const references to the stoich_map's
  const stoich_map& get_reactants() const;
  const stoich_map& get_products() const;
  const stoich_map& get_net_coefficients();
  // return whether reaction is reversible (k_reverse != 0)
  bool is_reversible() const;
  // return string representation of reaction
  CH_STD::string stringify() const;
}; // end class reaction

// class for reactions with model_species for model solution
class model_reaction : public reaction
{
public:
  // set up containers (sequence) and iterators for reaction
  typedef CH_STD::vector<model_reaction*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;

private:
  model_species::seq reactant_seq; // each reactant and product, listed
  model_species::seq product_seq;  // its stoich number of times (if possible)
  static quantity::type amount_type; // type used in rate calculation
  bool seq_available;		// whether the above seqs were filled ok

private:
  // prevent copy construction and assignment
  model_reaction(const model_reaction&);
  model_reaction& operator=(const model_reaction&);
  // create the product and reactant species sequences
  void create_species_seq();
  // return a list of model species from the given stoich_map having each
  // species appear in the list the number of times given by its
  // stoichiometric coefficient
  static CH_STD::pair<bool,model_species::seq>
    create_species_seq(const stoich_map& coeff_species);
public:
  // ctor: create with given rate constants
  explicit model_reaction(k* k_forward_, k* k_reverse_ = 0);
  // ctor: create from a reaction
  model_reaction(const reaction& reaction_, const species2model& s2m)
    throw (bad_pointer); // reaction()
  // dtor: do nothing
  ~model_reaction();

  // return current AMOUNT_TYPE
  static quantity::type get_amount_type();
  // set the amount_type, return old value
  static quantity::type set_amount_type(quantity::type type_ =
					quantity::Econcentration)
    throw (bad_type); // this
  // add species to reactants, return COEFFICIENT after += -COEFF
  double add_reactant(model_species* reactant, double coeff = 1.0e0);
  // add species to products, return COEFFICIENT after += COEFF
  double add_product(model_species* product, double coeff = 1.0e0);
  // calculates forward rate at the given temperature T
  double get_forward_rate(double T, double R = constant::r) const
    throw (bad_pointer, bad_type); // get_forward_k(),
				// model_species::get_quantity()
  // calculates forward rate at the given temperature T
  double get_reverse_rate(double T, double R = constant::r) const
    throw (bad_type); // model_species::get_quantity()
  // calculates net rate (forward - reverse) at the given temperature T
  double get_net_rate(double T, double R = constant::r) const
    throw (bad_pointer, bad_type); // get_forward_rate(), get_reverse_rate()
  // update derivatives of reactants and products using given rate
  void derivatives(double rate);
  // calculate rate and update derivatives of reactants and products, return rate
  double rate_derivatives(double T, double R = constant::r)
    throw (bad_pointer, bad_type); // get_net_rate()
  // return a sequence containing the reactants having each
  // species appear in the list the number of times given by its
  // stoichiometric coefficient
  const model_species::seq& get_reactant_seq()
    throw (bad_input); // this
  // return pointer to sequence of reactants or zero if not available
  const model_species::seq* get_reactant_seq_ptr();
  // return a sequence containing the products having each
  // species appear in the list the number of times given by its
  // stoichiometric coefficient
  const model_species::seq& get_product_seq()
    throw (bad_input); // this
  // return pointer to sequence of products or zero if not available
  const model_species::seq* get_product_seq_ptr();
  // for kmc: return the rate expression, only multiplying the rate constant
  // and gas phase species
  double get_fluid_forward_rate(double T, double R = constant::r) const
    throw (bad_pointer, bad_type); // get_forward_k()
				// model_species::get_quantity()
  double get_fluid_reverse_rate(double T, double R = constant::r) const
    throw (bad_type); // model_species::get_quantity()
}; // end class model_reaction

CH_END_NAMESPACE

#endif // not CH_REACTION_H

/* $Id: reaction.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
