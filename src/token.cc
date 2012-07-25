// Tokenizer interface method.
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

#include <iostream>
#include "file.h"
#include "t_string.h"
#include "token.h"
// make includes and declarations for lexer
#include <cstdio>
extern int tokenlex();
extern CH_STD::FILE* tokenin;
extern CH_STD::FILE* tokenout;

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

// these variable wil cause trouble during parallel execution
// global list of tokens used by lexer
extern token_seq token_list;
// current file name
static CH_STD::string token_input_path;

// tokenizer class methods
// ctor: set up everything
tokenizer::tokenizer(const CH_STD::string& path_)
  throw (bad_file, bad_input)
  : path(path_), tokens()
{
  // make sure we can access file
  file_stat file_info(path);
  if (!file_info.is_regular() || !file_info.read_permission())
    {
      throw bad_file(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		     ":tokenizer::tokenizer(): unable to open file "
		     + path + " for reading: " + file_info.why_no_read());
    }
  // set up lexer global variables
  // open file and set it up for lexing
  CH_STD::FILE* fpath = CH_STD::fopen(path.c_str(), "r");
  tokenin = fpath;
  // make sure the token list is empty
  token_list.clear();
  // set the current input file name
  token_input_path = path;
  // call the lexer to break up the file, check return value
  if (tokenlex() != 0)
    {
      throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		      ":tokenizer::tokenizer(): unable to token input stream "
		      "for file " + path);
    }
  // unset the token input path
  token_input_path.erase();
  // copy the global list to the object list and clear the global
  tokens = token_list;
  token_list.clear();
  // close the file and clean up
  CH_STD::fclose(fpath);
  tokenin = 0;
}

// dtor: do nothing
tokenizer::~tokenizer()
{}

// return iterator to beginning of token list
token_seq_citer
tokenizer::begin() const
{
  return tokens.begin();
}

// return iterator to end of token list
token_seq_citer
tokenizer::end() const
{
  return tokens.end();
}
 
// unrecoverable error
// default mesg = "parse error"
void
tokenizer::error(int line, const CH_STD::string& token,
		 const CH_STD::string& mesg)
    throw (bad_input)
{
  throw bad_input(PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
		  "tokenizer::error(): the parser has found a syntax "
		  "error in file " + token_input_path + " at line " +
		  t_string(line) + " at the characters '" + token + "':" + mesg);
  return;
}

// report error and return
// default mesg = "parse error"
void
tokenizer::recover(int line, const CH_STD::string& token,
		   const CH_STD::string& mesg)
{
  CH_STD::cerr << PACKAGE ":" __FILE__ ":" + t_string(__LINE__) +
    "tokenizer::recover(): the parser has found a syntax error in file " +
    token_input_path + " at line " + t_string(line) + " at the characters '"
    + token + "':" + mesg + " ...continuing" << CH_STD::endl;
  return;
}

CH_END_NAMESPACE

/* $Id: token.cc,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
