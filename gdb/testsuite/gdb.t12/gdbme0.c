static int filelocal = 1;	/* In Data section */
static int filelocal_bss;	/* In BSS section */
#ifndef __STDC__
#define	const	/**/
#endif
static const int filelocal_ro = 201;	/* In Read-Only Data section */

main ()
{
  init ();
  foo ();
  autovars (5, 6);
  localscopes ();
}

/* On some systems, such as AIX, unreferenced variables are deleted
   from the executable.  */
usestatics ()
{
  useit (filelocal);
  useit (filelocal_bss);
  useit (filelocal_ro);
}

init ()
{
  filelocal_bss = 101;
  init1 ();
}

useit (val)
{
    static int usedval;

    usedval = val;
}

autovars (bcd, abc)
     int bcd;
     int abc;
{
    int  i0 =  0,  i1 =  1,  i2 =  2,  i3 =  3,  i4 =  4,  i5 =  5;
    int  i6 =  6,  i7 =  7,  i8 =  8,  i9 =  9, i10 = 10, i11 = 11;
    int i12 = 12, i13 = 13, i14 = 14, i15 = 15, i16 = 16, i17 = 17;
    int i18 = 18, i19 = 19, i20 = 20, i21 = 21, i22 = 22, i23 = 23;
    int i24 = 24, i25 = 25, i26 = 26, i27 = 27, i28 = 28, i29 = 29;
    int i30 = 30, i31 = 31, i32 = 32, i33 = 33, i34 = 34, i35 = 35;
    int i36 = 36, i37 = 37, i38 = 38, i39 = 39, i40 = 40, i41 = 41;
    int i42 = 42, i43 = 43, i44 = 44, i45 = 45, i46 = 46, i47 = 47;
    int i48 = 48, i49 = 49, i50 = 50, i51 = 51, i52 = 52, i53 = 53;
    int i54 = 54, i55 = 55, i56 = 56, i57 = 57, i58 = 58, i59 = 59;
    int i60 = 60, i61 = 61, i62 = 62, i63 = 63, i64 = 64, i65 = 65;
    int i66 = 66, i67 = 67, i68 = 68, i69 = 69, i70 = 70, i71 = 71;
    int i72 = 72, i73 = 73, i74 = 74, i75 = 75, i76 = 76, i77 = 77;
    int i78 = 78, i79 = 79, i80 = 80, i81 = 81, i82 = 82, i83 = 83;
    int i84 = 84, i85 = 85, i86 = 86, i87 = 87, i88 = 88, i89 = 89;
    int i90 = 90, i91 = 91, i92 = 92, i93 = 93, i94 = 94, i95 = 95;
    int i96 = 96, i97 = 97, i98 = 98, i99 = 99;

    /* Use all 100 of the local variables to derail agressive optimizers */

    useit ( i0); useit ( i1); useit ( i2); useit ( i3); useit ( i4);
    useit ( i5); useit ( i6); useit ( i7); useit ( i8); useit ( i9);
    useit (i10); useit (i11); useit (i12); useit (i13); useit (i14);
    useit (i15); useit (i16); useit (i17); useit (i18); useit (i19);
    useit (i20); useit (i21); useit (i22); useit (i23); useit (i24);
    useit (i25); useit (i26); useit (i27); useit (i28); useit (i29);
    useit (i30); useit (i31); useit (i32); useit (i33); useit (i34);
    useit (i35); useit (i36); useit (i37); useit (i38); useit (i39);
    useit (i40); useit (i41); useit (i42); useit (i43); useit (i44);
    useit (i45); useit (i46); useit (i47); useit (i48); useit (i49);
    useit (i50); useit (i51); useit (i52); useit (i53); useit (i54);
    useit (i55); useit (i56); useit (i57); useit (i58); useit (i59);
    useit (i60); useit (i61); useit (i62); useit (i63); useit (i64);
    useit (i65); useit (i66); useit (i67); useit (i68); useit (i69);
    useit (i70); useit (i71); useit (i72); useit (i73); useit (i74);
    useit (i75); useit (i76); useit (i77); useit (i78); useit (i79);
    useit (i80); useit (i81); useit (i82); useit (i83); useit (i84);
    useit (i85); useit (i86); useit (i87); useit (i88); useit (i89);
    useit (i90); useit (i91); useit (i92); useit (i93); useit (i94);
    useit (i95); useit (i96); useit (i97); useit (i98); useit (i99);

    useit (abc); useit (bcd);

    marker1 ();
}

localscopes ()
{
    int localval;

    localval = 0;
    useit (localval);

    {
	int localval = 10;
	int localval1 = 11;
	useit (localval);
	useit (localval1);
	marker2 ();
	{
	    int localval = 20;
	    int localval2 = 12;
	    useit (localval);
	    useit (localval2);
	    marker3 ();
	    {
		int localval = 30;
		int localval3 = 13;
		useit (localval);
		useit (localval3);
		marker4 ();
	    }
	}
    }
}

marker1 () {}
marker2 () {}
marker3 () {}
marker4 () {}
