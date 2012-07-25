// -*- C++ -*-
// Definition of random number generator class.
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
#ifndef CH_MODEL_RNG_H
#define CH_MODEL_RNG_H 1

#include <cstdlib>
#include <string>
#include "except.h"
#include "token.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// typedef unsigned long int for better portability and ease of use
typedef unsigned long int ul_int; // must be at least 32 bits

// random number generator class
class rng
{
  CH_STD::string name;		// name of generator
  ul_int seed;			// random number generator seed

private:
  // prevent assignment
  rng& operator=(const rng&);
protected:
  // ctor: copy
  explicit rng(const rng& original);
public:
  // ctor: initialize seed and name
  rng(const CH_STD::string& name_, ul_int seed_ = 1);
  // dtor: do nothing
  virtual ~rng();

  // copy current rng, making and returning the appropriate type
  virtual rng* copy() = 0;
  // create a new random number generator of the given type, return pointer
  static rng* new_rng(const CH_STD::string& type = "mt");
  // parse rng input
  void parse(token_seq_citer& token_it, token_seq_citer end)
    throw (bad_input); // this
  // change the seed, return old one
  virtual ul_int set_seed(ul_int seed_);
  // return seed
  ul_int get_seed() const;
  // return name of generator
  CH_STD::string get_name() const;
  // return random int
  virtual ul_int get_random() = 0;
  // return a random int in range [0, N)
  ul_int get_random(ul_int n)
    { return get_random() % n; }
  // return the maximum integer reaturned by get_random()
  virtual ul_int get_max() = 0;
  // return random double in the range [0, MAX)
  double get_random_open(double max = 1.0e0)
    { return (max * get_random()) / (get_max() + 1.0e0); }
  // return random double in the range (0, MAX)
  double get_random_open_open(double max = 1.0e0)
    { return (max * (get_random() + 1.0e0)) / (get_max() + 2.0e0); }
  // return random double in the range [0, MAX]
  double get_random_closed(double max = 1.0e0)
    { return (max * get_random()) / get_max(); }
  // function operator which return random int [0, N)
  int operator()(ul_int n)
    { return get_random(n); }
}; // end class rng

// C library rand() - linear congruential rng
class rng_rand : public rng
{
public:
  // ctor: seed the rng with optional seed
  explicit rng_rand(ul_int seed_ = 1);
  // ctor: copy
  explicit rng_rand(const rng_rand& original);
  // dtor: do nothing
  ~rng_rand();

  // copy current rng_rand, return pointer to it
  virtual rng* copy();
  // change the seed, return old one
  virtual ul_int set_seed(ul_int seed_);
  // return random int in the range [0-RAND_MAX]
  virtual ul_int get_random()
    { return rand(); }
  // return the maximum integer reaturned by get_random()
  virtual ul_int get_max()
    { return RAND_MAX; }
}; // end class rng_rand

// the Mersenne Twister of Nishimura (recoded into ISO C++ by dd)
// This is the ``Mersenne Twister'' random number generator MT19937, which
// generates pseudorandom integers uniformly distributed in 0..(2^32 - 1)
// starting from any odd seed in 0..(2^32 - 1).  This version is a recode
// by Shawn Cokus (Cokus@math.washington.edu) on March 8, 1998 of a version by
// Takuji Nishimura (who had suggestions from Topher Cooper and Marc Rieffel in
// July-August 1997).
//
// Effectiveness of the recoding (on Goedel2.math.washington.edu, a DEC Alpha
// running OSF/1) using GCC -O3 as a compiler: before recoding: 51.6 sec. to
// generate 300 million random numbers; after recoding: 24.0 sec. for the same
// (i.e., 46.5% of original time), so speed is now about 12.5 million random
// number generations per second on this machine.
//
// According to the URL <http://www.math.keio.ac.jp/~matumoto/emt.html>
// (and paraphrasing a bit in places), the Mersenne Twister is ``designed
// with consideration of the flaws of various existing generators,'' has
// a period of 2^19937 - 1, gives a sequence that is 623-dimensionally
// equidistributed, and ``has passed many stringent tests, including the
// die-hard test of G. Marsaglia and the load test of P. Hellekalek and
// S. Wegenkittl.''  It is efficient in memory usage (typically using 2506
// to 5012 bytes of static data, depending on data type sizes, and the code
// is quite short as well).  It generates random numbers in batches of 624
// at a time, so the caching and pipelining of modern systems is exploited.
// It is also divide- and mod-free.
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation (either version 2 of the License or, at your
// option, any later version).  This library is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY, without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU Library General Public License for more details.  You should have
// received a copy of the GNU Library General Public License along with this
// library; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307, USA.
//
// The code as Shawn received it included the following notice:
//
//   Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.  When
//   you use this, send an e-mail to <matumoto@math.keio.ac.jp> with
//   an appropriate reference to your work.
//
// It would be nice to CC: <Cokus@math.washington.edu> when you write.
//
// private methods
// We initialize state[0..(length-1)] via the generator
//
//   x_new = (69069 * x_old) mod 2^32
//
// from Line 15 of Table 1, p. 106, Sec. 3.3.4 of Knuth's
// _The Art of Computer Programming_, Volume 2, 3rd ed.
//
// Notes (SJC): I do not know what the initial state requirements
// of the Mersenne Twister are, but it seems this seeding generator
// could be better.  It achieves the maximum period for its modulus
// (2^30) iff x_initial is odd (p. 20-21, Sec. 3.2.1.2, Knuth); if
// x_initial can be even, you have sequences like 0, 0, 0, ...;
// 2^31, 2^31, 2^31, ...; 2^30, 2^30, 2^30, ...; 2^29, 2^29 + 2^31,
// 2^29, 2^29 + 2^31, ..., etc. so I force seed to be odd below.
//
// Even if x_initial is odd, if x_initial is 1 mod 4 then
//
//   the          lowest bit of x is always 1,
//   the  next-to-lowest bit of x is always 0,
//   the 2nd-from-lowest bit of x alternates      ... 0 1 0 1 0 1 0 1 ... ,
//   the 3rd-from-lowest bit of x 4-cycles        ... 0 1 1 0 0 1 1 0 ... ,
//   the 4th-from-lowest bit of x has the 8-cycle ... 0 0 0 1 1 1 1 0 ... ,
//    ...
//
// and if x_initial is 3 mod 4 then
//
//   the          lowest bit of x is always 1,
//   the  next-to-lowest bit of x is always 1,
//   the 2nd-from-lowest bit of x alternates      ... 0 1 0 1 0 1 0 1 ... ,
//   the 3rd-from-lowest bit of x 4-cycles        ... 0 0 1 1 0 0 1 1 ... ,
//   the 4th-from-lowest bit of x has the 8-cycle ... 0 0 1 1 1 1 0 0 ... ,
//    ...
//
// The generator's potency (min. s>=0 with (69069-1)^s = 0 mod 2^32) is
// 16, which seems to be alright by p. 25, Sec. 3.2.1.3 of Knuth.  It
// also does well in the dimension 2..5 spectral tests, but it could be
// better in dimension 6 (Line 15, Table 1, p. 106, Sec. 3.3.4, Knuth).
//
// Note that the random number user does not see the values generated
// here directly since reload() will always munge them first, so maybe
// none of all of this matters.  In fact, the seed values made here could
// even be extra-special desirable if the Mersenne Twister theory says
// so-- that's why the only change I made is to restrict to odd seeds.
class rng_mt : public rng
{
  static const int length = 624; // length of state vector
  static const int period;	// a period parameter
  static const ul_int magic;	// a magic constant
  ul_int state[length + 1];	// state vector
  ul_int* next;			// next random value is computed from here
  int left;			// can *next++ this many times before reloading

private:
  // check seed and fill the state vector, set next and left
  void seed_state();
  // reload the random number generator
  ul_int reload();
  // in class inlines
  // mask all but the highest bit of u
  ul_int hiBit(ul_int u)
    { return u & 0x80000000UL; }
  // mask all but the lowest bit of u
  ul_int loBit(ul_int u)
    { return u & 0x00000001UL; }
  // mask the highest bit of u
  ul_int loBits(ul_int u)
    { return u & 0x7fffffffUL; }
  // move hi bit of u to hi bit of v
  ul_int mixBits(ul_int u, ul_int v)
    { return hiBit(u)|loBits(v); }
public:
  // ctor: (default) optional seed (should be odd in the range [0-2^32))
  explicit rng_mt(ul_int seed_ = 4357UL);
  // ctor: copy
  explicit rng_mt(const rng_mt& original);
  // dtor: do nothing
  ~rng_mt();

  // copy current rng_mt, return pointer to it
  virtual rng* copy();
  // change the seed, return old one
  virtual ul_int set_seed(ul_int seed_);
  // return random int in range [0, 2^32)
  ul_int get_random();
  // return maximum int returned by rng_mt
  virtual ul_int get_max()
    { return 0xffffffffUL; } // 0xffffffff = 2^32 - 1
}; // end class rng_mt

// inline functions
// return random int in range [0-2^32)
inline ul_int
rng_mt::get_random()
{
  ul_int y;

  if(--left < 0)
    return reload();

  y = *next++;
  y ^= (y >> 11);
  y ^= (y <<  7) & 0x9d2c5680UL;
  y ^= (y << 15) & 0xefc60000UL;
  return y ^ (y >> 18);
}

CH_END_NAMESPACE

#endif // CH_MODEL_RNG_H

/* $Id: rng.h,v 1.1.1.1 2004/11/25 20:24:08 banjo Exp $ */
