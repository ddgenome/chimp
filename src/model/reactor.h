// -*- C++ -*-
// Reactor configuration and solution information.
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
#ifndef CH_MODEL_REACTOR_H
#define CH_MODEL_REACTOR_H 1

#include <map>
#include <string>
#include <vector>
#include "except.h"
#include "quantity.h"
#include "reaction.h"
#include "species.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// abstract class to derive reactor types from
class reactor
{
  double temperature;		// reactor temperature
  double heating_rate;		// heating rate (for TPD, etc.)
  double pressure;		// reactor pressure
  double volume;		// reactor volume
  double weight;		// catalyst weight
  double sites;			// catalytic sites
protected:
  // rate information
  enum rate_amount { Emoles, Emolecules }; // numerator of rate expression
  enum rate_size { Evolume, Eweight, Esites }; // denominator of rate exp*time
  rate_amount amount_type;	// rate expressed in moles or molecules
  rate_size size_type;		// rate expressed per V, W, or Ns
  quantity::type fluid_type;	// amount fluid derivatives are expressed in

private:
  // prevent assignment
  reactor& operator=(const reactor&);
  // set how the amount is expressed in the rate expr, return old value
  CH_STD::string set_rate_amount_type(const CH_STD::string& amount_type_)
    throw (bad_type); // this
  // set how the reactor size is expressed in the rate expr, return old value
  CH_STD::string set_rate_size_type(const CH_STD::string& size_type_)
    throw(bad_type); // this
  // set the quantity type for fluids, return old value
  quantity::type set_fluid_type(const CH_STD::string& fluid_type_)
    throw (bad_type); // this, quantity::get_type()
protected:
  // ctor: copy
  explicit reactor(const reactor& original);
  // set the temperature, return old
  double set_temperature(double temperature_)
    throw (bad_value); // this
  // set the heating rate (temperature/time), return old
  double set_heating_rate(double heating_rate_);
  // set the pressure, return old
  double set_pressure(double pressure_)
    throw (bad_value); // this
  // add INCREMENT to total PRESSURE, return new total
  double add_to_pressure(double increment)
    throw (bad_value); // set_pressure()
  // set the volume of the reactor, return old
  double set_volume(double volume_)
    throw (bad_value); // this
  // set the catalyst weight, return old value
  double set_weight(double weight_)
    throw (bad_value); // this
  // set the number of catalytic sites, return old value
  double set_sites(double sites_)
    throw (bad_value); // this
  // individual species kmc equation, return relative change in quantity
  // molecules is how many molecules are reacting
  double kmc_reaction(model_species* msp) const;
public:
  // ctor: (default) set variables to ``typical'' values
  reactor();
  // dtor: do nothing
  virtual ~reactor();

  // parse reactor input
  virtual void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_value, bad_type); // this, set_temperature(),
				// set_pressure(), add_to_pressure(),
				// set_volume(), set_weight(), set_sites(),
				// set_fluid_type(), set_rate_amount_type(),
				// set_rate_size_type()
  // create a copy of the appropriate derived type, return pointer or zero
  virtual reactor* copy() = 0;
  // initialize the reactor and its solution variables
  virtual void initialize(model_species::seq_citer species_begin,
			  model_species::seq_citer species_end)
    throw (bad_type, bad_value); // model_reaction::set_amount_type(),
		      // species::get_quantity(), set_pressure()
  // set stuff up for a kmc run, return scale
  double kmc_initialize(unsigned int kmc_sites, double scale)
    throw (bad_value); // set_sites()
  // create a new reactor of appropriate type, return pointer or zero
  static reactor* new_reactor(const CH_STD::string& type = "batch");
  // return the TEMPERATURE
  double get_temperature() const;
  // return the heating rate (temperature/time), return old
  double get_heating_rate() const;
  // return the pressure
  double get_pressure() const;
  // return the volume of the reactor
  double get_volume() const;
  // return the catalyst weight
  double get_weight() const;
  // return the number of catalytic sites
  double get_sites() const;
  // return the quantity type used for fluids
  quantity::type get_fluid_type() const;
  // loop through species and modify derivatives using reactor design eqns
  void reactor_eqn(const model_species::seq& species_list)
    throw (bad_type); // batch_reactor::reactor_eqn(), cstr::reactor_eqn()
  // modify species derivatives using reactor design equations
  virtual double reactor_eqn(model_species* msp) = 0;
  // adjust rate according to units used in reactor
  double kmc_rate(double original_rate = 1.0e0)
    throw (bad_type); // this
  // make sure reaction is possible, given current species amounts
  bool kmc_quantities(const stoich_map& net, double molecules) const
    throw (bad_type); // model_species::get_quantity()
  // reactor equation for kinetic Monte Carlo
  // molecules is the number of gas phase molecules to change per
  // reactant and product
  void kmc_reaction(const model_species::seq& reactants,
		    const model_species::seq& products, double molecules)
    throw (bad_type, bad_value); // surface_quantity::add_to_quantity(),
				// kmc_reaction()
  // update everything given the time increment
  virtual void kmc_step(const model_species::seq_citer species_begin,
			const model_species::seq_citer species_end, double dx)
    throw (bad_type, bad_value); // set_temperature(),
		      // batch_reactor::kmc_step(),
		      // cstr::kmc_step(), set_temperature()
  // update an individual gas-phase species
  virtual void kmc_step(model_species* msp, double dx, double T0,
			double T1) = 0;
}; // end class reactor

// abstract class for flow reactors
class flow_reactor : public reactor
{
  double flow;			// total molar flow

private:
  // prevent assignment
  flow_reactor& operator=(const flow_reactor&);
protected:
  // ctor: copy
  explicit flow_reactor(const flow_reactor& original);
public:
  // ctor: (default) call reactor ctor, set flow to 0
  flow_reactor();
  // dtor: do nothing
  virtual ~flow_reactor();

  // parse generic flow reactor input
  virtual void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_value, bad_type); // this, reactor::parse()
  // initialize the reactor and its solution variables
  virtual void initialize(model_species::seq_citer species_begin,
			  model_species::seq_citer species_end)
    throw (bad_type, bad_value); // this, reactor::initialize(),
				// model_species::get_quantity(),
				// model_species::set_quantity()
  // return the total molar flow rate
  double get_flow() const;
  // set the total molar flow rate, return old
  double set_flow(double flow_);
  // add INCREMENT to total molar FLOW, return new total
  double add_to_flow(double increment);
}; // end class flow_reactor

CH_END_NAMESPACE

#endif // not CH_MODEL_REACTOR_H

/* $Id: reactor.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
