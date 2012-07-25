// Methods for creating, inspecting, and manipulating chemical species.
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

#include "species.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// initialize static variables
unique species::name_list("sp", true);
unique species_class::name_list("cl", true);

// species methods
// ctor: copy - protected
species::species(const species& original)
  : name(original.name), surface_coord(original.surface_coord)
{
  name_list.insert(name);
}

// ctor: (default) create unique name, optional surface coordination
// defaults surface_coord_ = 0U
species::species(unsigned int surface_coord_)
  : name(name_list.insert()), surface_coord(surface_coord_)
{}

// ctor: create species with given name, optional surface
// ctor: coordination, if surface_coord_ < 0, parse the string and
// ctor: determine coordination (number of `@')
// ctor: defaults surface_coord_ = 0
species::species(const CH_STD::string& name_, int surface_coord_)
  : name(name_), surface_coord(0U)
{
  name_list.insert(name);
  if (surface_coord_ < 0)
    {
      surface_coord = calc_surface_coordination();
    }
  else
    {
      surface_coord = (unsigned int) surface_coord_;
    }
}

// dtor: delete name from list of used names
species::~species()
{
  name_list.remove(name);
}

// species private methods
// calculate the surface coordination by parsing NAME
unsigned int
species::calc_surface_coordination() const
{
  unsigned int sc(0U);
  CH_STD::string::const_iterator c(name.begin());
  while (c != name.end() && *c == '@')
    {
      ++sc;
      ++c;
    }
  return sc;
}

// species public methods
// return NAME of species
CH_STD::string
species::get_name() const
{
  return name;
}

// return SURFACE_COORD of species
unsigned int
species::get_surface_coordination() const
{
  return surface_coord;
}

// return string representation of species
CH_STD::string
species::stringify() const
{
  return get_name();
}

// species_set methods
// ctor: copy the given set into this set
species_set::species_set(const species::set& speciess_)
  throw (bad_input)
  : species(), speciess(speciess_)
{
  // determine surface coordination of each species
  species::set_citer sp_it(speciess.begin());
  unsigned int coord((*sp_it)->get_surface_coordination());
  while (++sp_it != speciess.end())
    {
      // make sure coordination agrees
      if ((*sp_it)->get_surface_coordination() != coord)
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":species_set::species_set(): coordination of "
			  "species " + (*sp_it)->get_name() + " is not equal "
			  "to coordination of other species in the set");
	}
    }
}

// dtor: do nothing
species_set::~species_set()
{}

// return iterator to beginning of set
species::set_citer
species_set::begin() const
{
  return speciess.begin();
}

// return iterator to end of set
species::set_citer
species_set::end() const
{
  return speciess.end();
}

// model_species methods
// ctor: create from a species
model_species::model_species(const species& original)
  : species(original), amount(0), derivative(0.0e0)
{
  // create a quantity
  if (get_surface_coordination())
    {
      amount = new surface_quantity();
    }
  else
    {
      amount = new fluid_quantity();
    }
}

// dtor: delete quantity
model_species::~model_species()
{
  // delete quantity (no leaks)
  delete amount;
  amount = 0;
}

// model_species public methods
// return current specified quantity of species
// default type = Econcentration
double
model_species::get_quantity(quantity::type type) const
  throw(bad_type) // quantity::get_quantity()
{
  return amount->get_quantity(type);
}

// return current specified quantity of species
double
model_species::get_quantity(const CH_STD::string& type) const
  throw(bad_type) // quantity::get_quantity()
{
  return amount->get_quantity(type);
}

// return current DERIVATIVE wrt state variable
double
model_species::get_derivative() const
{
  return derivative;
}

// zero all the quantity values
void
model_species::zero_quantity()
{
  amount->zero_quantity();
}

// replace current quantity with that given
void
model_species::set_quantity(quantity* amount_)
{
  // delete current quantity
  delete amount;
  // set the given one
  amount = amount_;
  return;
}

// set the specified quantity of the model species, return old value
// default type = Econcentration, amount_ = 0.0e0
double
model_species::set_quantity(quantity::type type, double amount_)
  throw(bad_type, bad_value) // quantity::set_quantity()
{
  return amount->set_quantity(type, amount_);
}

// set the specified quantity of the model species, return old value
// default amount_ = 0.0e0
double
model_species::set_quantity(const CH_STD::string& type, double amount_)
  throw(bad_type, bad_value) // quantity::set_quantity()
{
  return amount->set_quantity(type, amount_);
}

// increment quantity by given amount, return new amount
double
model_species::add_to_quantity(quantity::type type, double increment)
  throw(bad_type, bad_value)	// quantity::set_quantity()
{
  return amount->add_to_quantity(type, increment);
}

// set the DERIVATIVE of the species, return the old value
// defaults derivative_ = 0.0e0
double
model_species::set_derivative(double derivative_)
{
  double old(derivative);
  derivative = derivative_;
  return old;
}

// add INCREMENT to derivative, return DERIVATIVE after increment
double
model_species::add_to_derivative(double increment)
{
  return derivative += increment;
}

// species_class methods
// ctor: (default) create unique name of class with zero members
// ctor: default type = quantity::Econcentration
species_class::species_class(quantity::type type)
  : name(name_list.insert()), amount_type(type)
{}

// ctor: name provided
// ctor: default type = quantity::Econcentration
species_class::species_class(const CH_STD::string& name_, quantity::type type)
  : name(name_), amount_type(type)
{
  name_list.insert(name);
}

// dtor: delete name from list of used names
species_class::~species_class()
{
  name_list.remove(name);
}

// species_class public methods
// return name of the species class
CH_STD::string
species_class::get_name() const
{
  return name;
}

// calculate and return summed quantity of species in class
double
species_class::get_quantity() const
  throw (bad_type)
{
  double sum(0.0e0);
  // add up the quantities of all the members of the class
  for (const_iterator it = begin(); it != end(); it++)
    {
      sum += (*it)->get_quantity(amount_type);
    }
  return sum;
}

// calculate and return summed quantity of species in class
double
species_class::get_derivative() const
{
  double sum(0.0e0);
  // add up the derivatives of all the members of the class
  for (const_iterator it = begin(); it != end(); it++)
    {
      sum += (*it)->get_derivative();
    }
  return sum;
}

CH_END_NAMESPACE

/* $Id: species.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
