// -*- C++ -*-
// Functions and variables used by lexer and needed by outside.
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
#ifndef CH_TOKEN_H
#define CH_TOKEN_H 1

#include <string>
#include <vector>
#include "except.h"

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// token sequence typedef's
typedef CH_STD::vector<CH_STD::string> token_seq;
typedef token_seq::iterator token_seq_iter;
typedef token_seq::const_iterator token_seq_citer;

// class to provide interface to lexer
class tokenizer
{
  CH_STD::string path;		// input file path
  token_seq tokens;		// list of input tokens

private:
  // prevent copy construction and assignment
  tokenizer(const tokenizer&);
  tokenizer& operator=(const tokenizer&);
public:
  // ctor: input file name given
  tokenizer(const CH_STD::string& path_)
    throw (bad_file, bad_input); // this, file_stat
  // dtor: do nothing
  ~tokenizer();

  // return iterator to beginning of token list
  token_seq_citer begin() const;
  // return iterator to end of token list
  token_seq_citer end() const;
  // error routines for lexer to call
  // unrecoverable error
  static void error(int line, const CH_STD::string& token,
		    const CH_STD::string& mesg = "parse error")
    throw (bad_input); // this
  // report error and return
  static void recover(int line, const CH_STD::string& token,
		      const CH_STD::string& mesg = "parse error");
}; // end class tokenizer

CH_END_NAMESPACE

#endif // not CH_TOKEN_H

/* $Id: token.h,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
