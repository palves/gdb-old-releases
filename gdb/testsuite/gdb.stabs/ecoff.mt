# This configuration is for a gcc that uses mips-tfile. If your gcc
# uses gas, you should configure gdb --with-gnu-as.
#
# The mips as doesn't grok #line directives, suppress them via -P during
# preprocessing and change long to word as mips as doesn't grok .long.
# The sed script removes blanks that mips-tfile doesn't like and
# embedds stabs in comments.
#
WEIRDSTABS_S=weird-ecoff.S

weird.o: ${srcdir}/${WEIRDSTABS_S} ${srcdir}/weird.def ${srcdir}/weird-ecoff.sed
	cp ${srcdir}/${WEIRDSTABS_S} tmp.c
	$(CC) -I${srcdir} -E -P -Dlong=word tmp.c >tmp.s
	sed -f ${srcdir}/weird-ecoff.sed <tmp.s >weird.s
	$(CC) -c weird.s
