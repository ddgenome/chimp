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
#ifndef CH_MECH_LEX_H
#define CH_MECH_LEX_H 1

// make includes and declarations for lexer
#include <cstdio>
extern int yylex();
extern CH_STD::FILE *yyin;
extern CH_STD::FILE *yyout;
extern char *yytext;

// set namespace to avoid possible clashes
CH_BEGIN_NAMESPACE

extern int mech_line_number;

CH_END_NAMESPACE

#endif // not CH_MECH_LEX_H

/* $Id: mech_lex.h,v 1.1.1.1 2004/11/25 20:24:05 banjo Exp $ */
