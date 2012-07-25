// Methods for setting up and executing model solutions.
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

#include "kmc.h"
#include <cmath>
#include <cstdlib>
#include <typeinfo>
#include "compare.h"
#include "constant.h"
#include "debug.h"
#include "point.h"
#include "precision.h"
#include "quantity.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// kmc methods
// ctor: (default) set up defaults
kmc::kmc()
  : integrator(), random(0), sites(0U), surface(), environments(), ensembles(),
    rxn_ens(), max_coordination(0U), max_sites(0U), surface_filename(),
    surface_out(), steps(0U), event_rate(false), scale(1.0e0), rate_scale(),
    rxn_count(), count_out(), env_type("nn"), env_radial(true)
{
  // set random to default rng
  random = rng::new_rng();
  // set the total number of sites
  sites = surface.get_size() * surface.get_size();
}

// ctor: copy
kmc::kmc(const kmc& o)
  throw (bad_pointer)
  : integrator(o), random(0), sites(o.sites), surface(o.surface),
    environments(o.environments), ensembles(o.ensembles), rxn_ens(o.rxn_ens),
    max_coordination(o.max_coordination), max_sites(o.max_sites),
    surface_filename(o.surface_filename), surface_out(), steps(0U),
    event_rate(o.event_rate), scale(o.scale), rate_scale(o.rate_scale),
    rxn_count(o.rxn_count), count_out(), env_type(o.env_type),
    env_radial(o.env_radial)
{
  // make sure random on original was set
  if (o.random == 0)
    {
      // something is very wrong
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":kmc::kmc(): original random number generator "
			"pointer has gotten corrupted and is zero");
    }
  // make a copy
  random = o.random->copy();
}

// dtor: delete the rng
kmc::~kmc()
{
  // we own the random number generator
  delete random;
  // loop through and delete the environments
  for (environment::seq_iter it(environments.begin());
       it != environments.end(); ++it)
    {
      delete *it;
      *it = 0;
    }
}

// kmc private methods
// change the random number generator
void
kmc::set_rng(const CH_STD::string& type)
  throw (bad_type)
{
  // delete old rng
  delete random;
  // get new one
  random = rng::new_rng(type);
  // make sure we got a good one
  if (random == 0)
    {
      throw bad_type(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":kmc::set_rng(): unable to create a random number "
		     " generator of type " + type);
    }
  return;
}

// change the random number generator seed
ul_int
kmc::set_rng_seed(ul_int seed)
{
  return random->set_seed(seed);
}

// set everything up
void
kmc::initialize()
  throw (bad_pointer, bad_input, bad_value, bad_type, bad_request)
{
  // call the base class initializer
  integrator::initialize();
  // call the kmc initializer for the reactor, and reset scale
  scale = state_info->get_reactor()->kmc_initialize(sites, scale);
  // calculate the maximum coordination needed by a surface species
  // max_sites should be at least as big as max_coordination
  max_sites = calc_max_coordination();
  // see if we need to initialize the surface
  model_species* empty(0);
  if (max_coordination > 0U)
    {
      // might have reactor sites, but not want to use lattice
      if (sites > 0U)
	{
	  // change the precision for coverages
	  precision::get().set_coverage(1.0e-1 / sites);
	  // open up the surface output file, if one was specified
	  if (surface_filename.size() > 0U)
	    {
	      // open the surface file, destroying contents
	      // NOTE: could use safe_ofstream here
	      surface_out.open(surface_filename.c_str());
	    }
	}
      else
	{
	  // get the number of sites in the reactor
	  double reactor_sites(state_info->get_reactor()->get_sites());
	  if (reactor_sites > precision::get().get_double())
	    {
	      // change the precision for coverages
	      precision::get().set_coverage(1.0e-1 * scale / reactor_sites);
	    }
	}
      // get the empty surface species (lattice needs it)
      empty = mech->get_species("@");
      // make sure empty site exists
      if (empty == 0)
	{
	  throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":kmc::initialize(): surface has non-zero "
			    "number of sites, but an empty site was not "
			    "included in the model");
	}
    }
  else				// better safe than sorry?
    {
      // make sure the surface has zero size
      surface.set_size(0U);
    }
  // create ensemble for all reactions and calculate max_sites
  create_ensembles(empty);
  // set up the reaction counter
  initialize_rxn_counter();
  // initialize surface to be empty (does nothing if surface size is zero)
  surface.initialize(empty);
  // create environments and initialize them
  create_environments(empty);
  // get ensembles of interest
  get_ensembles();
  // calculate the scaling factor for each reaction
  calc_rate_scale();
  // fill the surface with the apropriate initial coverages
  initial_coverage(empty);
  return;
}

// calculate the maximum surface coordination of all species in model
unsigned int
kmc::calc_max_coordination()
{
  for (model_species::seq_citer it(mech->species_seq_begin());
       it != mech->species_seq_end(); ++it)
    {
      unsigned int coord((*it)->get_surface_coordination());
      max_coordination = (coord > max_coordination) ? coord : max_coordination;
    }
  return max_coordination;
}

// loop through the reactions and insert the surface reactants into ensembles
void
kmc::create_ensembles(model_species* empty_site)
  throw (bad_input)
{
  // determine what ensembles are needed for reactions
  for (model_reaction::seq_citer it(mech->reaction_seq_begin());
       it != mech->reaction_seq_end(); ++it)
    {
      // get the forward and reverse ensembles
      ensemble reactant_ensemble((*it)->get_reactant_seq());
      ensemble product_ensemble((*it)->get_product_seq());
      // get the total coordination of the reactant ensemble
      unsigned int coord(reactant_ensemble.get_coordination());
      // make sure they have the same total coordination
      if (coord != product_ensemble.get_coordination())
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":kmc::create_ensembles(): reaction reactants and "
			  "products do not have the same total coordination "
			  "for reaction `" + (*it)->stringify() + "'");
	}
      // insert only if non-zero coordination and lattice being used
      if (coord > 0U && sites > 0U)
	{
	  // calculate max_sites
	  max_sites = (coord > max_sites) ? coord : max_sites;
	  // insert the reactant ensemble and empty map into ensembles
	  // assign the iterator to forward
	  ensemble_map_iter_pair forward_reverse;
	  forward_reverse.first = 
	    ensembles.insert(CH_STD::make_pair(reactant_ensemble,
					       ens_env_map())).first;
	  // should we insert the reverse reaction as well?
	  if ((*it)->is_reversible())
	    {
	      forward_reverse.second =
		ensembles.insert(CH_STD::make_pair(product_ensemble, 
						   ens_env_map())).first;
	    }
	  else			// not reversible
	    {
	      forward_reverse.second = ensembles.end();
	    }
	  // insert reaction and ensemble iterators into the map
	  rxn_ens.insert(CH_STD::make_pair(*it, forward_reverse));
	}
      else
	{
	  // reaction does not involve surface or lattice not used
	  rxn_ens.insert(CH_STD::make_pair(*it,
					   CH_STD::make_pair(ensembles.end(),
							     ensembles.end())));
	}
    }
  // make sure ensembles of empty sites <= max_coordination are included
  // (for surface initial coverages)
  if (sites > 0U)		// only necessary if lattice is used
    {
      for (unsigned int i(1U); i <= max_coordination; ++i)
	{
	  // create sequence of I empty sites
	  model_species::seq empties(i, empty_site);
	  // create an ensemble with that sequence
	  ensemble empty_ens(empties);
	  // insert the ensemble into the map
	  ensembles.insert(CH_STD::make_pair(empty_ens, ens_env_map()));
	}
    }
  return;
}

// setup rxn counter, if we need to
void
kmc::initialize_rxn_counter()
{
  // see if an output file was given
  if (count_filename.size() > 0U)
    {
      // open the counter file, destroying contents
      // NOTE: could use safe_ofstream here
      count_out.open(count_filename.c_str());
      // set up the counting map, loop though all the reactions
      for (model_reaction::seq_citer it(mech->reaction_seq_begin());
	   it != mech->reaction_seq_end(); ++it)
	{
	  // insert this reaction into the map
	  rxn_count.insert(CH_STD::make_pair(*it, CH_STD::make_pair(counter(), counter())));
	}
    }
  return;
}

// create and initialize the environments
void
kmc::create_environments(model_species* empty_site)
  throw (bad_request, bad_value, bad_pointer, bad_input)
{
  // set environment static variables
  // set the neighbor type
  environment::set_environment_type(env_type);
  // set how to create the sites
  environment::site_type_radial(env_radial);
  // set the max_sites in the environment
  environment::set_max_sites(max_sites);
  // set the environments rng
  environment::set_rng(random);
  // set the empty site species
  environment::set_empty_site(empty_site);
  // get the size of the surface
  unsigned int surface_size(surface.get_size());
  // CREATE THE ENVIRONMENTS
  // note: this is a killer, having to loop through three times to create,
  // connect, and initialize the environments
  // create a matrix of environments with the correct number of rows
  environment::matrix env_surface(surface_size);
  // get iterator to the first row
  environment::matrix_iter row_it(env_surface.begin());
  // loop through and create the columns
  for (unsigned int row(0U); row < surface_size; ++row)
    {
      // reserve the proper length of the column
      row_it->reserve(surface_size);
      // loop through the columns
      for (unsigned int col(0U); col < surface_size; ++col)
	{
	  // create environment pointer
	  environment* ep = new environment(surface.get_point(row, col));
	  // put it in the matrix
	  row_it->push_back(ep);
	  // put it in the sequence
	  environments.push_back(ep);
	}
      // go to next row
      ++row_it;
    }
  // now loop through all environments and have them set their neighbors
  for (environment::seq_iter eit(environments.begin());
       eit != environments.end(); ++eit)
    {
      // determine which environments are neighbors
      (*eit)->set_neighbors(env_surface);
    }
  // now loop through again and initialize the surface environments
  // this calculates all possible sites
  for (environment::seq_iter eit(environments.begin());
       eit != environments.end(); ++eit)
    {
      // call the initializer and get all the sites (sets of environments)
      (*eit)->initialize();
    }
  return;
}

// get all the ensembles from the environments, inserting those we are
// interested in; also count ensembles by size
void
kmc::get_ensembles()
{
  // create a set from the vector of environments
  environment::group env_set(environments.begin(), environments.end());
  // insert the ensembles currently offered by the surface into ensembles
  get_ensembles(env_set);
  return;
}

// get the ensembles from the given environments and put them in ensembles
void
kmc::get_ensembles(environment::group& changed_environments)
{
  // loop through environments and get the ensembles offered by each
  for (environment::group_iter env_it(changed_environments.begin());
       env_it != changed_environments.end(); ++env_it)
    {
      // loop through the ensembles at this environment
      for (ensemble::seq_citer it((*env_it)->ensembles_seq_begin());
	   it != (*env_it)->ensembles_seq_end(); ++it)
	{
	  // see if this ensemble is in the current mechanism
	  ensemble_map_iter em_it(ensembles.find(**it));
	  if (em_it != ensembles.end())
	    {
	      // insert this specific ensemble and environment into the map
	      em_it->second.insert(CH_STD::make_pair(*it, *env_it));
	    }
	}
    }
  return;
}

// determine the multiplicative factor to make the rate be consistent
// with non-kMC rate constants, and convert units to 1/time
void
kmc::calc_rate_scale()
  throw (bad_request, bad_value, bad_type)
{
  // first determine how many of each size site is available on the surface
  CH_STD::vector<counter> site_count(max_sites + 1);
  count_sites(site_count);
  // check what type of rate constant is being used
  if (event_rate)
    {
      // loop through all the reactions
      for (rxn_ensemble_iter_map_iter rxn_ens_it(rxn_ens.begin());
	   rxn_ens_it != rxn_ens.end(); ++rxn_ens_it)
	{
	  double scale(1.0e0);
	  // make sure there is a surface ensemble
	  if (rxn_ens_it->second.first != ensembles.end())
	    {
	      // avoid multiple counting of multisite ensembles
	      unsigned int
		coord(rxn_ens_it->second.first->first.get_coordination());
	      if (coord < 1U)
		{
		  throw bad_request(PACKAGE ":" __FILE__ ":" +
				    t_string(__LINE__) +
				    ":kmc::calc_rate_scale(): ensemble for "
				    "reaction " +
				    rxn_ens_it->first->stringify() +
				    " has nonpositive size, an error has "
				    "occurred");
		}
	      // find out how many of this size site we have
	      int count(site_count[coord].get_count());
	      if (count < 1)
		{
		  throw bad_request(PACKAGE ":" __FILE__ ":" +
				    t_string(__LINE__) +
				    ":kmc::calc_rate_scale(): number of "
				    "sites of size " + t_string(coord) +
				    " is nonpositive, so cannot perform "
				    "reaction " +
				    rxn_ens_it->first->stringify());
		}
	      if (env_radial)
		{
		  // each site in an ensemble will count it
		  scale /= coord;
		}
	      else		// neighbor only sites
		{
		  // FIXME: make more general
		  if (coord == 2U)
		    {
		      scale /= 2.0e0;
		    }
		}
	    }
	  // insert the scaling factor
	  rate_scale.insert(CH_STD::make_pair(rxn_ens_it->first,
					      CH_STD::make_pair(scale,
								scale)));
	}
    }
  else				// coverage based rate
    {
      // conversion factor to get rate in units of 1/time
      double reactor_scale(state_info->get_reactor()->kmc_rate());
      // these rates are for the entire reactor, but through scaling
      // we only are performing kmc on a small part and replicating it
      reactor_scale /= scale;
      // loop through all the reactions
      for (rxn_ensemble_iter_map_iter rxn_ens_it(rxn_ens.begin());
	   rxn_ens_it != rxn_ens.end(); ++rxn_ens_it)
	{
	  // initialize the scaling factors for this reaction
	  double f_scale(reactor_scale);
	  double r_scale(reactor_scale);
	  // see if there is a surface ensemble
	  // (non gas-phase reaction using lattice)
	  if (rxn_ens_it->second.first != ensembles.end())
	    {
	      // scaling for the total number of sites this size (``coverage'')
	      double inv_sites(site_scale(rxn_ens_it, site_count));
	      f_scale *= inv_sites;
	      r_scale *= inv_sites;
	      // correction for multisite species in reactions
	      multisite_scale(rxn_ens_it, site_count, f_scale, r_scale);
	      // correct scaling factors for species coverage limits
	      coverage_scale(rxn_ens_it->first, f_scale, r_scale);
	    }
	  // insert this information into the scaling map
	  rate_scale.insert(CH_STD::make_pair(rxn_ens_it->first,
					      CH_STD::make_pair(f_scale,
								r_scale)));
	}
    }
  return;
}

// fill up array which holds count of each size site
void
kmc::count_sites(CH_STD::vector<counter>& site_count) const
  throw (bad_value)
{
  // loop through all the environments
  for (environment::seq_citer env_it(environments.begin());
       env_it != environments.end(); ++env_it)
    {
      // loop through the sites at this environment
      for (environment::group_set_citer site_it((*env_it)->sites_begin());
	   site_it != (*env_it)->sites_end(); ++site_it)
	{
	  unsigned int site_size(site_it->size());
	  // make sure not to overun vector boundaries
	  if (site_size > max_sites)
	    {
	      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::count_sites(): returned site "
			      "size (" + t_string(site_size) + ") is "
			      "greater than size of site array (" +
			      t_string(max_sites) + "), something "
			      "has gone wrong");
	    }
	  // make sure it has something in it
	  else if (site_size < 1U)
	    {
	      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::count_sites(): returned site "
			      "size is zero, which is invalid");
	    }
	  // increment the count of environments of this size
	  ++site_count[site_size];
	}
    }
  return;
}

// return the inverse of the total possible sites available for this rxn
double
kmc::site_scale(rxn_ensemble_iter_map_iter rxn_ens_it,
		const CH_STD::vector<counter>& site_count) const
  throw (bad_value)
{
  // get the size of the ensemble for this reaction
  unsigned int
    ens_coord(rxn_ens_it->second.first->first.get_coordination());
  // get the total ensembles of this size on the surface
  int possible_sites(get_site_count(site_count, ens_coord));
  // make sure there are sets of environments of this size
  // (if environment type is Esingle, possible_sites could be zero)
  if (possible_sites > 0)
    {
      // rate should be divide by the total possible ensembles of this
      // size on lattice; this gives something like a coverage
      return 1.0e0 / possible_sites;
    }
  // else this reaction is not possible
  return 0.0e0;
}

// return the count of sites of the given size in the vector, checking
// for errors
int
kmc::get_site_count(const CH_STD::vector<counter>& site_count, int size) const
  throw (bad_value)
{
  // check validity of size
  if (size < 1)
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":kmc::get_site_count(): ensemble size ("
		      + t_string(size) + ") must be greater than 1");
    }
  // make sure the size is less than or equal to the size of the site array
  else if (size > (int) max_sites)
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":kmc::get_site_count(): ensemble size ("
		      + t_string(size) + ") exceeds the "
		      "previously calculated maximum (" +
		      t_string(max_sites) + ")");
    }
  // else
  return site_count[size].get_count();
}

// scaling correction for reactios with multisite species
void
kmc::multisite_scale(rxn_ensemble_iter_map_iter rxn_ens_it,
		     const CH_STD::vector<counter>& site_count,
		     double& f_scale, double& r_scale) const
  throw (bad_value)
{
  // double check if we are using the surface
  if (sites > 0U)
    {
      // get the reactants scaling factor
      f_scale *= multisite_scale(rxn_ens_it->first->get_reactant_seq(),
				 site_count);
      // see if we need to consider the reverse reaction
      if (rxn_ens_it->first->is_reversible())
	{
	  // get scaling factor for the products
	  r_scale *= multisite_scale(rxn_ens_it->first->get_product_seq(),
				     site_count);
	}
    }
  return;
}

// calculate multisite scaling for given reacting species
double
kmc::multisite_scale(const model_species::seq& speciess,
		     const CH_STD::vector<counter>& site_count) const
  throw (bad_value)
{
  // scale factor (return value)
  double ms_scale(1.0e0);
  // loop through all the species
  for (model_species::seq_citer sp_it(speciess.begin());
       sp_it != speciess.end(); ++sp_it)
    {
      // get the surface coordination of the species
      unsigned int coord((*sp_it)->get_surface_coordination());
      // see if species is a multisite species
      if (coord > 1U)
	{
	  // the actual max for multisite species is just the number of sites
	  // so alter the scale to account for this
	  // undo the ensemble scaling
	  ms_scale *= get_site_count(site_count, coord);
	  // and scale by the number of sites
	  ms_scale /= sites;
	}
    }
  return ms_scale;
}

// factor to scale surface rates by to account for coverage limits
void
kmc::coverage_scale(model_reaction* rxn, double& f_scale, double& r_scale)
  const
{
  // double check if we are using the surface
  if (sites > 0U)
    {
      // get the reactants scaling factor
      f_scale *= coverage_scale(rxn->get_reactants());
      // see if we need to consider the reverse reaction
      if (rxn->is_reversible())
	{
	  // get scaling factor for the products
	  r_scale *= coverage_scale(rxn->get_products());
	}
    }
  return;
}

// go through the reacting species and determine the maximum attainable
// rate given the types of surface species reacting
/* the scaling for rates to get maximum allowed by product of species
 * coverages:
 *                                        -
 *                 N                / N   \
 *                ---         n    / ---  / n
 *                | |          i  /  \    -  i
 *                | | (n / m )   /   /   n
 *                i=1   i   i   /    ---  i
 *                             /     i=1
 *
 * where n_i is the power the coverage is raised to for species i, m_i
 * is its coordination, and the sums and product are over all
 * _surface_ species
 */
double
kmc::coverage_scale(const stoich_map& reactants) const
{
  // scaling factor
  double cov_scale(1.0e0);
  // sum total of all the exponents
  double sum(0.0e0);
  // loop through each species
  for (stoich_map_citer it(reactants.begin()); it != reactants.end(); ++it)
    {
      // determine if it is a surface species
      unsigned int coord(it->first->get_surface_coordination());
      if (coord > 0U)
	{
	  // update the scaling factor
	  double power(it->second.get_power());
	  cov_scale *= CH_STD::pow(power / coord, power);
	  sum += power;
	}
    }
  // divide by denominator of scaling factor expression
  cov_scale /= CH_STD::pow(sum, sum); // 0^0 = 1
  return cov_scale;
}

// initialize the surface to the appropriate coverages
void
kmc::initial_coverage(model_species* empty_site)
  throw (bad_input, bad_request, bad_value, bad_pointer, bad_type)
{
  // see if we have to do anything
  if (!empty_site)
    {
      return;
    }
  // keep track and make sure total coverage < 1.0e0
  double total_coverage(0.0e0);
  // coverage of empty sites
  double empty_coverage(0.0e0);
  // loop through all the species
  for (model_species::seq_citer sp_it(mech->species_seq_begin());
       sp_it != mech->species_seq_end(); ++sp_it)
    {
      // see if this is the empty site
      if (*sp_it == empty_site)
	{
	  // assign its user-given coverage to the local variable
	  empty_coverage = (*sp_it)->get_quantity();
	  // set empty site coverage to one (1) (what is on the surface now)
	  (*sp_it)->set_quantity(quantity::Econcentration, 1.0e0);
	}
      else			// see if it is a surface species
	{
	  // get surface coordination
	  unsigned int coord((*sp_it)->get_surface_coordination());
	  // get coverage (or quantity)
	  double coverage((*sp_it)->get_quantity());
	  // see if it is a surface species an if its coverage is nonzero
	  if (coord > 0U && coverage > 0.0e0)
	    {
	      // make sure the addition of this species does not cause the
	      // total coverage to exceed one (1)
	      if (total_coverage + coverage * coord
		  > 1.0e0 + (5.0e-1 * coord) / sites)
		{
		  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
				  + ":kmc::initial_coverage(): sum of initial "
				  "coverages for all species exceeds 1.0e0 ("
				  + t_string(total_coverage + coverage) + ")");
		}
	      // put that coverage on the surface
	      if (sites > 0U)	// must place on lattice
		{
		  // create a reaction which places this species
		  model_reaction rxn(static_cast<k*>(0));
		  // insert the empty_sites as reactants
		  rxn.add_reactant(empty_site, coord);
		  // insert this species as the product
		  rxn.add_product(*sp_it);
		  // set the current coverage to zero
		  (*sp_it)->set_quantity(quantity::Econcentration, 0.0e0);
		  // perform the reaction until desired coverage is reached
		  while ((*sp_it)->get_quantity() < coverage - 5.0e-1 / sites)
		    {
		      // perform the reaction and make sure it goes
		      if (!perform_reaction(rxn))
			{
			  // no more of the required ensemble type
			  break;	// while (coverage)
			}
		      // debugging information
		      if (debug::get().get_level() > 2U)
			{
			  // output surface and quantity information
			  output(0.0e0, debug::get().get_stream());
			}
		    }
		}
	      // increment the total coverage
	      total_coverage += (*sp_it)->get_quantity() * coord;
	    }
	}
    }
  // make sure empty site coverage (if given) is also consistent
  double left_over(1.0e0 - total_coverage);
  if (empty_coverage > precision::get().get_coverage())
    {
      if (empty_coverage < left_over - precision::get().get_coverage()
	  || empty_coverage > left_over + precision::get().get_coverage())
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":kmc::initial_coverage(): given empty site surface "
			  "coverage does not agree with that left over after "
			  "placing all other surface species");
	}
    }
  // regardless, set to exactly what is left over
  // (first argument is ignored for surface species)
  empty_site->set_quantity(quantity::Econcentration, left_over);
  return;
}

// method to handle a kinetic Monte Carlo solution
double
kmc::step(double xi, double xf)
  throw (bad_pointer, bad_type, bad_value, bad_request, bad_input)
{
  // step within a try block so we can output information if it fails
  try
    {
      // perform Monte Carlo steps until the final time is reached
      while (xi < xf)
	{
	  // appropriately choose a reaction
	  CH_STD::pair<rxn_ensemble_iter_map_iter,double>
	    rxn_for_rev_it_rate(select_reaction());
	  // debugging information
	  if (debug::get().get_level() > 1U)
	    {
	      // output the reaction performed and time
	      debug::get().get_stream() << "kmc step " << steps + 1
		<< ":x = " << xi << ":reaction "
		<< rxn_for_rev_it_rate.first->first->stringify()
		<< CH_STD::endl;
	    }
	  // perform the reaction
	  perform_reaction(rxn_for_rev_it_rate.first,
			   rxn_for_rev_it_rate.second);
	  // get the time step (inverse of total transistion probability)
	  double dx(-(CH_STD::log(random->get_random_open_open())
		      / CH_STD::fabs(rxn_for_rev_it_rate.second)));
	  // have the reactor update everything
	  state_info->get_reactor()->kmc_step(mech->species_seq_begin(),
					      mech->species_seq_end(), dx);
	  // update the independent variable
	  xi += dx;
	  // increment the kmc step counter
	  ++steps;
	  if (debug::get().get_level() > 2U)
	    {
	      // output surface and quantity information
	      output(xi, debug::get().get_stream());
	    }
	}
    }
  catch (CH_STD::exception& e)
    {
      // say we caught an exception
      *out_file << "# caught exception" << CH_STD::endl;
      // try to print out current output values
      output(xi);
      // throw it again
      throw;
    }
  return xi;
}

// calculate total probability and select a reaction to be performed
// return that reaction, its ensembles, and total transition probability
// the sign of the total transition probability determines the direction
// of the reaction
CH_STD::pair<kmc::rxn_ensemble_iter_map_iter,double>
kmc::select_reaction()
  throw (bad_pointer, bad_type, bad_request)
{
  // the total transition (reaction) probability
  double total_rate(0.0e0);
  // map of the reactions and their rates (to determine direction of reaction)
  CH_STD::map<model_reaction*,double> rates;
  // the map of the cumulative rate and reaction it corresponds to
  CH_STD::map<double,rxn_ensemble_iter_map_iter> cum_rates;
  // get the rates for each reaction in the mechanism
  for (rxn_ensemble_iter_map_iter rxn_ens_it(rxn_ens.begin());
       rxn_ens_it != rxn_ens.end(); ++rxn_ens_it)
    {
      // get the net rate of the reaction
      double net_rate(get_net_rate(rxn_ens_it));
      // insert net rate into map
      rates.insert(CH_STD::make_pair(rxn_ens_it->first, net_rate));
      // update the total rate for all moves (reactions)
      double old_total(total_rate);
      total_rate += CH_STD::fabs(net_rate);
      // make sure net rate != 0
      if (total_rate > old_total)
	{
	  // insert new total rate and the corresponding reaction/ensemble
	  // iterator into cumulative map
	  cum_rates.insert(CH_STD::make_pair(total_rate, rxn_ens_it));
	}
    }
  // make sure a reaction is possible
  if (total_rate < precision::get().get_double())
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":kmc::select_reaction(): sum total of all absolute "
			"rates is equivalent to zero: "
			+ t_string(total_rate));
    }
  // get random number in range [0, total_rate)
  double r(random->get_random_open(total_rate));
  // insert into cum_rates map and get iterator
  CH_STD::pair<CH_STD::map<double,rxn_ensemble_iter_map_iter>::iterator,bool>
    rate_rxn_ens_it_bool(cum_rates.insert(CH_STD::make_pair(r, rxn_ens.end())));
  // next entry is the one we want
  ++rate_rxn_ens_it_bool.first;
  // get the rate
  CH_STD::map<model_reaction*,double>::iterator
    rate_it(rates.find(rate_rxn_ens_it_bool.first->second->first));
  // make sure it found it
  if (rate_it == rates.end())
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":kmc::select_reaction(): rate/reaction map has "
			"been corrupted");
    }
  // determine sign of rate
  if (rate_it->second < 0.0e0)
    {
      // reverse reaction
      total_rate *= -1.0e0;
      // increment the counter, if necessary
      if (count_out.is_open())
	{
	  ++(rxn_count[rate_rxn_ens_it_bool.first->second->first].second);
	}
    }
  // forward reaction
  else if (count_out.is_open())
    {
      ++(rxn_count[rate_rxn_ens_it_bool.first->second->first].first);
    }
  // return the pair
  return CH_STD::make_pair(rate_rxn_ens_it_bool.first->second, total_rate);
}

// calculate the net reaction rate of a reaction
double
kmc::get_net_rate(rxn_ensemble_iter_map_citer rxn_ens_it) const
  throw (bad_pointer, bad_type, bad_request)
{
  // set up needed rate variables
  double f_rate(0.0e0);
  double r_rate(0.0e0);
  if (sites > 0U)		// lattice is used
    {
      // get the rates excluding the surface species
      f_rate = rxn_ens_it->first->get_fluid_forward_rate(state_info->get_reactor()->get_temperature());
      r_rate = rxn_ens_it->first->get_fluid_reverse_rate(state_info->get_reactor()->get_temperature());
    }
  else				// no lattice
    {
      // get the rates for the reaction
      f_rate = rxn_ens_it->first->get_forward_rate(state_info->get_reactor()->get_temperature());
      r_rate = rxn_ens_it->first->get_reverse_rate(state_info->get_reactor()->get_temperature());
    }
  // make sure it is ok to perform this reaction
  check_quantities(rxn_ens_it->first, f_rate, r_rate);
  // see if there is a surface ensemble (non gas-phase reaction using lattice)
  if (rxn_ens_it->second.first != ensembles.end())
    {
      // find out how many of this reactions ensemble type we have
      f_rate *= rxn_ens_it->second.first->second.size();
      // check if reaction is reversible
      if (rxn_ens_it->second.second != ensembles.end())
	{
	  // find out how many of this reactions ensemble type we have
	  r_rate *= rxn_ens_it->second.second->second.size();
	}
    }
  // scale the rates to proper amount and units
  // find the iterator to the rate scaling values
  CH_STD::map<model_reaction*,CH_STD::pair<double,double> >::const_iterator
    scale_it(rate_scale.find(rxn_ens_it->first));
  // make sure it got found
  if (scale_it == rate_scale.end())
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":kmc::get_net_rate(): tried to find scale factor "
			"for reaction `" + rxn_ens_it->first->stringify() +
			"' to convert to proper units, but its scaling "
			"information was not entered into the map");
    }
  f_rate *= scale_it->second.first;
  r_rate *= scale_it->second.second;
  // calculate net rate
  double net_rate(f_rate - r_rate);
  // output reaction and its rates
  if (debug::get().get_level() > 2U)
    {
      debug::get().get_stream() << '\t' << rxn_ens_it->first->stringify()
				<< ":f=" << f_rate << ";r=" << r_rate << ";n="
				<< net_rate << ";" << CH_STD::endl;
    }
  return net_rate;
}

// given the reactor sites and scaling, see if there are enough of
// each species to complete the reaction
void
kmc::check_quantities(model_reaction* rxn, double& f_rate, double& r_rate)
  const
  throw (bad_type)
{
  // see if there even is a chance of this forward reaction
  if (f_rate > precision::get().get_double())
    {
      // check the forward reaction
      if (!state_info->get_reactor()->kmc_quantities(rxn->get_net_coefficients(), scale))
	{
	  f_rate = 0.0e0;
	}
    }
  else
    {
      f_rate = 0.0e0;
    }
  if (r_rate > precision::get().get_double())
    {
      // check for the reverse reaction
      if (!state_info->get_reactor()->kmc_quantities(rxn->get_net_coefficients(), - scale))
	{
	  r_rate = 0.0e0;
	}
    }
  else
    {
      r_rate = 0.0e0;
    }
  return;
}

// perform the given reaction, update surface and species quantities
bool
kmc::perform_reaction(model_reaction& rxn)
  throw (bad_request, bad_value, bad_pointer, bad_input, bad_type)
{
  // create ensemble for reactants
  ensemble reactant_ens(rxn.get_reactant_seq());
  // reactants should just be empty sites, and should be in the map
  ensemble_map_iter em_it(ensembles.find(reactant_ens));
  // make sure it found it
  if (em_it == ensembles.end())
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":kmc::perform_reaction(): empty site ensemble "
			"required not found in ensemble map, something is "
			"terribly wrong");
    }
  // make sure the map for this ensemble is not empty
  if (em_it->second.empty())
    {
      return false;
    }
  // create a rxn_ensemble_iter_map with just this entry
  rxn_ensemble_iter_map tmp_map;
  // insert the single entry and save the iterator
  rxn_ensemble_iter_map_iter tmp_it(tmp_map.insert(CH_STD::make_pair(&rxn, CH_STD::make_pair(em_it, ensembles.end()))).first);
  // perform the reaction
  perform_reaction(tmp_it, 1.0e0);
  return true;
}

// perform the given reaction, update surface and species quantities
void
kmc::perform_reaction(rxn_ensemble_iter_map_iter rxn_for_rev, double rate)
  throw (bad_value, bad_pointer, bad_request, bad_input, bad_type)
{
  // iterator to the map entry containing the map of iterators to choose from
  ensemble_map_iter ens_map_it;
  // lists of reactants and products
  const model_species::seq* reactants(0);
  const model_species::seq* products(0);
  // select a reactant ensemble for this reaction
  if (rate > 0.0e0)
    {
      // forward reaction
      ens_map_it = rxn_for_rev->second.first;
      // set up lists of reactants and products
      reactants = rxn_for_rev->first->get_reactant_seq_ptr();
      products = rxn_for_rev->first->get_product_seq_ptr();
    }
  else if (rate < 0.0e0)
    {
      // reverse reaction
      ens_map_it = rxn_for_rev->second.second;
      // set up lists of reactants and products
      reactants = rxn_for_rev->first->get_product_seq_ptr();
      products = rxn_for_rev->first->get_reactant_seq_ptr();
    }
  else
    {
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":kmc::perform_reaction(): specified rate is neither "
		      "positive nor negative");
    }
  // make sure we got reactants and products
  if (reactants == 0 || products == 0)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":kmc::perform_reaction(): unable to create reactants "
		      "or products into a sequence for reaction `" +
		      rxn_for_rev->first->stringify() + "', probably due to "
		      "non-integral stoichiometric coefficient");
    }
  // see if it is a reaction involving the surface
  if (ens_map_it != ensembles.end())
    {
      // get the size of the map of ensembles
      unsigned int size(ens_map_it->second.size());
      // make sure there are ensembles in this map
      if (size < 1U)
	{
	  throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":kmc::perform_reaction(): a reaction was "
			    "requested which has none of its ensembles "
			    "currently on the surface");
	}
      // randomly select an environment/ensemble from the map list of them
      // NOTE: this may be nondeterministic (relies on how memory is allocated)
      unsigned int r(random->get_random(size));
      // get the beginning of the map
      ens_env_map_iter ens_env_it(ens_map_it->second.begin());
      // step through the map until we get the one we want
      for (unsigned int u(0); u < r; ++u) ++ens_env_it;
      // declare needed variables for call to change_ensemble()
      ensemble::deq destroyed_ens;
      environment::group changed;
      // call change_ensemble() to perform the reaction on the surface
      (ens_env_it->second)->change_ensemble(ens_env_it->first, *products,
					    destroyed_ens, changed);
      // delete the ensembles that were destroyed
      delete_ensembles(destroyed_ens);
      // get the new ensembles
      get_ensembles(changed);
    }
  // update coverages and pressures using reactor equations
  // scale the gas-phase molecules changed by scale
  state_info->get_reactor()->kmc_reaction(*reactants, *products, scale);
  return;
}

// delete the ensembles in the deque
void
kmc::delete_ensembles(ensemble::deq& old_ensembles)
{
  // NOTE: this could be time consuming
  // loop through the deque of ensembles that were destroyed (by reaction)
  for (ensemble::deq_iter ens_it(old_ensembles.begin());
       ens_it != old_ensembles.end(); ++ens_it)
    {
      // find the map entry for this kind of ensemble
      ensemble_map_iter ens_map_it(ensembles.find(**ens_it));
      // some ensembles are of no importance, see if this one is
      if (ens_map_it != ensembles.end())
	{
	  // erase this entry from the map of this ensemble, check if it was
	  if (ens_map_it->second.erase(*ens_it) < 1)
	    {
	      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__)
				+ ":kmc::perform_reaction(): an ensemble "
				"was determined to be in the mechanism, "
				"but the pointer to is was not entered "
				"into the deque of ensemble pointers; "
				"something has been corrupted");
	    }
	}
      // delete the ensemble regardless of whether it is in mechanism
      delete *ens_it;
      *ens_it = 0;
    }
  return;
}

// output first row of file
void
kmc::output_header()
{
  // call base class method
  integrator::output_header();
  // output header for kmc steps
  *out_file << "\tsteps";
  // insert a new line and flush the buffer
  *out_file << CH_STD::endl;
  // see if we need to output the reaction counter information
  if (count_out.is_open())
    {
      unsigned int u(0U);
      // output general information
      count_out << "# output column description" << CH_STD::endl
		<< "# " << u++ << ":x" << CH_STD::endl;
      count_out << "# " << u++ << ":total kmc steps" << CH_STD::endl;
      // output all of the reactions in order
      for (CH_STD::map<model_reaction*,CH_STD::pair<counter,counter> >::const_iterator rc_it(rxn_count.begin()); rc_it != rxn_count.end(); ++rc_it)
	{
	  // output each of the reactions
	  count_out << "# " << u++ << ": for/rev steps for "
		    << rc_it->first->stringify() << CH_STD::endl;
	}
    }
  return;
}

// method to output progress of integration
void
kmc::output(double x)
  throw (bad_type, bad_request, bad_value)
{
  output(x, *out_file);
  return;
}

// output the current output point and its values to the given stream
void
kmc::output(double x, CH_STD::ostream& output_stream)
  throw (bad_type, bad_request, bad_value)
{
  // call base class method
  integrator::output(x, output_stream);
  // output the number of kmc steps
  output_stream << '\t' << steps;
  // insert a new line and flush the buffer
  output_stream << CH_STD::endl;
  // output surface, if desired
  if (surface_out.is_open())
    {
      surface_out << "x = " << x << CH_STD::endl
		  << surface.stringify() << CH_STD::endl;
    }
  // see if we need to output the reaction counter information
  if (count_out.is_open())
    {
      // output where we are in the simulation
      count_out << x << '\t' << steps;
      // output all of the reactions in order
      for (CH_STD::map<model_reaction*,CH_STD::pair<counter,counter> >::const_iterator rc_it(rxn_count.begin()); rc_it != rxn_count.end(); ++rc_it)
	{
	  // output count for each of the reactions
	  count_out << '\t' << rc_it->second.first.get_count()
		    << '/' << rc_it->second.second.get_count();
	}
      count_out << CH_STD::endl;
    }
  return;
}

// kmc public methods
// parse integrator input
void
kmc::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input, bad_request, bad_value, bad_type, bad_pointer)
{
  // loop through input
  while (token_it != end)
    {
      if (icompare(*token_it, "begin") == 0)
	{
	  ++token_it;		// next token
	  if (icompare(*token_it, "state") == 0)
	    {
	      // call the state parser
	      state_info->parse(++token_it, end);
	      continue;		// while ()
	    }
	  else if (icompare(*token_it, "rng") == 0)
	    {
	      // change the random number generator
	      set_rng(*++token_it);
	      // call the random number generator parser with the next token
	      random->parse(++token_it, end);
	      continue;		// while ()
	    }
	  else
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::parse(): syntax error in input "
			      "for integrator: do not know how to begin "
			      + *token_it);
	    }
	}
      // set size of lattice
      else if (icompare(*token_it, "size") == 0)
	{
	  // change the size of the lattice
	  int new_size(CH_STD::atoi((++token_it)->c_str()));
	  if (new_size < 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::parse(): syntax error in input "
			      "for integrator: can not set lattice size < 0: "
			      + *token_it);
	    }
	  unsigned int size((unsigned int) new_size);
	  // change the lattice size
	  surface.set_size(size);
	  // update the total number of kmc sites
	  sites = size * size;
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      // set name of surface file
      else if (icompare(*token_it, "surface_file") == 0)
	{
	  // next token is the name of file to output surface snapshots to
	  surface_filename = *++token_it;
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      // set scaling factor
      else if (icompare(*token_it, "scale") == 0)
	{
	  // set the scaling factor equal to the next argument
	  scale = CH_STD::atof((++token_it)->c_str());
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      // set type of rate constant
      else if (icompare(*token_it, "rate_constant") == 0)
	{
	  // get the next token
	  CH_STD::string rc_type(*++token_it);
	  // event based rate constant
	  if (icompare(rc_type, "event") == 0)
	    {
	      event_rate = true;
	    }
	  else if (icompare(rc_type, "coverage") == 0)
	    {
	      event_rate = false;
	    }
	  else
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::parse(): syntax error in input "
			      "for rate constant type, unknown type: "
			      + *token_it);
	    }
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      // set name of counter file
      else if (icompare(*token_it, "reaction_counter") == 0)
	{
	  // next token is the name of file to output surface snapshots to
	  count_filename = *++token_it;
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      // set type of neighbor model
      else if (icompare(*token_it, "neighbor") == 0)
	{
	  // next token is the type of neighbors to allow
	  env_type = *++token_it;
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      // set type of neighbor model
      else if (icompare(*token_it, "site_type") == 0)
	{
	  // next token is type of sites to create
	  CH_STD::string site_type(*++token_it);
	  if (icompare(site_type, "radial") == 0)
	    {
	      env_radial = true;
	    }
	  else if (icompare(site_type, "neighbor") == 0)
	    {
	      env_radial = false;
	    }
	  else
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::parse(): syntax error in input "
			      "for site type, unknown type: " + *token_it);
	    }
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of integrator input
	  if (icompare(*++token_it, "integrator") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":kmc::parse(): syntax error in input "
			      "for integrator: corresponding end token does "
			      "not end an integrator: " + *token_it);
	    }
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":kmc::parse(): syntax error in input "
			  "for integrator: unrecognized token: "
			  + *token_it);
	}
    }
  // end of file reached
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "kmc::parse(): syntax error in input for integrator: "
		  "end of file reached while parsing input");
  // shouldn't get here
  return;
}

// copy this and return pointer to new object
integrator*
kmc::copy() const
  throw (bad_pointer)
{
  return new kmc(*this);
}

CH_END_NAMESPACE

/* $Id: kmc.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
