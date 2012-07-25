// -*- C++ -*-
// Mechanism parsing and managin classes.
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
#ifndef CH_MECHANISM_H
#define CH_MECHANISM_H 1

#include <map>
#include <set>
#include <string>
#include "except.h"
#include "file.h"
#include "parameter.h"
#include "reaction.h"
#include "species.h"
#include "unique.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// mechanism managing class
class mechanism
{
public:
  // set up typedef's
  typedef CH_STD::vector<mechanism*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;

private:
  file_name name;		// name information for input file
  reaction::seq reactions;	// list of reactions
  species::map speciesm;	// list of species names and pointers
  parameter_map parameterm;	// map of parameter names and pointers
  static unique name_list;	// keep track of all current mechanism names

private:
  // prevent copy construction and assignment
  mechanism(const mechanism&);
  mechanism& operator=(const mechanism&);
  // initialize the mechanism after parsing
  void initialize();
  // check all reactions for species sets and create needed reactions
  void expand_species_sets()
    throw (bad_input, bad_request); // this, check_coefficient(),
				//  make_reactions()
  static unsigned int check_coefficient(const stoichiometric& stoich)
    throw (bad_input);		// this
  // create all possible stoich maps given species_sets
  void expand_species(CH_STD::set<CH_STD::multiset<species*> >& combinations,
		      species_set::seq_citer current,
		      const species_set::seq_citer& end);
  // create reactions from the sets of species
  static void make_reactions(reaction* rxn,
			     CH_STD::map<reaction*,reaction::seq>& new_rxns,
			     const CH_STD::set<CH_STD::multiset<species*> >&
			       combinations,
			     const stoich_map& single_reactants,
			     const stoich_map& products)
    throw (bad_request, bad_input); // this, make_reaction()
  // create a reaction given the components, return pointer to it
  static reaction* make_reaction(k* k_forward, k* k_reverse, bool own,
				 const CH_STD::multiset<species*>& spectators,
				 const stoich_map& stoich_reactants,
				 const stoich_map& stoich_products)
    throw (bad_input);		// reaction::add_reactant(),
				// reaction::add_product()
public:
  // ctor: (default) create unique name
  mechanism()
    throw (bad_file); // file_name()
  // ctor: user given name (name of file)
  explicit mechanism(const CH_STD::string& name_)
    throw (bad_file); // file_name()
  // dtor: erase name from list, delete everything
  ~mechanism();

  // return name of mechanism
  CH_STD::string get_name() const;
  // set up and parse the mechanism input file
  void parse()
    throw (bad_file, bad_input, bad_request); // this, file_stat,
				//  expand_species_sets()
  // insert a reaction into the mechanism
  void insert_reaction(reaction* rxn);
  // safely insert a species into map, return pointer to species
  species* insert_species(const CH_STD::string& species_name);
  // safely insert a parameter into map, return pointer to parameter
  // value is only used if a new parameter is created
  parameter* insert_parameter(const CH_STD::string& parameter_name,
			      double value = 0.0e0);
  // return pointer to species of given name, zero if not present
  species* get_species(const CH_STD::string& species_name);
  // return pointer to parameter of given name, zero if not present
  parameter* get_parameter(const CH_STD::string& parameter_name);
  // return iterator to beginning of reaction list
  reaction::seq_citer reaction_seq_begin() const;
  // return iterator to beginning of reaction list
  reaction::seq_citer reaction_seq_end() const;
  // return iterator to beginning of species map
  species::map_citer species_map_begin() const;
  // return iterator to beginning of species map
  species::map_citer species_map_end() const;
  // return the size of the species map
  int get_total_species() const;
  // return the size of the reaction sequence
  int get_total_reactions() const;
  // return a (possibly very large) string of the mehanism
  CH_STD::string stringify() const;
}; // end class mechanism

CH_END_NAMESPACE

#endif // not CH_MECHANISM_H

/* $Id: mechanism.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
