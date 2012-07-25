%{
/* Lexical analyzer for tokenizing input. */
/* Copyright (C) 2004 David J. Dooling <banjo@users.sourceforge.net>
 *
 * This file is part of CHIMP.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "token.h"

extern "C" int yywrap();

// declare namespace
CH_BEGIN_NAMESPACE

/* list of tokens in input, with comments removed */
token_seq token_list;

CH_END_NAMESPACE

%}

surface_species \@+[A-Za-z_0-9]*
word [A-Za-z_][A-Za-z_0-9]*
number -?([0-9]+(\.[0-9]*)?|[0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?
qstring \"[^\"\n]*[\"\n]
ws [\t ]

%%
  /* set line number for error reporting */
  int line_number = 1;

{ws}	;			/* ignore white space */

#.*	;			/* comments */

\n	{ ++line_number; }

{qstring} {
	  /* check quote and insert without quotes */
	  if (yytext[yyleng - 1] != '"') /* forgot end quote */
    	    {
	      CH_CHIMP::tokenizer::recover(line_number, yytext, "no terminating "
	                                   "double-quote before end of line, one "
	                                   "inserted");
	      ++line_number;
	    }
	  yytext[yyleng - 1] = '\0'; /* cut off ending quote or end line */
	  ++yytext;		/* move past initial quote */
	  CH_CHIMP::token_list.push_back(yytext);
	}

{surface_species} |
{word}	|
{number} |
.	{ CH_CHIMP::token_list.push_back(yytext); }

%%

extern "C" {
int
yywrap()
{
  return 1;
}
}

/* $Id: token_lex.ll,v 1.1.1.1 2004/11/25 20:24:06 banjo Exp $ */
