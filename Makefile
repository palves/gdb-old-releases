# On HPUX, you need to add -Ihp-include to CFLAGS.
# The headers in the directory hp-include override system headers
# and tell GDB to use BSD executable file format.
# You must also define REGEX & REGEX1 below and ALLOCA & ALLOCA1 (get
# alloca.c from the emacs distribution) to the CLIBS.
# If you compile GDB with GCC on HPUX, you must make sure that the "nm" used
# in "munch" is GNU's nm.  This is because gcc uses a different .o
# file format than the native HPUX compiler.

# On USG (System V) machines, you must make sure to setup REGEX &
# REGEX1 to point at regex.o and use the USG version of CLIBS.  
# If your system has a broken alloca() -- most do -- then get
# alloca.c from the GNU Emacs distribution and set ALLOCA & ALLOCA1.
# Also, if you compile gdb with a compiler which uses the coff
# encapsulation feature (this is a function of the compiler used, NOT
# of the m-?.h file selected by config.gdb), you must make sure that
# the GNU nm is the one that is used by munch.

# On Sunos 4.0 machines, make sure to compile *without* shared
# libraries if you want to run gdb on itself.  Make sure to compile
# any program on which you want to run gdb without shared libraries.

# If you are compiling with GCC, make sure that either 1) You use the
# -traditional flag, or 2) You have the fixed include files where GCC
# can reach them.  Otherwise the ioctl calls in inflow.c will be
# incorrectly compiled.  The "fixincludes" script in the gcc
# distribution will probably fix your include files up.

CC=cc
SHELL=/bin/sh

# Set this up with gcc if you have gnu ld and the loader will print out
# line numbers for undefinded refs.  
CC-LD=${CC}

# -I. for "#include <obstack.h>".  Possibly regex.h also.
#CFLAGS = -g -pg -I. -O
CFLAGS = -I. -g
#LDFLAGS = -pg -g
LDFLAGS = -g

# define this to be "obstack.o" if you don't have the obstack library installed
# you must at the same time define OBSTACK1 as "obstack.o" 
# so that the dependencies work right.  Similarly with REGEX and "regex.o".
# You must define REGEX and REGEX1 on USG machines.
# If your system is missing alloca(), or, more likely, it's there but it
# doesn't work, define ALLOCA and ALLOCA1.
OBSTACK = obstack.o
OBSTACK1 = obstack.o
REGEX = regex.o
REGEX1 = regex.o
ALLOCA = alloca.o
ALLOCA1 = alloca.o
ADD_FILES = $(OBSTACK)  $(REGEX)  $(ALLOCA)  $(GNU_MALLOC)
ADD_DEPS  = $(OBSTACK1) $(REGEX1) $(ALLOCA1) $(GNU_MALLOC)

#
# define this to be "malloc.o" if you want to use the gnu malloc routine
# (useful for debugging memory allocation problems in gdb).  Otherwise, leave
# it blank.
GNU_MALLOC =
#GNU_MALLOC = malloc.o

# Flags to be used in compiling malloc.o
# Specify range checking for storage allocation.
MALLOC_FLAGS =
#MALLOC_FLAGS = ${CFLAGS} -Drcheck -Dbotch=fatal -DMSTATS

# for BSD
CLIBS = $(ADD_FILES)
# for USG
#CLIBS= $(ADD_FILES) -lPW

SFILES = blockframe.c breakpoint.c coffread.c command.c core.c dbxread.c \
	 environ.c eval.c expprint.c findvar.c infcmd.c inflow.c infrun.c \
	 kdb-start.c main.c printcmd.c \
	 remote.c source.c stack.c standalone.c stuff.c symmisc.c symtab.c \
	 utils.c valarith.c valops.c valprint.c values.c version.c expread.y \
	 xgdb.c

DEPFILES = convex-dep.c umax-dep.c gould-dep.c default-dep.c sun3-dep.c \
	   sparc-dep.c hp9k320-dep.c news-dep.c i386-dep.c

PINSNS = gld-pinsn.c i386-pinsn.c sparc-pinsn.c vax-pinsn.c m68k-pinsn.c \
	 ns32k-pinsn.c

HFILES = command.h defs.h environ.h expression.h frame.h getpagesize.h \
	 inferior.h symseg.h symtab.h value.h wait.h \
	 a.out.encap.h a.out.gnu.h stab.gnu.h 

OPCODES = m68k-opcode.h pn-opcode.h sparc-opcode.h npl-opcode.h vax-opcode.h \
	  ns32k-opcode.h

MFILES = m-hp9k320.h m-i386.h m-i386gas.h m-isi.h m-merlin.h m-news.h \
	 m-npl.h m-pn.h m-sparc.h m-sun2.h m-sun3.h m-sun2os4.h \
	 m-sun3os4.h m-sun4os4.h m-umax.h m-vax.h 

POSSLIBS = obstack.h obstack.c regex.c regex.h malloc.c 

TESTS = testbpt.c testfun.c testrec.c testreg.c testregs.c

OTHERS = Makefile createtags munch config.gdb ChangeLog README TAGS \
	 gdb.texinfo .gdbinit COPYING expread.tab.c stab.def hp-include

TAGFILES = ${SFILES} ${DEPFILES} ${PINSNS} ${HFILES} ${OPCODES} ${MFILES} \
	   ${POSSLIBS}
TARFILES = ${TAGFILES} ${OTHERS}

OBS = main.o blockframe.o breakpoint.o findvar.o stack.o source.o \
    values.o eval.o valops.o valarith.o valprint.o printcmd.o \
    symtab.o symmisc.o coffread.o dbxread.o infcmd.o infrun.o remote.o \
    command.o utils.o expread.o expprint.o pinsn.o environ.o version.o

TSOBS = core.o inflow.o dep.o

NTSOBS = standalone.o

TSSTART = /lib/crt0.o

NTSSTART = kdb-start.o

gdb : $(OBS) $(TSOBS) $(ADD_DEPS)
	-rm -f init.c
	./munch $(OBS) $(TSOBS) > init.c
	${CC-LD} $(LDFLAGS) -o gdb init.c $(OBS) $(TSOBS) $(CLIBS)

xgdb : $(OBS) $(TSOBS) xgdb.o $(ADD_DEPS)
	-rm -f init.c
	./munch $(OBS) $(TSOBS) xgdb.o > init.c
	$(CC-LD) $(LDFLAGS) -o xgdb init.c $(OBS) $(TSOBS) xgdb.o \
	-lXaw -lXt -lX11 $(CLIBS)

kdb : $(NTSSTART) $(OBS) $(NTSOBS) $(ADD_DEPS)
	-rm -f init.c
	./munch $(OBS) $(NTSOBS) > init.c
	$(CC-LD) $(LDFLAGS) -c init.c $(CLIBS)
	ld -o kdb $(NTSSTART) $(OBS) $(NTSOBS) init.o -lc $(CLIBS)

# If it can figure out the appropriate order, createtags will make sure
# that the proper m-*, *-dep, *-pinsn, and *-opcode files come first
# in the tags list.  It will attempt to do the same for dbxread.c and 
# coffread.c.  This makes using M-. on machine dependent routines much 
# easier.
#
TAGS: ${TAGFILES}
	createtags ${TAGFILES}
tags: TAGS

gdb.tar: ${TARFILES}
	rm -f gdb.tar
	mkdir dist-gdb
	cd dist-gdb ; for i in ${TARFILES} ; do ln -s ../$$i . ; done
	tar chf gdb.tar dist-gdb
	rm -rf dist-gdb

gdb.tar.Z: gdb.tar
	compress gdb.tar

clean:
	-rm -f ${OBS} ${TSOBS} ${NTSOBS} ${OBSTACK} ${REGEX}
	-rm -f init.c init.o 
	-rm -f gdb

realclean: clean
	-rm -f expread.tab.c tags TAGS

xgdb.o : xgdb.c defs.h param.h symtab.h frame.h
	$(CC) -c $(CFLAGS) xgdb.c -o $@

expread.tab.c : expread.y
	@echo 'Expect 101 shift/reduce conflicts and 1 reduce/reduce conflict.'
	yacc expread.y
	mv y.tab.c expread.tab.c

expread.o : expread.tab.c defs.h param.h symtab.h frame.h expression.h
	$(CC) -c ${CFLAGS} expread.tab.c
	mv expread.tab.o expread.o

#
# Only useful if you are using the gnu malloc routines.
#
malloc.o : malloc.c
	${CC} -c ${MALLOC_FLAGS} malloc.c

#
# dep.o depends on ALL the dep files since we don't know which one
# is really being used.
#
dep.o : ${DEPFILES} defs.h param.h frame.h inferior.h obstack.h \
	a.out.encap.h

# pinsn.o depends on ALL the opcode printers
# since we don't know which one is really being used.
pinsn.o : ${PINSNS} defs.h param.h symtab.h obstack.h symseg.h frame.h \
	  ${OPCODES}

#
# The rest of this is a standard dependencies list (hand edited output of
# cpp -M).  It does not include dependencies of .o files on .c files.
#
blockframe.o : defs.h param.h symtab.h obstack.h symseg.h frame.h 
breakpoint.o : defs.h param.h symtab.h obstack.h symseg.h frame.h
coffread.o : defs.h param.h 
command.o : command.h defs.h
core.o : defs.h  param.h a.out.encap.h
dbxread.o : param.h defs.h symtab.h obstack.h symseg.h a.out.encap.h \
	    stab.gnu.h
environ.o : environ.h 
eval.o : defs.h  param.h symtab.h obstack.h symseg.h value.h expression.h 
expprint.o : defs.h symtab.h obstack.h symseg.h param.h expression.h
findvar.o : defs.h param.h symtab.h obstack.h symseg.h frame.h value.h 
infcmd.o : defs.h  param.h symtab.h obstack.h symseg.h frame.h inferior.h \
	   environ.h value.h
inflow.o : defs.h  param.h frame.h inferior.h
infrun.o : defs.h  param.h symtab.h obstack.h symseg.h frame.h inferior.h \
	   wait.h
kdb-start.o : defs.h param.h 
main.o : defs.h  command.h param.h
malloc.o :  getpagesize.h
obstack.o : obstack.h 
printcmd.o :  defs.h param.h frame.h symtab.h obstack.h symseg.h value.h \
	      expression.h 
regex.o : regex.h 
remote.o : defs.h  param.h frame.h inferior.h wait.h
source.o : defs.h  symtab.h obstack.h symseg.h param.h
stack.o :  defs.h param.h symtab.h obstack.h symseg.h frame.h 
standalone.o : defs.h param.h symtab.h obstack.h symseg.h frame.h \
	       inferior.h wait.h 
symmisc.o : defs.h symtab.h obstack.h symseg.h obstack.h 
symtab.o : defs.h  symtab.h obstack.h symseg.h param.h  obstack.h
utils.o : defs.h  param.h 
valarith.o : defs.h param.h symtab.h obstack.h symseg.h value.h expression.h 
valops.o :  defs.h param.h symtab.h obstack.h symseg.h value.h frame.h \
	    inferior.h
valprint.o :  defs.h param.h symtab.h obstack.h symseg.h value.h 
values.o :  defs.h param.h symtab.h obstack.h symseg.h value.h 

robotussin.h : getpagesize.h   
symtab.h : obstack.h symseg.h 
a.out.encap.h : a.out.gnu.h
