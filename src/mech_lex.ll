%{
/* Lexical analyzer for parsing mechanisms. */
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

/* include all the requisite headers to define the union */
#include "parameter.h"
#include "k.h"
#include <utility>
#include "species.h"
#include "reaction.h"
/* include bison's header */
#include "mech_parse.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

/* so we can report line numbers */
int mech_line_number = 1;

CH_END_NAMESPACE

%}

surface_species \@+[A-Za-z_0-9]*
word [A-Za-z_][A-Za-z_0-9]*
number ([0-9]+(\.[0-9]*)?|[0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?

%%

[\t ]	;			/* ignore white space */

\n	{ CH_CHIMP::mech_line_number++; } /* increment line number */

#.*	;			/* comments */

{surface_species} {
	  /* surface species */
	  /* this has own rule because `@' not acceptible for parameters */
	  yylval.sval = new char[yyleng + 1];
	  CH_STD::strcpy(yylval.sval, yytext);
	  return SURFACE_SPECIES;
	}

{word}	{
	  /* word */
	  yylval.sval = new char[yyleng + 1];
	  CH_STD::strcpy(yylval.sval, yytext);
	  return SVAL;
	}

{number} {
	  /* number */
	  yylval.dval = CH_STD::atof(yytext);
	  return NUMBER;
	}

"->"	{
	  /* forward reaction operator */
	  return FORWARD_ARROW;
	}

"<-"	{
	  /* reverse reaction operator */
	  return REVERSE_ARROW;
	}

.	{ return yytext[0]; }	/* anything else */

%%

/* $Id: mech_lex.ll,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
