# ldAout.tcl --
#
#	This "tclldAout" procedure in this script acts as a replacement
#	for the "ld" command when linking an object file that will be
#	loaded dynamically into Tcl or Tk using pseudo-static linking.
#
# Parameters:
#	The arguments to the script are the command line options for
#	an "ld" command.
#
# Results:
#	The "ld" command is parsed, and the "-o" option determines the
#	module name.  ".a" and ".o" options are accumulated.
#	The input archives and object files are examined with the "nm"
#	command to determine whether the modules initialization
#	entry and safe initialization entry are present.  A trivial
#	C function that locates the entries is composed, compiled, and
#	its .o file placed before all others in the command; then
#	"ld" is executed to bind the objects together.
#
#
# Copyright (c) 1995, by General Electric Company. All rights reserved.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# This work was supported in part by the ARPA Manufacturing Automation
# and Design Engineering (MADE) Initiative through ARPA contract
# F33615-94-C-4400.

proc tclLdAout {} {
  global env
  global argv
  
  # seenDotO is nonzero if a .o or .a file has been seen

  set seenDotO 0

  # minusO is nonzero if the last command line argument was "-o".

  set minusO 0

  # head has command line arguments up to but not including the first
  # .o or .a file. tail has the rest of the arguments.

  set head {}
  set tail {}

  # nmCommand is the "nm" command that lists global symbols from the
  # object files.

  set nmCommand {|nm -g}

  # entryPoints is the list of _Init and _SafeInit entries found in the
  # module

  set entryPoints {}

  # libraries is the list of -L and -l flags to the linker.

  set libraries {}

  # Process command line arguments

  foreach a $argv {
    if {!$minusO && [regexp {\.[ao]$} $a]} {
      set seenDotO 1
      lappend nmCommand $a
    }
    if {$minusO} {
      set outputFile $a
      set minusO 0
    } elseif {![string compare $a -o]} {
      set minusO 1
    }
    if {[regexp {^-[lL]} $a]} {
      lappend libraries $a
    } elseif {$seenDotO} {
      lappend tail $a
    } else {
      lappend head $a
    }
  }

  # Extract the module name from the "-o" option

  if {![info exists outputFile]} {
    error "-o option must be supplied to link a Tcl load module"
  }
  set m [file tail $outputFile]
  set l [expr [string length $m] - [string length $env(SHLIB_SUFFIX)]]
  if {[string compare [string range $m $l end] $env(SHLIB_SUFFIX)]} {
    error "Output file does not appear to have a $env(SHLIB_SUFFIX) suffix"
  }
  set modName [string toupper [string index $m 0]]
  append modName [string tolower [string range $m 1 [expr $l-1]]]
  
  # Catalog initialization entry points found in the module

  set f [open $nmCommand r]
  while {[gets $f l] >= 0} {
    if {[regexp { _?([A-Z][a-z0-9_]*_(Safe)?Init)$} $l trash symbol]} {
      lappend entryPoints $symbol
    }
  }
  close $f

  # Compose a C function that resolves the initialization entry points and
  # embeds the required libraries in the object code.

  set C {#include <string.h>}
  append C \n
  append C {char TclLoadLibraries_} $modName { [] =} \n
  append C {  "@LIBS: } $libraries {";} \n
  foreach symbol $entryPoints {
    append C {extern int } $symbol { (); } \n
  }
  append C {static struct } \{ \n
  append C {  char * name;} \n
  append C {  int (*value)();} \n
  append C \} {dictionary [] = } \{ \n
  foreach symbol $entryPoints {
    append C {  } \{ { "} $symbol {", } $symbol { } \} , \n
  }
  append C {  0, 0 } \n \} \; \n
  append C {typedef struct Tcl_Interp Tcl_Interp;} \n
  append C {typedef int Tcl_PackageInitProc (Tcl_Interp *);} \n
  append C {Tcl_PackageInitProc *} \n
  append C TclLoadDictionary_ $modName { (symbol)} \n
  append C {    char * symbol;} \n
  append C {{
    int i;
    for (i = 0; dictionary [i] . name != 0; ++i) {
      if (!strcmp (symbol, dictionary [i] . name)) {
	return dictionary [i].value;
      }
    }
    return 0;
}} \n

  # Write the C module and compile it

  set cFile tcl$modName.c
  set f [open $cFile w]
  puts -nonewline $f $C
  close $f
  set ccCommand "$env(CC) -c $env(SHLIB_CFLAGS) $cFile"
  puts stderr $ccCommand
  eval exec $ccCommand

  # Now compose and execute the ld command that packages the module

  set ldCommand ld
  foreach item $head {
    lappend ldCommand $item
  }
  lappend ldCommand tcl$modName.o
  foreach item $tail {
    lappend ldCommand $item
  }
  puts stderr $ldCommand
  eval exec $ldCommand

  # Clean up working files

  exec /bin/rm $cFile [file rootname $cFile].o
}
