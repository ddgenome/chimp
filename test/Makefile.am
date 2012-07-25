## Process this file with automake to produce Makefile.in

EXTRA_DIST = bi.chimp bi.mech bi.out bi.par bi.task \
catalyst.chimp catalyst.mech catalyst.out catalyst.par catalyst.task \
complex.chimp complex.mech complex.out complex.par complex.task \
event.chimp event.coverage.par event.coverage.task event.event.par event.event.task event.mech event.out \
gas.chimp gas.mech gas.out gas.par gas.task \
gas_cstr.chimp gas_cstr.out  gas_cstr.task \
liquid.chimp liquid.mech  liquid.out liquid.par liquid.task \
multi.chimp multi.mech multi.out multi.par multi.task \
scale.chimp scale.mech scale.out scale.par scale.task \
set.chimp set.comp.mech set.mech set.out set.par set.task \
tpd.chimp tpd.explicit.mech tpd.explicit.task tpd.mech tpd.out tpd.par tpd.task\
uni.chimp uni.mech uni.out uni.par uni.task

## only run tests if perl exists
if PERLEXIST
TESTS = rtest
noinst_PROGRAMS = rtest
rtest_SOURCES = rtest.pl

rtest$(EXEEXT) : rtest.pl
	perl -c rtest.pl
	cp rtest.pl rtest
	chmod 755 rtest
endif
