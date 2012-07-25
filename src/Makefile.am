## Process this file with automake to produce Makefile.in

SUBDIRS = model

##INCLUDES = -I$(top_srcdir)/lib

bin_PROGRAMS = chimp

chimp_SOURCES = chimp.cc chimp.h compare.cc compare.h constant.cc constant.h counter.cc counter.h debug.cc debug.h except.h file.cc file.h handler.cc handler.h k.cc k.h manager.cc manager.h mech_lex.h mech_lex.ll mech_parse.yy mechanism.cc mechanism.h model_mech.cc model_mech.h par_task.cc par_task.h parameter.cc parameter.h precision.cc precision.h quantity.cc quantity.h reaction.cc reaction.h species.cc species.h t_string.h task.cc task.h token.cc token.h token_lex.ll unique.cc unique.h

EXTRA_DIST = mech_parse.h

chimp_LDADD = model/libmodel.a @LEXLIB@

# custom variables
# make sure bison outputs the header file
AM_YFLAGS = -d

# custom rules
# make sure mech_parse.h is treated properly
mech_parse.h: mech_parse.cc
mech_lex.o: mech_parse.h

# change prefix on the comment lexer
token_lex.cc: $(srcdir)/token_lex.ll
	$(LEX) $(AM_LFLAGS) $(LFLAGS) -Ptoken $<
	mv $(LEX_OUTPUT_ROOT:yy=token).c $@
