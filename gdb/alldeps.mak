# Start of "alldeps.mak" definitions
ALLDEPFILES = $(ALLDEPFILES_MAINDIR) $(ALLDEPFILES_SUBDIR)
ALLDEPFILES_MAINDIR=\
altos-xdep.c\
am29k-pinsn.c\
am29k-tdep.c\
arm-convert.s\
arm-pinsn.c\
arm-tdep.c\
arm-xdep.c\
convex-pinsn.c\
convex-tdep.c\
convex-xdep.c\
coredep.c\
exec.c\
gould-pinsn.c\
gould-xdep.c\
hp300ux-xdep.c\
i386-pinsn.c\
i386-tdep.c\
i386-xdep.c\
i387-tdep.c\
i960-pinsn.c\
i960-tdep.c\
infptrace.c\
m68k-pinsn.c\
m68k-tdep.c\
m88k-pinsn.c\
m88k-tdep.c\
m88k-xdep.c\
mips-pinsn.c\
mips-tdep.c\
mips-xdep.c\
mipsread.c\
news-xdep.c\
nindy-tdep.c\
ns32k-pinsn.c\
pyr-pinsn.c\
pyr-tdep.c\
pyr-xdep.c\
remote-adapt.c\
remote-eb.c\
remote-mm.c\
remote-nindy.c\
remote-vx.c\
solib.c\
sparc-pinsn.c\
sparc-tdep.c\
sparc-xdep.c\
sun3-xdep.c\
sun386-xdep.c\
symm-tdep.c\
symm-xdep.c\
tahoe-pinsn.c\
tdesc.c\
ultra3-xdep.c\
umax-xdep.c\
vax-pinsn.c

ALLDEPFILES_SUBDIR=\
nindy-share/Onindy.c\
nindy-share/nindy.c\
nindy-share/ttybreak.c\
nindy-share/ttyflush.c\
vx-share/xdr_ld.c\
vx-share/xdr_ptrace.c\
vx-share/xdr_rdb.c\
vx-share/xdr_regs.c

ALLPARAM=\
tm-29k.h\
tm-3b1.h\
tm-88k.h\
tm-altos.h\
tm-altosgas.h\
tm-arm.h\
tm-bigmips.h\
tm-convex.h\
tm-hp300bsd.h\
tm-hp300hpux.h\
tm-i386v-g.h\
tm-i386v.h\
tm-irix3.h\
tm-isi.h\
tm-merlin.h\
tm-mips.h\
tm-news.h\
tm-nindy960.h\
tm-np1.h\
tm-pn.h\
tm-pyr.h\
tm-sparc.h\
tm-sun2.h\
tm-sun2os4.h\
tm-sun3.h\
tm-sun386.h\
tm-sun3os4.h\
tm-sun4os4.h\
tm-symmetry.h\
tm-tahoe.h\
tm-ultra3.h\
tm-umax.h\
tm-vax.h\
tm-vx68.h\
tm-vx960.h\
xm-3b1.h\
xm-88k.h\
xm-altos.h\
xm-arm.h\
xm-bigmips.h\
xm-convex.h\
xm-hp300bsd.h\
xm-hp300hpux.h\
xm-i386sco.h\
xm-i386v.h\
xm-i386v32.h\
xm-irix3.h\
xm-isi.h\
xm-merlin.h\
xm-mips.h\
xm-news.h\
xm-news1000.h\
xm-np1.h\
xm-pn.h\
xm-pyr.h\
xm-rtbsd.h\
xm-sparc.h\
xm-sun2.h\
xm-sun3.h\
xm-sun386.h\
xm-sun3os4.h\
xm-sun4os4.h\
xm-symmetry.h\
xm-tahoe.h\
xm-ultra3.h\
xm-umax.h\
xm-vax.h

ALLCONFIG=\
./tconfig/3b1\
./tconfig/a29k\
./tconfig/a29k-kern\
./tconfig/altos\
./tconfig/altosgas\
./tconfig/arm\
./tconfig/bigmips\
./tconfig/convex\
./tconfig/dec3100\
./tconfig/hp300bsd\
./tconfig/hp300hpux\
./tconfig/i386sco\
./tconfig/i386v\
./tconfig/i386v-g\
./tconfig/i386v32\
./tconfig/i386v32-g\
./tconfig/i960\
./tconfig/irix3\
./tconfig/isi\
./tconfig/littlemips\
./tconfig/m88k\
./tconfig/merlin\
./tconfig/news\
./tconfig/news1000\
./tconfig/nindy960\
./tconfig/none\
./tconfig/np1\
./tconfig/pn\
./tconfig/pyramid\
./tconfig/sun2os3\
./tconfig/sun2os4\
./tconfig/sun3\
./tconfig/sun386\
./tconfig/sun3os3\
./tconfig/sun3os4\
./tconfig/sun4\
./tconfig/sun4os3\
./tconfig/sun4os4\
./tconfig/symmetry\
./tconfig/tahoe\
./tconfig/ultra3\
./tconfig/umax\
./tconfig/vax\
./tconfig/vxworks68\
./tconfig/vxworks960\
./xconfig/3b1\
./xconfig/altos\
./xconfig/altosgas\
./xconfig/arm\
./xconfig/bigmips\
./xconfig/convex\
./xconfig/dec3100\
./xconfig/hp300bsd\
./xconfig/hp300hpux\
./xconfig/i386sco\
./xconfig/i386v\
./xconfig/i386v-g\
./xconfig/i386v32\
./xconfig/i386v32-g\
./xconfig/irix3\
./xconfig/isi\
./xconfig/littlemips\
./xconfig/m88k\
./xconfig/merlin\
./xconfig/news\
./xconfig/news1000\
./xconfig/none\
./xconfig/np1\
./xconfig/pn\
./xconfig/pyramid\
./xconfig/rtbsd\
./xconfig/sun2os3\
./xconfig/sun2os4\
./xconfig/sun3\
./xconfig/sun386\
./xconfig/sun3os3\
./xconfig/sun3os4\
./xconfig/sun4\
./xconfig/sun4os3\
./xconfig/sun4os4\
./xconfig/symmetry\
./xconfig/tahoe\
./xconfig/ultra3\
./xconfig/umax\
./xconfig/vax

# End of "alldeps.mak" definitions
