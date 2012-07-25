// Functions to generate random numbers.
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
#endif // HAVE_CONFIG_H

#include "rng.h"
#include "compare.h"
#include "t_string.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// static instance variables
const int rng_mt::period = 397; // a period parameter
const ul_int rng_mt::magic = 0x9908b0dfUL; // a magic constant

// rng methods
// ctor: intialize seed and name
// ctor: default seed_ = 1
rng::rng(const CH_STD::string& name_, ul_int seed_)
  : name(name_), seed(seed_)
{}

// ctor: copy (protected)
rng::rng(const rng& original)
  : name(original.name), seed(original.seed)
{}

// dtor: do nothing
rng::~rng()
{}

// rng public methods
// create a new random number generator of the given type, return
// pointer or zero if type is invalid
// default type = "mt"
rng*
rng::new_rng(const CH_STD::string& type)
{
  // see what type of random number generator they want
  if (icompare(type, "rand") == 0)
    {
      return new rng_rand();
    }
  else if (icompare(type, "mt") == 0)
    {
      return new rng_mt();
    }
  // else				// unknown rng
  return 0;
}

// parse rng input
void
rng::parse(token_seq_citer& token_it, token_seq_citer end)
  throw (bad_input)
{
  // loop through input
  while (token_it != end)
    {
      if (icompare(*token_it, "seed") == 0)
	{
	  // set the rng seed using next token
	  set_seed(CH_STD::strtoul((++token_it)->c_str(), (char **)0, 0));
	  // increment token
	  ++token_it;
	  continue;		// while ()
	}
      else if (icompare(*token_it, "end") == 0)
	{
	  // make sure it is the end of rng input
	  if (icompare(*++token_it, "rng") != 0)
	    {
	      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			      ":rng::parse(): syntax error in input "
			      "for random number generator: corresponding end "
			      "token does not end an rng: " + *token_it);
	    }
	  // increment one further
	  ++token_it;
	  // return to caller
	  return;
	}
      else			// unknown token
	{
	  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
			  ":rng::parse(): syntax error in input "
			  "for integrator: unrecognized token: "
			  + *token_it);
	}
    }
  // end of input reached in mid rng input
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "rng::parse(): syntax error in input for rng: "
		  "end of file reached while parsing input");
  // shouldn't get here
  return;
}

// return the value of the seed
ul_int
rng::get_seed() const
{
  return seed;
}

// return the name of the current random number generator
CH_STD::string
rng::get_name() const
{
  return name;
}

// change the seed of the random number generator
ul_int
rng::set_seed(ul_int seed_)
{
  ul_int old(seed);
  seed = seed_;
  return old;
}

// rng_rand methods
// ctor: seed the rng with optional seed
// ctor: default seed_ = 1
rng_rand::rng_rand(ul_int seed_)
  : rng("rand", seed_)
{
  srand(get_seed());
}

// ctor: copy
rng_rand::rng_rand(const rng_rand& original)
  : rng(original)
{}				// don't call srand because seed is global

// dtor: do nothing
rng_rand::~rng_rand()
{}

// rng_rand public methods
// copy current rng_rand, return pointer to it
rng*
rng_rand::copy()
{
  return new rng_rand(*this);
}

// change the seed, return old
ul_int
rng_rand::set_seed(ul_int seed_)
{
  srand(seed_);
  return rng::set_seed(seed_);
}

// rng_mt class methods
// ctor: (default) optional seed (should be odd in the range [0-2^32))
// ctor: default seed_ = 4357UL
rng_mt::rng_mt(ul_int seed_)
  : rng("mt", seed_)
{
  seed_state();
}

// ctor: copy
rng_mt::rng_mt(const rng_mt& original)
  : rng(original)
{
  seed_state();
}

// dtor: do nothing
rng_mt::~rng_mt()
{}

// rng_mt private methods
// initialize the twister
void
rng_mt::seed_state()
{
  register ul_int x = (get_seed() | 1UL) & 0xffffffffUL;
  register ul_int* s = state;
  register int j;

  for(left = 0, *s++ = x, j = length; --j;
      *s++ = (x *= 69069UL) & 0xffffffffUL);
}

// reload the random number generator
ul_int
rng_mt::reload()
{
  register ul_int* p0 = state;
  register ul_int* p2 = state + 2;
  register ul_int* pp = state + period;
  register ul_int s0;
  register ul_int s1;
  register int j;

  // if(left < -1)
  //  seed_state();

  left = length - 1;
  next = state + 1;

  for(s0 = state[0], s1 = state[1], j = length - period + 1; --j;
      s0 = s1, s1 = *p2++)
    *p0++ = *pp++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? magic : 0UL);

  for(pp = state, j = period; --j; s0 = s1, s1 = *p2++)
    *p0++ = *pp++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? magic : 0UL);

  s1=state[0];
  *p0 = *pp ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? magic : 0UL);
  s1 ^= (s1 >> 11);
  s1 ^= (s1 <<  7) & 0x9d2c5680UL;
  s1 ^= (s1 << 15) & 0xefc60000UL;
  return s1 ^ (s1 >> 18);
}

// rng_mt public methods
// copy current rng_mt, return pointer to it
rng*
rng_mt::copy()
{
  return new rng_mt(*this);
}

// change the seed, return old one
ul_int
rng_mt::set_seed(ul_int seed_)
{
  // set the seed
  ul_int old(rng::set_seed(seed_));
  // re-initialize the generator
  seed_state();
  return old;
}

CH_END_NAMESPACE

/* $Id: rng.cc,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
