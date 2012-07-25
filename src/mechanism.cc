// Mechanism set-up and manipulating methods.
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

#include "mechanism.h"
#include <cmath>
#include <list>
#include <utility>		// make_pair()
#include "precision.h"
#include "t_string.h"

// parser includes and variables
#include <cstdio>
extern int yyparse();
extern CH_STD::FILE* yyin;
extern CH_STD::FILE* yyout;

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initialize static instance variables
// do not allow different mechanisms to have same name
unique mechanism::name_list("mechanism");

// mechanism class methods
// ctor: (default) create a unique name
mechanism::mechanism()
  throw (bad_file)
  : name(name_list.insert()), reactions(), speciesm(), parameterm()
{}

// ctor: user given name (base name of file)
mechanism::mechanism(const CH_STD::string& name_)
  throw (bad_file)
  : name(name_), reactions(), speciesm(), parameterm()
{
  // insert given name into static list
  name_list.insert(name.get_path());
}

// dtor: erase name from list, delete everything
mechanism::~mechanism()
{
  name_list.remove(name.get_path());
  // delete all reactions
  for (reaction::seq_iter it = reactions.begin(); it != reactions.end(); ++it)
    {
      delete *it;
    }
  // delete all parameters
  for (parameter_map_iter it = parameterm.begin(); it != parameterm.end(); ++it)
    {
      delete it->second;
    }
  // delete all species
  for (species::map_iter it = speciesm.begin(); it != speciesm.end(); ++it)
    {
      delete it->second;
    }
}

// mechanism private methods
// initialize the mechananism
void
mechanism::initialize()
{
  // check all reaction for species sets
  expand_species_sets();
  return;
}

// check all reactions for species sets and create needed reactions
void
mechanism::expand_species_sets()
  throw (bad_input, bad_request)
{
  // set up containers for new reactions
  CH_STD::map<reaction*,reaction::seq> new_reactions;
  // loop through all the reactions
  for (reaction::seq_iter rxn_it(reactions.begin()); rxn_it != reactions.end();
       ++rxn_it)
    {
      // get copy of the reactant sequence
      const stoich_map& reactants((*rxn_it)->get_reactants());
      // set up containers for set and non-sets
      species_set::seq reactant_sets;
      stoich_map reactant_singles;
      // set of species_sets to delete
      CH_STD::set<species_set*> species_delete;
      // loop through the reactants, determining what is what
      for (stoich_map_citer sm_it(reactants.begin()); sm_it != reactants.end();
	   ++sm_it)
	{
	  // try to cast species to a species_set
	  species_set* ssp = dynamic_cast<species_set*>(sm_it->first);
	  // see if cast was successful
	  if (ssp != 0)
	    {
	      // make sure coefficient is an integer
	      unsigned int coeff(check_coefficient(sm_it->second));
	      // put this entry into the set sequence coeff number of times
	      reactant_sets.insert(reactant_sets.end(), coeff, ssp);
	      // put it into the species to delete
	      species_delete.insert(ssp);
	    }
	  else
	    {
	      // put this entry into the single sequence
	      reactant_singles.insert(*sm_it);
	    }
	}
      // make sure there are no spectator species sets in products
      const stoich_map& products((*rxn_it)->get_products());
      for (stoich_map_citer sm_it(products.begin()); sm_it != products.end();
	   ++sm_it)
	{
	  // get the species pointer
	  species* sp(sm_it->first);
	  // try to cast species to a species_set
	  species_set* ssp = dynamic_cast<species_set*>(sp);
	  // see if cast was successful
	  if (ssp != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":mechanism::expand_species_sets(): "
			      "can not have spectator species sets as "
			      "products of a reaction - " +
			      (*rxn_it)->stringify());		      
	    }
	}
      // see if there were any species sets
      if (!reactant_sets.empty())
	{
	  // create all possible combinations of those in the sets
	  CH_STD::set<CH_STD::multiset<species*> > combos;
	  expand_species(combos, reactant_sets.begin(), reactant_sets.end());
	  // create reactions from the sets of species
	  make_reactions(*rxn_it, new_reactions, combos,
			 reactant_singles, products);
	  // delete the species sets used in this reaction
	  for (CH_STD::set<species_set*>::iterator it(species_delete.begin());
	       it != species_delete.end(); ++it)
	    {
	      delete *it;
	    }
	}
    }
  // see if we need to go through the trouble
  if (!new_reactions.empty())
    {
      // convert the sequence of reactions into a doubly-linked list
      // NOTE: need to do this so we do not invalidate the vector iterators
      CH_STD::list<reaction*> rxn_list(reactions.begin(), reactions.end());
      // clear reactions vector (will refill later)
      reactions.clear();
      // set up container for reactions to be deleted
      CH_STD::vector<CH_STD::list<reaction*>::iterator> delete_iters;
      // loop through the list
      for (CH_STD::list<reaction*>::iterator rxn_it(rxn_list.begin());
	   rxn_it != rxn_list.end(); ++rxn_it)
	{
	  // see if this reaction has replacements
	  CH_STD::map<reaction*,reaction::seq>::iterator
	    add_rxns(new_reactions.find(*rxn_it));
	  // make sure replacements were found
	  if (add_rxns != new_reactions.end())
	    {
	      // put the iterator into the list to be deleted
	      delete_iters.push_back(rxn_it);
	      // put the replacement reactions in before the wasted one
	      rxn_list.insert(rxn_it, add_rxns->second.begin(),
			      add_rxns->second.end());
	    }
	}
      // go through and delete the superfluous reactions
      for (CH_STD::vector<CH_STD::list<reaction*>::iterator>::iterator
	     d_it(delete_iters.begin()); d_it != delete_iters.end(); ++d_it)
	{
	  // delete this set containing reaction
	  delete **d_it;
	  **d_it = 0;
	  // remove this reaction from the mechanism
	  rxn_list.erase(*d_it);
	}
      // put all the reactions back into the sequence
      reactions.insert(reactions.end(), rxn_list.begin(), rxn_list.end());
    }
  return;
}

// make sure the given stoichiometric coefficient integer, check power
unsigned int
mechanism::check_coefficient(const stoichiometric& stoich)
  throw (bad_input)
{
  // make sure coefficient an integer
  double dcoeff(0.0e0);
  if (CH_STD::modf(stoich.get_coefficient(), &dcoeff)
      > precision::get().get_double())
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":mechanism::check_coefficient(): attempting to use "
		      "species sets with non-integer coefficient");
    }
  // make sure it is at least one
  if (dcoeff < 1.0e0 - precision::get().get_double())
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":mechanism::check_coefficient(): coefficient for "
		      "species set must be at least one, coeff = " +
		      t_string(dcoeff));
    }
  // make sure power and coefficient match
  double power(stoich.get_power());
  if (dcoeff < power - precision::get().get_double() ||
      dcoeff > power + precision::get().get_double())
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":mechanism::expand_species_sets(): "
		      "species stoichiometric coefficient and "
		      "power in rate expression must be the same "
		      "to use species sets");
    }
  // cast double to int (make sure truncation works right) and return
  return static_cast<int>(dcoeff + 1.0e-1);
}

// create all possible stoich maps given species_sets
void
mechanism::expand_species(CH_STD::set<CH_STD::multiset<species*> >&
			    combinations,
			  species_set::seq_citer current,
			  const species_set::seq_citer& end)
{
  // see if we even need to do anything
  if (current == end)
    {
      return;
    }
  // see if this is the last time it is called
  if (current + 1 == end)
    {
      // put species from this set into combinations
      for (species::set_citer ss_it((*current)->begin());
	   ss_it != (*current)->end(); ++ss_it)
	{
	  // make a multiset
	  CH_STD::multiset<species*> single;
	  // insert the species pointer
	  single.insert(*ss_it);
	  // insert the multiset into the set of combinations
	  combinations.insert(single);
	}
      return;
    }
  // else
  // map for sub calls to fill
  CH_STD::set<CH_STD::multiset<species*> > sub_combos;
  // recursion: call for the next set
  expand_species(sub_combos, current + 1, end);
  // loop through the sub combination vectors
  for (CH_STD::set<CH_STD::multiset<species*> >::iterator
	 sub_it(sub_combos.begin());
       sub_it != sub_combos.end(); ++sub_it)
    {
      // insert each species in this species_set
      for (species::set_citer ss_it((*current)->begin());
	   ss_it != (*current)->end(); ++ss_it)
	{
	  // create a copy of the current sub combination
	  CH_STD::multiset<species*> combo_copy(*sub_it);
	  // insert this species into it
	  combo_copy.insert(*ss_it);
	  // insert the vector into combinations
	  combinations.insert(combo_copy);
	}
    }
  return;
}

// create reactions from the sets of species
void
mechanism::make_reactions(reaction* rxn,
			  CH_STD::map<reaction*,reaction::seq>& new_rxns,
			  const CH_STD::set<CH_STD::multiset<species*> >&
			    combinations,
			  const stoich_map& single_reactants,
			  const stoich_map& products)
  throw (bad_request, bad_input)
{
  // get the rate constants for the reaction
  CH_STD::pair<k*,k*> rate_constants(rxn->get_rate_constants());
  // make sure only one of the reactions owns the rate constants
  bool own(true);
  // make sure we were called for the right reasons
  if (combinations.empty())
    {
      throw bad_request(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			":mechanism::make_reactions(): attempting to "
			"make a set of reactions without any sets of "
			"species");
    }
  else				// combinations exist
    {
      // loop through the sets of species
      for (CH_STD::set<CH_STD::multiset<species*> >::const_iterator
	     combo_it(combinations.begin());
	   combo_it != combinations.end(); ++combo_it)
	{
	  new_rxns[rxn].push_back(make_reaction(rate_constants.first,
						rate_constants.second, own,
						*combo_it, single_reactants,
						products));
	  own = false;
	}
    }
  return;
}

// create a reaction given the components, return pointer to it
reaction*
mechanism::make_reaction(k* k_forward, k* k_reverse, bool own,
			 const CH_STD::multiset<species*>& spectators,
			 const stoich_map& stoich_reactants,
			 const stoich_map& stoich_products)
  throw (bad_input)
{
  // create a reaction
  reaction* rxn = new reaction(k_forward, k_reverse, own);
  // add the non set reactants
  for (stoich_map_citer sm_it(stoich_reactants.begin());
       sm_it != stoich_reactants.end(); ++sm_it)
    {
      rxn->add_reactant(sm_it->first, sm_it->second);
    }
  // add the non set products
  for (stoich_map_citer sm_it(stoich_products.begin());
       sm_it != stoich_products.end(); ++sm_it)
    {
      rxn->add_product(sm_it->first, sm_it->second);
    }
  // add the set to both sides of the reaction
  for (CH_STD::multiset<species*>::const_iterator sp_it(spectators.begin());
       sp_it != spectators.end(); ++sp_it)
    {
      // use default coefficient
      rxn->add_reactant(*sp_it);
      rxn->add_product(*sp_it);
    }
  // return the reaction
  return rxn;
}

// mechanism public methods
// return name of mechanism
CH_STD::string
mechanism::get_name() const
{
  return name.get_path();
}

// set up parse variables and call parser (which calls lexer)
void
mechanism::parse()
  throw (bad_file, bad_input, bad_request)
{
  // make sure file exists and we can read it
  file_stat file_info(name.get_path());
  if (!file_info.is_regular() || !file_info.read_permission())
    {
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":mechanism::parse(): unable to open file "
		     + name.get_path() + " for reading: " +
		     file_info.why_no_read());
    }
  // set up parser input file
  // open file and set it up for lexing
  CH_STD::FILE* fpath = CH_STD::fopen(name.get_path().c_str(), "r");
  yyin = fpath;
  // call the parser, test return value
  if (yyparse() != 0)
    {
      // a parse error occurred
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":mechanism::parse(): a parse error occurred in file "
		      + name.get_path() + ", error should have been reported "
		      "above");
    }
  // close input file and unset yyin
  CH_STD::fclose(fpath);
  yyin = 0;
  // set up mechanism
  initialize();
  return;
}

// insert a reaction into the mechanism
void
mechanism::insert_reaction(reaction* rxn)
{
  // push reaction onto end of list
  reactions.push_back(rxn);
  return;
}

// safely insert a species into the map
species*
mechanism::insert_species(const CH_STD::string& species_name)
{
  // get an iterator to species pair
  species::map_citer sp_it(speciesm.find(species_name));
  // see if that species exists
  if (sp_it == speciesm.end())
    {
      // create a new species with that name, have the ctor check
      // surface coordination
      species* spec = new species(species_name, -1);
      // insert entry into map
      speciesm.insert(CH_STD::make_pair(species_name, spec));
      return spec;
    }
  // return pointer to species
  return sp_it->second;
}

// safely insert a parameter in to map
// default value = 0.0e0
parameter*
mechanism::insert_parameter(const CH_STD::string& parameter_name, double value)
{
  // get an iterator to parameter pair
  parameter_map_citer par_it(parameterm.find(parameter_name));
  // see if it found one
  if (par_it == parameterm.end())
    {
      // create the parameter with give name
      parameter* par = new parameter(parameter_name, value);
      // insert it into map
      parameterm.insert(CH_STD::make_pair(parameter_name, par));
      return par;
    }
  // return pointer to parameter
  return par_it->second;
}

// return pointer to species of given name, return zero if not present
species*
mechanism::get_species(const CH_STD::string& species_name)
{
  // get an iterator to species pair
  species::map_citer sp_it(speciesm.find(species_name));
  // see if that species exists
  if (sp_it == speciesm.end())
    {
      return 0;
    }
  // else return the pointer
  return sp_it->second;
}

// return pointer to species of given name, return zero if not present
parameter*
mechanism::get_parameter(const CH_STD::string& parameter_name)
{
  // get an iterator to parameter pair
  parameter_map_citer par_it(parameterm.find(parameter_name));
  // see if that parameter exists
  if (par_it == parameterm.end())
    {
      return 0;
    }
  // else return the pointer
  return par_it->second;
}

// return iterator to beginning of reaction list
reaction::seq_citer
mechanism::reaction_seq_begin() const
{
  return reactions.begin();
}

// return iterator to beginning of reaction list
reaction::seq_citer
mechanism::reaction_seq_end() const
{
  return reactions.end();
}

// return iterator to beginning of species list
species::map_citer
mechanism::species_map_begin() const
{
  return speciesm.begin();
}

// return iterator to end of species list
species::map_citer
mechanism::species_map_end() const
{
  return speciesm.end();
}

// return the size of the species map
int
mechanism::get_total_species() const
{
  return speciesm.size();
}

// return the size of the reaction sequence
int
mechanism::get_total_reactions() const
{
  return reactions.size();
}

// return a (possibly very large) string of the mechanism
CH_STD::string
mechanism::stringify() const
{
  CH_STD::string mech_string("# ");
  mech_string += name.get_path() + "\n";
  // loop through the reactions
  for (reaction::seq_citer it(reaction_seq_begin()); it != reaction_seq_end();
       ++it)
    {
      mech_string += (*it)->stringify() + "\n";
    }
#if 0 // test
  // print out species and parameters
  mech_string += "# species\n";
  for (species::map_citer it(speciesm.begin()); it != speciesm.end(); ++it)
    mech_string += it->second->get_name() + "\n";
  mech_string += "# parameters\n";
  for (parameter_map_citer it(parameterm.begin()); it != parameterm.end(); ++it)
    mech_string += it->second->get_name() + " = "
      + t_string(it->second->get_value()) + "\n";
#endif // 0 test
  return mech_string;
}

CH_END_NAMESPACE

/* $Id: mechanism.cc,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
