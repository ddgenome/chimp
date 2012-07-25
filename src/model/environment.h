// -*- C++ -*-
// Class of information about which species surround a surface species.
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
#ifndef CH_MODEL_ENVIRONMENT_H
#define CH_MODEL_ENVIRONMENT_H 1

#include <deque>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "ensemble.h"
#include "except.h"
#include "lattice.h"
#include "point.h"
#include "rng.h"
#include "species.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// abstract base class for surface environment information
class environment
{
public:
  // set up typedef's
  typedef CH_STD::vector<environment*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;
  typedef CH_STD::vector<seq> matrix;
  typedef matrix::iterator matrix_iter;
  typedef matrix::const_iterator matrix_citer;
  typedef CH_STD::set<environment*> group;
  typedef group::iterator group_iter;
  typedef group::const_iterator group_citer;
  typedef CH_STD::deque<group> group_deq;
  typedef group_deq::iterator group_deq_iter;
  typedef group_deq::const_iterator group_deq_citer;
  typedef CH_STD::set<group> group_set;
  typedef group_set::iterator group_set_iter;
  typedef group_set::const_iterator group_set_citer;
  typedef CH_STD::map<ensemble*,seq> map;
  typedef map::iterator map_iter;
  typedef map::const_iterator map_citer;

private:
  // enumeration for the types of environments (refer to as environment::Efoo)
  enum type { Esingle, Enn, Ennn };
  lattice_point* center;	// pointer to whose environment this is
  seq multisite;		// if species is on multiple sites, those envs
  seq neighbors;		// neighboring environments
  group connected;		// given max_sites, what envnmnts touch this
  group_set sites;		// sites containing this environment
  ensemble::seq ensembles;	// ensembles available around this point
  map ensemble_env;		// environments in each ensemble
  bool initialized;		// whether neighbors have been set
  static type env_type;		// the type of environment to use
  static bool radial;		// what types of sites to allow
  static unsigned int max_sites; // maximum number of sites needed for reaction
  static rng* random;		// random number generator
  static model_species* empty_site; // empty site species

private:
  // prevent copy construction and assignment
  environment(const environment&);
  environment& operator=(const environment&);
  // convert string into environment type
  static type string2type(const CH_STD::string& type_string)
    throw (bad_value); // this
  // convert type into string
  static CH_STD::string type2string(type env_type_)
    throw (bad_value); // this
  // create list of sets of environments which are <= SITES neighbors away
  void connectivity(group& touch, group_set& groups, unsigned int n_sites)
    throw (bad_input);		// this
  // create all sites which radiate from this site, put int groups
  void radiate(group& touch, group_set& groups, unsigned int n_sites);
  // create sites with only neighbors
  void neighbor_sites(group_set& groups, unsigned int n_sites);
  // use connect to create the ensembles containing this site
  void create_ensembles()
    throw (bad_request);	// this
  // properly insert the given sequence of species and ennvironments those
  // species are on into ensemble containers
  void insert_ensemble(const model_species::seq& speciess, const seq& envs);
  // place a sequence of species onto sites in the sequence of envs
  bool place_species(const model_species::seq& speciess, seq& envs)
    throw (bad_request, bad_pointer); // this, set_species()
  // put all possible connected sites in sites
  void get_sites(int coord, const seq& envs, group_set& sites);
  // return whether the given environment is a neighbor of this one
  bool is_neighbor(const environment* env);
  // return the type of species on this site and any sites which share species
  CH_STD::pair<model_species*,seq> get_species() const;
  // change the species which occupies the center site, return old species
  model_species* set_species(model_species* center_species)
    throw (bad_pointer); // this
  // place a multi-site species on the center site, return old species
  model_species* set_species(model_species* center_species,
			     const seq& multisite_)
    throw (bad_pointer); // set_species()
public:
  // ctor: set center
  explicit environment(lattice_point* center_);
  // dtor: delete ensemble pointers
  ~environment();

  // set the type of environment to use, return old type
  static CH_STD::string set_environment_type(const CH_STD::string& env_string)
    throw (bad_value); // type2string(), string2type()
  // change the type of sites allow, return old value
  static bool site_type_radial(bool radial_);
  // set the maximum number of sites required for any reaction in the mechanism
  // return old value
  static unsigned int set_max_sites(unsigned int max_sites_);
  // set the random number generator (must be done before initialization
  static void set_rng(rng* random_);
  // set the empty site species
  static void set_empty_site(model_species* empty_site_);
  // set the neighbors of this environment
  void set_neighbors(const matrix& surface)
    throw (bad_value); // this
  // set up environment pointers using the given matrix of environments
  void initialize()
    throw(bad_pointer, bad_input, bad_request); // this, connectivity(),
				// create_ensembles()
  // exchange an ensemble with a new one, update everything, put affected
  // environments in changed
  void change_ensemble(ensemble* reactants, const model_species::seq& products,
		       ensemble::deq& remove, group& changed)
    throw (bad_pointer, bad_request); // this, place_species(),
				// create_ensembles()
  // return an iterator to the beginning of sites
  group_set_citer sites_begin() const;
  // return an iterator to the end of sites
  group_set_citer sites_end() const;
  // return an iterator to the beginning sequence of ensembles
  ensemble::seq_citer ensembles_seq_begin() const;
  // return an iterator to the end sequence of ensembles
  ensemble::seq_citer ensembles_seq_end() const;
}; // end class environment

CH_END_NAMESPACE

#endif // not CH_MODEL_ENVIRONMENT_H

/* $Id: environment.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
