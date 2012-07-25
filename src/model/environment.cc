// Methods which determine the sets of surface species available for reaction.
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

#include "environment.h"
#include <algorithm>
#include <utility>
#include "compare.h"
#include "counter.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initilize static variables
environment::type environment::env_type = environment::Enn;
bool environment::radial = true;
unsigned int environment::max_sites = 0U;
rng* environment::random = 0;
model_species* environment::empty_site = 0;

// environment class methods
// ctor: set center
environment::environment(lattice_point* center_)
  : center(center_), multisite(), neighbors(), connected(), sites(),
    ensembles(), ensemble_env(), initialized(false)
{}

// dtor: delete ensemble pointers
environment::~environment()
{
  // loop through and delete pointers, this will invalidate points
  for (ensemble::seq_iter it(ensembles.begin()); it != ensembles.end(); ++it)
    {
      delete *it;
      *it = 0;
    }
}

// environment private methods
environment::type
environment::string2type(const CH_STD::string& type_string)
  throw (bad_value)
{
  // see what type they want
  if (icompare(type_string, "single") == 0)
    {
      return Esingle;
    }
  else if (icompare(type_string, "nn") == 0)
    {
      return Enn;
    }
  else if (icompare(type_string, "nnn") == 0)
    {
      return Ennn;
    }
  //else unknown type
  throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  ":environment::string2type(): trying to change "
		  "environment type to unknown type: " + type_string);
}

// convert type into string
CH_STD::string
environment::type2string(type env_type_)
  throw (bad_value)
{
  // find out type
  switch (env_type_)
    {
    case Esingle:
      return "single";
      break;

    case Enn:
      return "nn";
      break;

    case Ennn:
      return "nnn";
      break;

    default:
      // else something very wrong
      throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":environment::type2string(): given environment type "
		      "is not a recognized type");
      break;
    }
  // should not get here
  return "error";
}

// create list of sets of environments which are <= SITES neighbors away
void
environment::connectivity(group& touch, group_set& groups,
			  unsigned int n_sites)
  throw (bad_input)
{
  // see what type of sites are desired
  if (radial)			// snake out
    {
      radiate(touch, groups, n_sites);
      // add all acceptable criss-cross groups to set
      for (group_set_citer out(groups.begin()); out != groups.end(); ++out)
	{
	  // loop from the current group to the end
	  group_set_citer in(out);
	  while (++in != sites.end())
	    {
	      // create a new group which is the sum of the two given
	      group combined(out->begin(), out->end());
	      combined.insert(in->begin(), in->end());
	      // insert all combined groups whose sum envs <= max_sites
	      if (combined.size() <= max_sites)
		{
		  sites.insert(combined);
		}
	    }
	}
    }
  else				// only neighbors
    {
      // put this environment in touch
      touch.insert(this);
      // put the neighbors in touch
      touch.insert(neighbors.begin(), neighbors.end());
      // make sure max ensemble size <= the number of neighbors + 1
      if (n_sites > neighbors.size() + 1U)
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":environment::connectivity(): current site type "
			  "is neighbor, affording "
			  + t_string(neighbors.size() + 1U) + " site "
			  "ensembles, but the largest reaction in mechanism "
			  "requires a " + t_string(n_sites) + "site "
			  "ensemble");
	}
      // create sites
      neighbor_sites(groups, n_sites);
    }
  return;
}

// get all of the possible sites which radiate out from this
void
environment::radiate(group& touch, group_set& groups,
		     unsigned int n_sites)
{
  // only insert if we are allowed
  if (n_sites > 0U)
    {
      // add this site to the list of touchable sites
      touch.insert(this);
      // loop through the direct neighbors
      for (seq_citer it(neighbors.begin()); it != neighbors.end(); ++it)
	{
	  // recursion: call this environments radiate method with N_SITES - 1
	  group_set new_groups;
	  (*it)->radiate(touch, new_groups, n_sites - 1U);
	  // loop through the new groups
	  for (group_set_citer git(new_groups.begin());
	       git != new_groups.end(); ++git)
	    {
	      // copy the small group
	      group small_copy(*git);
	      // add this environment to the site
	      small_copy.insert(this);
	      // add the expanded set to groups
	      groups.insert(small_copy);
	    }
	}
      // add an entry containing just this environment
      group single;
      single.insert(this);
      groups.insert(single);
    }
  return;
}

// create sites with only neighbors
void
environment::neighbor_sites(group_set& groups, unsigned int n_sites)
{
  if (n_sites > 1U)
    {
      // create sites one smaller
      group_set small_sites;
      // recursion: create the smaller sites
      neighbor_sites(small_sites, n_sites - 1);
      // add each of the neighbors to each of the small_sites
      for (group_set_citer git(small_sites.begin()); git != small_sites.end();
	   ++git)
	{
	  // loop through all the neighbors
	  for (seq_citer it(neighbors.begin()); it != neighbors.end(); ++it)
	    {
	      // copy the small site so we can add to it
	      group small_copy(*git);
	      // add this neighboring environment
	      small_copy.insert(*it);
	      // add this to the set of groups (sites)
	      groups.insert(small_copy);
	    }
	}
    }
  // add an entry containing just this environment
  // only significant the first and last time this method is called
  group single;
  single.insert(this);
  groups.insert(single);
  return;
}

// use connect to create the ensembles containing this site
void
environment::create_ensembles()
  throw (bad_request)
{
  // clear the containers we will be filling
  ensembles.clear();
  ensemble_env.clear();
  // loop through all the sites (connected environments)
  for (group_set_citer it(sites.begin()); it != sites.end(); ++it)
    {
      unsigned int site_size(it->size());
      // do not include sites of the maximum size having an empty site in
      // in the middle
      if (!radial && center->get_species() == empty_site
	  && site_size == max_sites)
	{
	  // make sure this is the maximum possible for this env type
	  switch (env_type)
	    {
	    case Esingle:
	      // not applicable
	      break;

	    case Enn:
	      if (max_sites == 5U)
		// do not use this site
		continue;	// for (it)
	      break;

	    case Ennn:
	      if (max_sites == 9U)
		// do not use this site
		continue;	// for (it)
	      break;

	    default:
	      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
				":environment::create_ensembles(): the "
				"environment type has been corrupted");
	      break;
	    }
	}
      // create an empty sequence of model_species
      model_species::seq ms_seq;
      ms_seq.reserve(it->size());
      // multisite flag
      bool multisite(false);
      // loop through the environments in this site
      for (group_citer git(it->begin()); git != it->end(); ++git)
	{
	  // get the species and multisite information
	  CH_STD::pair<model_species*,seq>
	    species_multi((*git)->get_species());
	  // see if it is a single site species
	  if (species_multi.second.empty())
	    {
	      // insert the species into the sequence
	      ms_seq.push_back(species_multi.first);
	    }
	  else			// multisite species
	    {
	      // set flag
	      multisite = true;
	      // see if multi-site species is contained entirely in this
	      // connected set
	      bool contained(true);
	      for (seq_citer multi_it(species_multi.second.begin());
		   multi_it != species_multi.second.end(); ++multi_it)
		{
		  // try to find this env in the current group (it)
		  if (it->find(*multi_it) == it->end())
		    {
		      // not found
		      contained = false;
		      break;	// for (multi_it)
		    }
		}
	      if (contained)
		{
		  // insert the species into the sequence
		  ms_seq.push_back(species_multi.first);
		}
	      else
		{
		  // clear the species sequence
		  ms_seq.clear();
		  break;	// for (git)
		}
	    }
	  // test for failure
	  if (ms_seq.empty())
	    {
	      // failed, do not continue this for (git) loop
	      break;
	    }
	}
      // check for failure
      if (!ms_seq.empty())
	{
	  // see if there were any multisite species
	  if (multisite)
	    {
	      // make sure multisite species are inserted the proper amount
	      CH_STD::map<model_species*,counter> species_count;
	      for (model_species::seq_citer sp_it(ms_seq.begin());
		   sp_it != ms_seq.end(); ++sp_it)
		{
		  // increment the number of time this appears in the sequence
		  ++species_count[*sp_it];
		}
	      // clear the species sequence
	      ms_seq.clear();
	      // insert each species the proper number of times
	      for (CH_STD::map<model_species*,counter>::const_iterator mm_it(species_count.begin()); mm_it != species_count.end(); ++mm_it)
		{
		  int coord = mm_it->first->get_surface_coordination();
		  int count = mm_it->second.get_count();
		  for (int i(0); i < count / coord; ++i)
		    {
		      ms_seq.push_back(mm_it->first);
		    }
		}
	    }
	  // perform the insertion into the containers
	  insert_ensemble(ms_seq, seq(it->begin(), it->end()));
	}
      // else try the next site (it)
    }
  return;
}

// properly insert the given sequence of species and ennvironments those
// species are on into ensemble containers
void
environment::insert_ensemble(const model_species::seq& speciess,
			     const seq& envs)
{
  // create an ensemble from the sequence
  ensemble* surface_species = new ensemble(speciess);
  // insert it into the sequence
  ensembles.push_back(surface_species);
  // add the ensemble and environment pointers to the map
  ensemble_env.insert(CH_STD::make_pair(surface_species, envs));
  return;
}

// place a sequence of species onto sites in the sequence of envs
bool
environment::place_species(const model_species::seq& speciess, seq& envs)
  throw (bad_request, bad_pointer)
{
  // make sure there are more species left to place
  if (speciess.empty())
    {
      // make sure there are no more environments as well
      if (!envs.empty())
	{
	  throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":environment::place_species(): all species have "
			    "been placed, but there are still lattice points "
			    "available");
	}
      return true;
    }			   
  // get the species we are trying to place
  model_species* surf_species(speciess.front());
  // get the surface coordination
  unsigned int coord(surf_species->get_surface_coordination());
  // make sure it is a surface species
  if (coord < 1U)
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":environment::place_species(): species " +
			surf_species->get_name() + " is not a surface species "
			"and therefore can not be placed on surface");
    }
  // else
  // declare containers we will need to pass
  model_species::seq species_left(speciess.begin() + 1U, speciess.end());
  group_set sites_set;
  // get the possible sites we can place surf_species on
  get_sites(coord, envs, sites_set);
  // convert the set of sites into a vector
  CH_STD::vector<group> sites(sites_set.begin(), sites_set.end());
  // randomize the vector
  CH_STD::random_shuffle(sites.begin(), sites.end(), *random);
  // go through the randomized list and try to place the species
  CH_STD::vector<group>::iterator site_it(sites.begin());
  while (site_it != sites.end())
    {
      // create new lists without the used sites
      seq envs_left;
      // create envs_left
      for (seq_citer orig_it(envs.begin()); orig_it != envs.end(); ++orig_it)
	{
	  // see if this environment is in the site we are using
	  group_citer found_env(site_it->find(*orig_it));
	  if (found_env == site_it->end())
	    {
	      // not found in the current site
	      envs_left.push_back(*orig_it);
	    }
	}
      // try to place the rest of the species on the rest of the sites
      if (place_species(species_left, envs_left))
	{
	  // put species on the sites
	  for (group_iter git(site_it->begin()); git != site_it->end(); ++git)
	    {
	      // create a sequence of envs
	      seq multi;
	      for (group_iter it(site_it->begin()); it != site_it->end(); ++it)
		{
		  if (it != git)
		    {
		      multi.push_back(*it);
		    }
		}
	      // place the species on the current environment
	      (*git)->set_species(surf_species, multi);
	    }
	  // success
	  return true;
	}
      // else try the next one
      ++site_it;
    }
  // could not place it given the previous placements
  return false;
#if 0				// do not need this
  // determine the maximum coordination of the species to be placed
  int max_coord(0);
  for (model_species::seq_citer it(speciess.begin()); it != speciess.end();
       ++it)
    {
      int coord((*it)->get_surface_coordination());
      // make sure it is a surface species
      if (coord < 1)
	{
	  throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			    ":environment::place_species(): species " +
			    (*it)->get_name() + " is not a surface species and "
			    "therefore can not be placed on surface");
	}
      max_coord = (coord > max_coord) ? coord : max_coord;
    }
  // split up to make placing of single site species fast
  if (max_coord > 1)		// multiply coordinated
    {
      // get first for each sequence
      model_species::seq_citer species_it(speciess.begin());
      seq_citer env_it(envs.begin());
      // try to place them
      while (species_it != speciess.end())
	{
	  int coord((*species_it)->get_surface_coordination());
	  seq multi(1, *env_it);
	  for (int i(1); i < coord; ++i)
	    {
	      // try to assemble a connecting site
	      while (++env_it != envs.begin())
		{
		  if (multi.back()->is_neighbor(*++env_it))
		    {
		      multi.push_back(*env_it);
		      break;	// while
		    }
		}
	      // see if placement succeeded
	      if (env_it == envs.end())
		{
		  // back up and try again
		}
	    }
	  // put the species on the sites
	  for (seq_citer it(multi.begin()); it != multi.end(); ++it)
	    {
	      (*it)->set_species(*species_it, multi);
	    }
	  // move to next species
	  ++species_it;
	}      
    }
  else				// all single coordination
    {
      // get the first environment involved in the reaction
      seq_citer current_env(envs.begin());
      // loop through the products and place them
      for (model_species::seq_citer it(speciess.begin()); it != speciess.end();
	   ++it)
	{
	  // place species
	  (*current_env)->set_species(*it);
	  // set next environment to the current
	  ++current_env;
	}
    }
  return;
#endif // 0
}

// put all possible connected sites in sites
void
environment::get_sites(int coord, const seq& envs, group_set& sites)
{
  if (coord == 1)
    {
      for (seq_citer it(envs.begin()); it != envs.end(); ++it)
	{
	  group single;
	  single.insert(*it);
	  sites.insert(single);
	}
      return;
    }
  // else
  group_set small;
  // recursion: get all sites of size (coord - 1)
  get_sites(coord - 1, envs, small);
  // loop through environments
  for (seq_citer eit(envs.begin()); eit != envs.end(); ++eit)
    {
      // try to add this environment to each small set of ens
      for (group_set_iter sit(small.begin()); sit != small.end(); ++sit)
	{
	  // make sure this env is not already in this group (``site'')
	  if (sit->find(*eit) == sit->end())
	    {
	      // loop through small site, determine if this site is connected
	      for (group_iter git(sit->begin()); git != sit->end(); ++git)
		{
		  // is the environment in question a neighbor?
		  if ((*git)->is_neighbor(*eit))
		    {
		      // create a new group from the smaller site
		      group site(sit->begin(), sit->end());
		      // add this environment to it
		      site.insert(*eit);
		      // insert the site into the list
		      // could be identical to a group already in the set
		      sites.insert(site);
		      // already insert so don't keep trying
		      break;	// for (git)
		    }
		}
	    }
	}
    }
  return;
}

// return whether the given environment is a neighbor of this one
bool
environment::is_neighbor(const environment* env)
{
  for (seq_citer it(neighbors.begin()); it != neighbors.end(); ++it)
    {
      if (*it == env)
	{
	  return true;
	}
    }
  return false;
}

// return the type of species on this site
CH_STD::pair<model_species*,environment::seq>
environment::get_species() const
{
  return CH_STD::make_pair(center->get_species(), multisite);
}

// change the species which occupies the center site, return old species
model_species*
environment::set_species(model_species* center_species)
  throw (bad_pointer)
{
  if (!initialized)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":environment::set_species(): current environment "
			"has not been set yet, so you can not change species");
    }
  // erase record of multiple site species
  multisite.clear();
  // change the species
  model_species* old(center->set_species(center_species));
  // return old species
  return old;
}

// place a multi-site species on the center site, return old species
model_species*
environment::set_species(model_species* center_species,
			 const seq& multisite_)
  throw (bad_pointer)
{
  model_species* old(set_species(center_species));
  // set what other sites are involved
  multisite = multisite_;
  return old;
}

// environment public methods
// set the type of environment to use, return old type
CH_STD::string
environment::set_environment_type(const CH_STD::string& env_string)
  throw (bad_value)
{
  // convert current type into a string
  CH_STD::string old(type2string(env_type));
  // change the type
  env_type = string2type(env_string);
  return old;
}

// change the type of sites allow, return old value
bool
environment::site_type_radial(bool radial_)
{
  bool old(radial);
  radial = radial_;
  return old;
}

// set the maximum number of sites required for any reaction in the mechanism
// return old value
unsigned int
environment::set_max_sites(unsigned int max_sites_)
{
  int old(max_sites);
  max_sites = max_sites_;
  return old;
}

// set the random number generator (must be done before initialization)
void
environment::set_rng(rng* random_)
{
  random = random_;
  return;
}

// set the empty site species
void
environment:: set_empty_site(model_species* empty_site_)
{
  empty_site = empty_site_;
  return;
}

// set the neighbors of this environment
// note: must be done to entire surface before initialize() is called
void
environment::set_neighbors(const matrix& surface)
  throw (bad_value)
{
  if (env_type == Esingle)
    {
      // nothing to do for a single
      return;
    }
  // else
  // get where we are
  CH_STD::pair<unsigned int,unsigned int> rc(center->get_position());
  int row(rc.first);
  int column(rc.second);
  // set up indices of the surrounding envionments
  int up(row - 1);
  int down(row + 1);
  int left(column - 1);
  int right(column + 1);
  // get the information on the size of the surface
  int rows(surface.size());
  // in case it is not square
  int columns(surface.begin()->size());
  // adjust desired position according to periodicity
  up = (up < 0) ? rows - 1 : up;
  down = (down >= rows) ? 0 : down;
  left = (left < 0) ? columns - 1 : left;
  right = (right >= columns) ? 0 : right;
  // set the nearest neighbors
  neighbors.push_back(surface[up][column]);
  neighbors.push_back(surface[down][column]);
  neighbors.push_back(surface[row][right]);
  neighbors.push_back(surface[row][left]);
  if (env_type == Enn)
    {
      return;
    }
  // set the next-nearest neighbors
  neighbors.push_back(surface[up][left]);
  neighbors.push_back(surface[up][right]);
  neighbors.push_back(surface[down][left]);
  neighbors.push_back(surface[down][right]);
  if (env_type == Ennn)
    {
      return;
    }
  // else unknown environment type
  throw bad_value(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  ":environment::set_neighbors(): current environment "
		  "has been corrupted and is not a recognized type");
  return;
}

// set up environment pointers using the given matrix of environments
void
environment::initialize()
  throw (bad_pointer, bad_input, bad_request)
{
  // make sure random number generator has been set
  if (random == 0)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":environment::initialize(): random number generator "
			"has not been set, cannot initialize");
    }
  // clear out the sites container
  sites.clear();
  // get the environments are <= max_sites away
  connectivity(connected, sites, max_sites);
  // create all ensembles which include this sight
  create_ensembles();
  initialized = true;
  return;
}

// exchange an ensemble with a new one, update everything, put ensembles to
// delete in remove, affected environments in changed
void
environment::change_ensemble(ensemble* reactants,
			     const model_species::seq& products,
			     ensemble::deq& remove, group& changed)
  throw (bad_pointer, bad_request)
{
  if (!initialized)
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":environment::change_ensemble(): current environment "
			"has not been set yet, so you can not change an "
			"ensemble");
    }
  // make sure the old ensemble is owned by this environment
  map_iter old_ens_env(ensemble_env.find(reactants));
  if (old_ens_env == ensemble_env.end())
    {
      throw bad_pointer(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":environment::change_ensemble(): could not find the "
			"given old pointer in this environment");
    }
  // get a set of all the affected environments
  // add this point
  changed.insert(this);		// should be in connected as well
  // add the environments connected to this point
  changed.insert(connected.begin(), connected.end());
  // loop through the environments involved in reaction
  for (seq_citer it(old_ens_env->second.begin());
       it != old_ens_env->second.end(); ++it)
    {
      // insert environments connected to them as well
      changed.insert((*it)->connected.begin(), (*it)->connected.end());
    }
  // get all (ugh) of the ensembles for the pertinent environments
  for (group_citer it(changed.begin()); it != changed.end(); ++it)
    {
      // insert faster for deques
      remove.insert(remove.end(), (*it)->ensembles.begin(),
		    (*it)->ensembles.end());
    }
  // make sure the size the the old ensemble and list of products is the name
  // create an ensemble from products
  ensemble prods(products);
  if (reactants->get_coordination() != prods.get_coordination())
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":environment::change_ensemble(): total coordination "
			"of surface species in reactants (" +
			t_string(reactants->get_coordination()) + ") does not "
			"match that in the products (" +
			t_string(prods.get_coordination()) + ")");
			
    }
  // use the ensemble to get a list of the surface species
  model_species::seq surface_products(prods.begin(), prods.end());
  // randomize the list of surface species
  CH_STD::random_shuffle(surface_products.begin(), surface_products.end(),
			 *random);
  // change the species on the environments involved in reaction
  place_species(surface_products, old_ens_env->second);
  // re-create the ensembles for the affected environments
  for (group_citer it(changed.begin()); it != changed.end(); ++it)
    {
      (*it)->create_ensembles();
    }
  return;
}

// return an iterator to the beginning of sites
environment::group_set_citer
environment::sites_begin() const
{
  return sites.begin();
}

// return an iterator to the end of sites
environment::group_set_citer
environment::sites_end() const
{
  return sites.end();
}

// return an iterator to the beginning sequence of ensembles
ensemble::seq_citer
environment::ensembles_seq_begin() const
{
  return ensembles.begin();
}

// return an iterator to the end sequence of ensembles
ensemble::seq_citer
environment::ensembles_seq_end() const
{
  return ensembles.end();
}

CH_END_NAMESPACE

/* $Id: environment.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
