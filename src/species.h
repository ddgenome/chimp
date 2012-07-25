// -*- C++ -*-
// Class declarations for chemical species.
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
#ifndef CH_SPECIES_H
#define CH_SPECIES_H 1

#include <map>
#include <set>
#include <string>
#include <vector>
#include "except.h"
#include "quantity.h"
#include "unique.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// molecules (species)
class species
{
public:
  // set up containers and iterators for species
  typedef CH_STD::vector<species*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;
  // a map of names and species pointers to aid in determining uniqueness
  typedef CH_STD::map<CH_STD::string,species*> map;
  typedef map::iterator map_iter;
  typedef map::const_iterator map_citer;
  typedef CH_STD::set<species*> set;
  typedef set::iterator set_iter;
  typedef set::const_iterator set_citer;

private:
  CH_STD::string name;		// name for the species
  unsigned int surface_coord;	// how many catalytic sites species occupies
  static unique name_list;	// keep track of all species names

private:
  // prevent assignment
  species& operator=(const species&);
  // calculate the surface coordination by parsing NAME
  unsigned int calc_surface_coordination() const;
protected:
  // ctor: copy
  explicit species(const species& original);
public:
  // ctor: (default) create a unique name, optional surface coordination
  explicit species(unsigned int surface_coord_ = 0U);
  // ctor: with name, optional surface coordination, if surface_coord_ < 0,
  // ctor: parse the string and determine coordination (number of `@')
  explicit species(const CH_STD::string& name_, int surface_coord_ = 0);
  // dtor: delete name from list of used names
  virtual ~species();

  // return NAME of species
  CH_STD::string get_name() const;
  // return SURFACE_COORD of species
  unsigned int get_surface_coordination() const;
  // return string representation of species
  CH_STD::string stringify() const;
}; // end class species

// class for a set of spectator species in a reaction
class species_set : public species
{
public:
  // typedefs
  typedef CH_STD::vector<species_set*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;

  species::set speciess;	// set of species

private:
  // prevent copy construction and assignment
  species_set(const species_set&);
  species_set& operator=(const species_set&);
public:
  // ctor: set of species is argument
  species_set(const species::set& speciess_)
    throw (bad_input);		// this
  // dtor: do nothing (do not own the species pointers)
  virtual ~species_set();

  // return iterator to beginning of set
  species::set_citer begin() const;
  // return iterator to end of set
  species::set_citer end() const;
}; // end class species_set

// species used for model solution class
class model_species : public species
{
public:
  // sequence of model_species
  typedef CH_STD::vector<model_species*> seq;
  typedef seq::iterator seq_iter;
  typedef seq::const_iterator seq_citer;

private:
  quantity* amount;		// amount of species
  double derivative;		// derivative wrt state variable

private:
  // prevent copy construction and assignment
  model_species(const model_species&);
  model_species& operator=(const model_species&);
public:
  // ctor: create from species
  explicit model_species(const species& original);
  // dtor: delete quantity* amount
  virtual ~model_species();

  // return current specified quantity of species
  double get_quantity(quantity::type type = quantity::Econcentration) const
    throw (bad_type); // quantity::get_quantity()
  double get_quantity(const CH_STD::string& type) const
    throw (bad_type); // quantity::get_quantity()
  // return DERIVATIVE of species
  double get_derivative() const;
  // zero all quantities
  void zero_quantity();
  // replace current quantity with the one given
  void set_quantity(quantity* amount_);
  // set the specified quantity of the model species, return old value
  double set_quantity(quantity::type type = quantity::Econcentration,
		      double amount_ = 0.0e0)
    throw (bad_type, bad_value); // quantity::set_quantity()
  double set_quantity(const CH_STD::string& type, double amount_ = 0.0e0)
    throw (bad_type, bad_value); // quantity::set_quantity()
  // increment quantity by given amount, return new amount
  double add_to_quantity(quantity::type type = quantity::Econcentration,
			 double increment = 0.0e0)
    throw (bad_type, bad_value); // quantity::add_to_quantity()
  // set species derivative, return old DERIVATIVE
  double set_derivative(double derivative_ = 0.0e0);
  // add INCREMENT to derivative, return DERIVATIVE after increment
  double add_to_derivative(double increment);
}; // end class model_species

// mapping of species to model_species
typedef CH_STD::map<species*,model_species*> species2model;
typedef species2model::iterator species2model_iter;
typedef species2model::const_iterator species2model_citer;

// species used for network generation
class mechanism_species : public species
{
  CH_STD::string formula;	// empirical formula
  CH_STD::string string_code;	// string code representation of molecule
  bool reactive;		// states if the species is allowed to react
  bool reacted;			// states if the species has been reacted

private:
  // prevent copy construction and assignment
  mechanism_species(const mechanism_species&);
  mechanism_species& operator=(const mechanism_species&);
public:
  // ctor: create from species
  explicit mechanism_species(const species& original);
  // dtor: do nothing
  virtual ~mechanism_species();

  // is the species allowed to react? return true if it is
  bool is_reactive();
  // has the species been reacted? return true if it is
  bool is_reacted();
}; // end class mechanism_species

// lumping class class (?!?)
// inherit from STL to keep consistent interface
class species_class : public CH_STD::vector<model_species*>
{
  CH_STD::string name;		// species class name
  quantity::type amount_type;	// which type of quantity to lump
  static unique name_list;	// make sure names are unique

private:
  // prevent copy construction and assignment
  species_class(const species_class&);
  species_class& operator=(const species_class&);
public:
  // ctor: (default) create unique name and set type
  explicit species_class(quantity::type type = quantity::Econcentration);
  // ctor: with name provided
  explicit species_class(const CH_STD::string& name_,
			 quantity::type type = quantity::Econcentration);
  // dtor: delete name from list of used names
  ~species_class();

  // return name of species class
  CH_STD::string get_name() const;
  // return current sum quantity in class lump
  double get_quantity() const
    throw (bad_type); // quantity::get_quantity()
  // return current sum derivative in class lump
  double get_derivative() const;
}; // end class species_class

CH_END_NAMESPACE

#endif // not CH_SPECIES_H

/* $Id: species.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
