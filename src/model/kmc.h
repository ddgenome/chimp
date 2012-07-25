// -*- C++ -*-
// Kinetic Monte Carlo integration class.
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
#ifndef CH_MODEL_KMC_H
#define CH_MODEL_KMC_H 1

#include <fstream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "counter.h"
#include "except.h"
#include "ensemble.h"
#include "environment.h"
#include "integrate.h"
#include "lattice.h"
#include "rng.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// class for the setup and integration using kinetic Monte Carlo
class kmc : public integrator
{
  // typedef
  typedef CH_STD::map<ensemble*,environment*> ens_env_map;
  typedef ens_env_map::iterator ens_env_map_iter;
  typedef ens_env_map::const_iterator ens_env_map_citer;
  typedef CH_STD::map<ensemble,ens_env_map> ensemble_map;
  typedef ensemble_map::iterator ensemble_map_iter;
  typedef ensemble_map::const_iterator ensemble_map_citer;
  typedef CH_STD::pair<ensemble_map_iter,ensemble_map_iter>
    ensemble_map_iter_pair;
  typedef CH_STD::map<model_reaction*,ensemble_map_iter_pair>
    rxn_ensemble_iter_map;
  typedef rxn_ensemble_iter_map::iterator rxn_ensemble_iter_map_iter;
  typedef rxn_ensemble_iter_map::const_iterator rxn_ensemble_iter_map_citer;

private:
  rng* random;			// random number generator
  unsigned int sites;		// total number of surface sites
  lattice surface;		// catalyst surface
  environment::seq environments; // the environments for all the lattice points
  ensemble_map ensembles;	// map of what ensembles are available
  rxn_ensemble_iter_map rxn_ens; // forward/reverse ensembles for a reaction
  unsigned int max_coordination; // highest coordination of a surface species
  unsigned int max_sites;	// maximum number of sites needed in reactions
  CH_STD::string surface_filename; // file to output surface snapshots to
  CH_STD::ofstream surface_out;	// stream to output surface snapshots to
  unsigned int steps;		// how many kmc steps have been taken
  bool event_rate;		// are rate constants event based?
  double scale;			// scale-up factor for reactors
  // rate scale factor for coverages and unit conversion
  CH_STD::map<model_reaction*,CH_STD::pair<double,double> > rate_scale;
  // times each rxn performed
  CH_STD::map<model_reaction*,CH_STD::pair<counter,counter> > rxn_count;
  CH_STD::string count_filename; // name of file to output count into
  CH_STD::ofstream count_out;	// file to output rxn counter
  CH_STD::string env_type;	// input for environment type
  bool env_radial;		// environment site creation scheme

private:
  // prevent assignment
  kmc& operator=(const kmc&);
  // ctor: copy
  explicit kmc(const kmc& original)
    throw (bad_pointer); // this, integrator::integrator()
  // change the random number generator, return pointer to new one
  void set_rng(const CH_STD::string& type)
    throw (bad_type); // this
  // set the random number seed, return old one
  ul_int set_rng_seed(ul_int seed);
  // set everything up
  virtual void initialize()
    throw (bad_pointer, bad_input, bad_value, bad_type, bad_request); // this,
				// lattice::initialize(), initial_coverage(),
				// create_environments(), create_ensembles(),
				// model_reaction::set_amount_type(),
				// integrator::initialize(),
				// reactor::kmc_initialize(),
				// calc_rate_scale()
  // calculate the maximum surface coordination of all species in model,
  // return max_coordination
  unsigned int calc_max_coordination();
  // loop through the reactions and insert the surface reactants into ensembles
  void create_ensembles(model_species* empty_site)
    throw (bad_input); // this, model_reaction::get_reactant_seq(),
                       // model_reaction::get_product_seq()
  // setup rxn counter, if we need to
  void initialize_rxn_counter();
  // create and initialize the environments, fill ensembles
  void create_environments(model_species* empty_site)
    throw (bad_request, bad_value, bad_pointer, bad_input); // this,
				// lattice::get_point(),
				// environment::set_neighbors(),
				// environment::initialize()
  // get all the ensembles from the environments, inserting those we are
  // interested in
  void get_ensembles();
  // get the ensembles from the given environments
  void get_ensembles(environment::group& changed_environments);
  // factor to scale surface rates by to account for coverage ``units''
  // and reactor units
  void calc_rate_scale()
    throw (bad_request, bad_value, bad_type); //  this, reactor::kmc_rate(),
				// count_sites(), site_scale(),
				// multisite_scale()
  // fill up array which holds count of each size site
  void count_sites(CH_STD::vector<counter>& site_count) const
    throw (bad_value);	// this
  // return the inverse of the total possible sites available for this rxn
  double site_scale(rxn_ensemble_iter_map_iter rxn_ens_it,
		    const CH_STD::vector<counter>& site_count) const
    throw (bad_value);	// get_site_count()
  // return the current count of sites having the same size as reaction's
  int get_site_count(const CH_STD::vector<counter>& site_count, int size) const
    throw (bad_value);		// this
  // scaling correction for reactions with multisite species
  void multisite_scale(rxn_ensemble_iter_map_iter rxn_ens_it,
		       const CH_STD::vector<counter>& site_count,
		       double& f_scale, double& r_scale) const
    throw (bad_value);		// multisite_scale()
  // calculate multisite scaling for given reacting species
  double multisite_scale(const model_species::seq& speciess,
			 const CH_STD::vector<counter>& site_count) const
    throw (bad_value);		// get_site_count()
  // determine factor to account for coverage-based rate constant
  void coverage_scale(model_reaction* rxn, double& f_scale, double& r_scale)
    const;
  // go through the reacting species and determine the maximum rate
  double coverage_scale(const stoich_map& reactants) const;
  // initialize the surface to the appropriate coverages
  void initial_coverage(model_species* empty_site)
    throw (bad_input, bad_request, bad_value, bad_pointer, bad_type); // this,
				// perform_reaction(),
				// model_species::set_quantity(),
				// model_species::get_quantity()
  // method to handle a kinetic Monte Carlo integration
  virtual double step(double ti, double tf)
    throw (bad_pointer, bad_type, bad_value, bad_request, bad_input);
				// select_reaction(), perform_reaction(),
				// reactor::kmc_step()
  // calculate total probability and select a reaction to be performed
  // return that reaction, its ensembles, and total transition probability
  // the sign of the total transition probability determines the direction
  // of the reaction
  CH_STD::pair<rxn_ensemble_iter_map_iter,double> select_reaction()
    throw (bad_pointer, bad_type, bad_request); // this, get_net_rate()
  // calculate the net reaction rate of a reaction
  double get_net_rate(rxn_ensemble_iter_map_citer rxn_ens_it) const
    throw (bad_pointer, bad_type, bad_request); // this,
				// model_reaction::get_fluid_forward_rate(),
				// model_reaction::get_fluid_reverse_rate(),
				// model_reaction::get_forward_rate(),
				// model_reaction::get_reverse_rate(),
				// check_quantities()
  // make sure if reaction is performed, no quantities will become invalid
  void check_quantities(model_reaction* rxn, double& f_rate, double& r_rate)
    const
    throw (bad_type);		// check_quantities()
  // perform the given reaction, update surface and species quantities
  bool perform_reaction(model_reaction& rxn)
    throw (bad_request, bad_value, bad_pointer, bad_input, bad_type); // this
				// perform_reaction(),
				// model_reaction::get_reactant_seq()
  void perform_reaction(rxn_ensemble_iter_map_iter rxn_for_rev, double rate)
    throw (bad_value, bad_pointer, bad_request, bad_input, bad_type); // this,
				// environment::change_ensemble(),
				// model_reaction::get_reactant_seq(),
				// model_reaction::get_product_seq(),
				// model_species::add_to_quantity(),
				// reactor::kmc_eqn()
  // delete the ensembles in the deque
  void delete_ensembles(ensemble::deq& old_ensembles);
  // output first row of file
  virtual void output_header();
  // method to output progress of integration
  virtual void output(double x)
    throw (bad_type, bad_request, bad_value); // output()
  // output the current output point and its values to the given stream
  virtual void output(double x, CH_STD::ostream& output_stream)
    throw (bad_type, bad_request, bad_value); // integrate::output(),
				// lattice::stringify()
public:
  // ctor: (default) set size to default and create default rng
  kmc();
  // dtor: delete the rng pointer and environments
  virtual ~kmc();

  // parse integrator input
  virtual void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input, bad_request, bad_value, bad_type, bad_pointer); // this,
				// lattice::set_size(), state::parse(),
				// set_rng(), rng::parse()
  // copy the integrator and return pointer to new object
  virtual integrator* copy() const
    throw (bad_pointer); // kmc()
}; // end class kmc

CH_END_NAMESPACE

#endif // not CH_MODEL_KMC_H

/* $Id: kmc.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
