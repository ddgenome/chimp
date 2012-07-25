%{
/* Parser syntax for mechanism. */
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
#endif /* HAVE_CONFIG_H */

#include <cstdio>
#include <utility>
/* include all the required headers */
#include "except.h"
#include "k.h"
#include "manager.h"
#include "mech_lex.h"
#include "mechanism.h"
#include "parameter.h"
#include "reaction.h"
#include "species.h"
#include "t_string.h"

/* declare for the benefit of the C++ compiler */
extern int yyerror(const char *mesg);

// tell the compiler we are going to freely use the chimp namespace
CH_USING_NAMESPACE;

%}

%union {
  /* these are from the lexer */
  double dval;
  char* sval;
  /* these are for non-terminal symbols */
  CH_CHIMP::par_expression* par_expression_val;
  CH_CHIMP::par_expression_seq* par_expression_seq_val;
  CH_CHIMP::k* k_val;
  CH_STD::pair<CH_CHIMP::k*,CH_CHIMP::k*>* pair_k_k_val;
  CH_CHIMP::species* species_val;
  CH_CHIMP::species::set* species_set_val;
  CH_STD::pair<CH_CHIMP::species*,CH_CHIMP::stoichiometric>* pair_species_stoich_val;
  CH_CHIMP::stoich_map* stoich_map_val;
}

%token <sval> SURFACE_SPECIES
%token <sval> SVAL
%token <dval> NUMBER
%token FORWARD_ARROW
%token REVERSE_ARROW
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS
%left '^'

%type <par_expression_val> parameter_exp
%type <par_expression_seq_val> parameter_exp_list
%type <k_val> rate_constant
%type <pair_k_k_val> rate_exp
%type <species_val> gen_species species
%type <species_set_val> species_set
%type <pair_species_stoich_val> stoich_species species_exp
%type <stoich_map_val> species_exp_list

%%

reaction_list:  reaction ';'
	| reaction_list reaction ';'
	;

reaction: species_exp_list rate_exp species_exp_list {
		/* create an empty reaction with given rate constants */
		reaction* rxn = new reaction($2->first, $2->second);
		delete $2;
		/* perform in a try block */
		try
		  {
		    /* add reactants */
		    for (stoich_map_citer it($1->begin()); it != $1->end();
			 ++it)
		      {
			rxn->add_reactant(it->first, it->second);
		      }
		    /* add products */
		    for (stoich_map_citer it($3->begin()); it != $3->end();
			 ++it)
		      {
			rxn->add_product(it->first, it->second);
		      } 
		  }
		catch (bad_input& bi)
		  {
		    yyerror(bi.what());
		    YYABORT;
		  }
		/* delete the reactant and product stoich_maps */
		delete $1;
		delete $3;
		/* insert reaction into current mechanism */
		try
		  {
		    task_manager::get().get_current_mechanism()->insert_reaction(rxn);
		  }
		catch (bad_pointer& bp)
		  {
		    yyerror(bp.what());
		    YYABORT;
		  }
	}
	| species {
		/* for inert or reactant pool, added by species rule */
		; /* do nothing */
	}
	;

species_exp_list: species_exp {
		/* create a map, must delete when done */
		stoich_map* smp = new stoich_map;
		/* insert the species/stoich pair */
		smp->insert(*$1);
		/* delete the pair */
		delete $1;
		/* assign the map to the symbol */
		$$ = smp;
	}
	| species_exp_list '+' species_exp {
		/* use species/stoich pair to increment or create entry in map */
		try
		  {
		    (*$1)[$3->first].increment($3->second);
		  }
		catch (bad_input& bi)
		  {
		    yyerror(bi.what());
		    YYABORT;
		  }
		/* delete the pair */
		delete $3;
		/* assign the map to the symbol */
		$$ = $1;
	}
	;

species_exp: stoich_species {
		/* no power given */
		$$ = $1;
	}
	| stoich_species '^' NUMBER {
		/* set rate law power (different from stoich coeff) */
		$1->second.set_power($3);
		/* assign value of symbol */
		$$ = $1;
	}
	;

stoich_species: gen_species {
		/* no stoichiometric coeff given -> 1 */
		/* create a stoichiometric coefficient */
		stoichiometric stoich(1.0e0);
		/* set symbol to a new species*-stoichiometric pair */
		$$ = new CH_STD::pair<species*,stoichiometric>($1, stoich);
	}
	| NUMBER gen_species {
		/* stoichiometric coefficient given */
		/* create a stoichiometric coefficient */
		stoichiometric stoich($1);
		/* set symbol to a new species*-stoichiometric pair */
		$$ = new CH_STD::pair<species*,stoichiometric>($2, stoich);
	}
	;

gen_species: species {
		$$ = $1;
	}
	| '[' species_set ']' {
		/* create a species_set */
		species_set* ssp = new species_set(*$2);
		/* assign its value to the symbol */
		$$ = ssp;
		/* delete the set of species */
		delete $2;
	}
	;

species_set: species {
		/* create the set, must be deleted */
		species::set* ssp = new species::set;
		/* insert the surface species */
		ssp->insert($1);
		/* assign set pointer to symbol */
		$$ = ssp;
	}
	| species_set ',' species {
		/* insert the new species into the set */
		$1->insert($3);
		/* assign set pointer to symbol */
		$$ = $1;
	}
	;

species: SVAL {
		/* insert species in mechanism (creating if necessary) and
		 * assign pointer to symbol */
		try
		  {
		    $$ = task_manager::get().get_current_mechanism()->insert_species($1);
		  }
		catch (bad_pointer& bp)
		  {
		    yyerror(bp.what());
		    YYABORT;
		  }
		/* delete the sval pointer */
		delete[] $1;
	}
	| SURFACE_SPECIES {
		/* insert surface species in mechanism (creating if necessary)
		 * and assign pointer to symbol */
		try
		  {
		    $$ = task_manager::get().get_current_mechanism()->insert_species($1);
		  }
		catch (bad_pointer& bp)
		  {
		    yyerror(bp.what());
		    YYABORT;
		  }
		/* delete the sval pointer */
		delete[] $1;
	}
	;

rate_exp: FORWARD_ARROW rate_constant {
		/* assign forward rate constant and null pointer */
		$$ = new CH_STD::pair<k*,k*>($2, 0);
	}
	| FORWARD_ARROW rate_constant REVERSE_ARROW rate_constant {
		/* put forward first */
		$$ = new CH_STD::pair<k*,k*>($2, $4);
	}
	| REVERSE_ARROW rate_constant FORWARD_ARROW rate_constant {
		/* put forward first */
		$$ = new CH_STD::pair<k*,k*>($4, $2);
	}
	;

rate_constant: SVAL '(' parameter_exp_list ')' {
		/* try to create a rate constant */
		try
		  {
		    /* create a constant of appropriate type */
		    k* kp(k::new_k($1, *$3));
		    /* make sure a valid type was given */
		    if (kp == 0)
		      {
			/* invalid rate constant type */
			CH_STD::string er("unknown rate constant type: ");
			er += $1;
			yyerror(er.c_str());
			YYABORT;
		      }
		    /* delete the parameter experssion list */
		    delete $3;
		    /* assign the symbol */
		    $$ = kp;
		  }
		/* see if the appropriate number of parameters were given */
		catch (bad_input& bi)
		  {
		    yyerror(bi.what());
		    YYABORT;
		  }
		/* see if transfer coefficient is valid */
		catch (bad_value& bv)
		  {
		    yyerror(bv.what());
		    YYABORT;
		  }
		/* delete the sval pointer */
		free($1);
	}
	;

parameter_exp_list: parameter_exp {
		/* create a vector, must delete above */
		par_expression_seq* pes = new par_expression_seq;
		/* insert the pointer to the parameter_exp */
		pes->push_back($1);
		/* assign the vector to the symbol */
		$$ = pes;
	}
	| parameter_exp_list ',' parameter_exp {
		/* insert the parameter_exp pointer */
		$1->push_back($3);
		$$ = $1;
	}
	;

parameter_exp: parameter_exp '+' parameter_exp {
		$$ = new par_sum($1, $3);
	}
	| parameter_exp '-' parameter_exp {
		$$ = new par_difference($1, $3);
	}
	| parameter_exp '*' parameter_exp {
		$$ = new par_product($1, $3);
	}
	| parameter_exp '/' parameter_exp {
		$$ = new par_ratio($1, $3);
	}
	| parameter_exp '^' parameter_exp {
		$$ = new par_pow($1, $3);
	}
	| '-' parameter_exp %prec UMINUS {
		$$ = new par_minus($2);
	}
	| '(' parameter_exp ')' {
		// just set the symbol
		$$ = $2;
	}
	| SVAL {
		// put parameter into mechanism, create par_single, and equate
		// symbol with pointer
		try
		  {
		    $$ = new par_single(task_manager::get().get_current_mechanism()->insert_parameter($1));
		  }
		catch (bad_pointer& bp)
		  {
		    yyerror(bp.what());
		    YYABORT;
		  }
		/* delete the sval pointer */
		free($1);
	}
	| NUMBER {
		// create a par_single, put parameter (with number as name)
		// into mechanism, and equate symbol with pointer to par_single
		try
		  {
		    $$ = new par_single(task_manager::get().get_current_mechanism()->insert_parameter(t_string($1), $1));
		  }
		catch (bad_pointer& bp)
		  {
		    yyerror(bp.what());
		    YYABORT;
		  }
	}
	;

%%

int
yyerror(const char *mesg)
{
  /* task_manager::get_current_mechanism() might throw an exception */
  try
    {
      fprintf(stderr, PACKAGE ":" __FILE__ ":yyparse(): parse error in %s at "
	      "line %d: %s at '%s'\n",
	      CH_CHIMP::task_manager::get().get_current_mechanism()->get_name().c_str(),
	      CH_CHIMP::mech_line_number, mesg, yytext);
    }
  catch (bad_pointer& bp)
    {
      fprintf(stderr, PACKAGE ":" __FILE__ ":yyparse(): parse error in at "
	      "line %d: %s at '%s'\n", CH_CHIMP::mech_line_number, mesg, yytext);
    }
  return 0;
}

/* $Id: mech_parse.yy,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
