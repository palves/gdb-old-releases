# console.tcl --
#
# This code constructs the console window for an application.  It
# can be used by non-unix systems that do not have built-in support
# for shells.
#
#
# Copyright (c) 1995 Sun Microsystems, Inc.
#
# See the file "license.terms" for information on usage and redistribution
# of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

# TODO: fix history - last event skiped - change history command
#	or use "history event [expr [history nextid] - 1]]"
# TODO: history - remember partially written command
# TODO: get better default size for console -
#       auto configure based on font size???

# tkConsoleInit --
# This procedure constructs and configures the console windows.
#
# Arguments:
# 	None.

proc tkConsoleInit {} {
    global tcl_platform

    text .console  -yscrollcommand ".sb set" -setgrid true
    scrollbar .sb -command ".console yview"
    pack .sb -side right -fill both
    pack .console -fill both -expand 1 -side left
    if {$tcl_platform(platform) == "macintosh"} {
        after idle {.console configure -font {Monaco 9 normal}}
    }

    tkConsoleBind .console

    .console tag configure stderr -foreground red
    .console tag configure stdout -foreground black
    .console tag configure stdin -foreground blue

    focus .console
    
    wm protocol . WM_DELETE_WINDOW { wm withdraw . }
    tkConsolePrompt
}

# tkConsoleInvoke --
# Processes the command line input.  If the command is complete it
# is evaled in the main interpreter.  Otherwise, the continuation
# prompt is added and more input may be added.
#
# Arguments:
# None.

proc tkConsoleInvoke {args} {
    set ranges [.console tag ranges input]
    set cmd ""
    if {$ranges != ""} {
	set pos 0
	while {[lindex $ranges $pos] != ""} {
	    set start [lindex $ranges $pos]
	    set end [lindex $ranges [incr pos]]
	    append cmd [.console get $start $end]
	    incr pos
	}
    }
    if {$cmd == ""} {
	tkConsolePrompt
    } elseif [info complete $cmd] {
	.console mark set output end
	.console tag delete input
        set result [interp eval "" [list history add $cmd exec]]
	if {$result != ""} {
	    .console insert insert "$result\n"
	}
	tkConsoleHistory reset
	tkConsolePrompt
    } else {
	tkConsolePrompt partial
    }
    .console yview -pickplace insert
}

# tkConsoleHistory --
# This procedure implements command line history for the
# console.  In general is evals the history command in the
# main interpreter to obtain the history.  The global variable
# histNum is used to store the current location in the history.
#
# Arguments:
# cmd -	Which action to take: prev, next, reset.

set histNum 1
proc tkConsoleHistory {cmd} {
    global histNum
    
    switch $cmd {
    	prev {
	    incr histNum -1
	    if {$histNum == 0} {
		set cmd {history event [expr [history nextid] -1]}
	    } else {
		set cmd "history event $histNum"
	    }
    	    if {[catch {interp eval "" $cmd} cmd]} {
    	    	incr histNum
    	    	return
    	    }
	    .console delete promptEnd end
    	    .console insert promptEnd $cmd {input stdin}
    	}
    	next {
	    incr histNum
	    if {$histNum == 0} {
		set cmd {history event [expr [history nextid] -1]}
	    } elseif {$histNum > 0} {
		set cmd ""
		set histNum 1
	    } else {
		set cmd "history event $histNum"
	    }
	    if {$cmd != ""} {
		catch {interp eval "" $cmd} cmd
	    }
	    .console delete promptEnd end
	    .console insert promptEnd $cmd {input stdin}
    	}
    	reset {
    	    set histNum 1
    	}
    }
}

# tkConsolePrompt --
# This procedure draws the prompt.  If tcl_prompt1 or tcl_prompt2
# exists in the main interpreter it will be called to generate the 
# prompt.  Otherwise, a hard coded default prompt is printed.
#
# Arguments:
# partial -	Flag to specify which prompt to print.

proc tkConsolePrompt {{partial normal}} {
    if {$partial == "normal"} {
	set temp [.console index "end - 1 char"]
	.console mark set output end
    	if [interp eval "" "info exists tcl_prompt1"] {
    	    interp eval "" "eval \[set tcl_prompt1\]"
    	} else {
    	    puts -nonewline "tcl> "
    	}
    } else {
	set temp [.console index output]
	.console mark set output end
    	if [interp eval "" "info exists tcl_prompt2"] {
    	    interp eval "" "eval \[set tcl_prompt2\]"
    	} else {
	    puts -nonewline "> "
    	}
    }
    .console mark set output $temp
    tkTextSetCursor .console end
    .console mark set promptEnd insert
    .console mark gravity promptEnd left
}

# tkConsoleBind --
# This procedure first ensures that the default bindings for the Text
# class have been defined.  Then certain bindings are overridden for
# the class.
#
# Arguments:
# None.

proc tkConsoleBind {win} {
    catch {tkTextBind dummy_arg}
    
    bindtags $win "$win Text . all"

    bind $win <Return> {
	%W mark set insert {end - 1c}
	tkTextInsert %W "\n"
	tkConsoleInvoke
	break
    }
    bind $win <Delete> {
	if {[%W tag nextrange sel 1.0 end] != ""} {
	    %W tag remove sel sel.first promptEnd
	} else {
	    if [%W compare insert < promptEnd] {
		break
	    }
	}
    }
    bind $win <BackSpace> {
	if {[%W tag nextrange sel 1.0 end] != ""} {
	    %W tag remove sel sel.first promptEnd
	} else {
	    if [%W compare insert <= promptEnd] {
		break
	    }
	}
    }
    bind $win <Control-a> {
	if [%W compare insert < promptEnd] {
	    tkTextSetCursor %W {insert linestart}
	} else {
	    tkTextSetCursor %W promptEnd
        }
	break
    }
    bind $win <Control-d> {
	if [%W compare insert < promptEnd] {
	    break
	}
    }
    bind $win <Control-k> {
	if [%W compare insert < promptEnd] {
	    %W mark set insert promptEnd
	}
    }
    bind $win <Control-t> {
	if [%W compare insert < promptEnd] {
	    break
	}
    }
    bind $win <Meta-d> {
	if [%W compare insert < promptEnd] {
	    break
	}
    }
    bind $win <Meta-BackSpace> {
	if [%W compare insert <= promptEnd] {
	    break
	}
    }
    bind $win <Control-h> {
	if [%W compare insert <= promptEnd] {
	    break
	}
    }
    bind $win <Control-p> {
	tkConsoleHistory prev
	break
    }
    bind $win <Control-n> {
	tkConsoleHistory next
	break
    }
    bind $win <Control-v> {
	if [%W compare insert > promptEnd] {
	    catch {
		%W insert insert [selection get -displayof %W] {input stdin}
		%W see insert
	    }
	}
	break
    }
    bind $win <F9> {
	eval destroy [winfo child .]
	source $tk_library/console.tcl
    }
    foreach copy {F16 Meta-w Control-i} {
	bind Text <$copy> {
	    if {[selection own -displayof %W] == "%W"} {
		clipboard clear -displayof %W
		catch {
		    clipboard append -displayof %W [selection get -displayof %W]
		}
	    }
	    break
	}
    }
    foreach paste {F18 Control-y} {
	bind $win <$paste> {
	    catch {
	        set clip [selection get -displayof %W -selection CLIPBOARD]
		set list [split $clip \n\r]
		tkTextInsert %W [lindex $list 0]
		foreach x [lrange $list 1 end] {
		    %W mark set insert {end - 1c}
		    tkTextInsert %W "\n"
		    tkConsoleInvoke
		    tkTextInsert %W $x
		}
	    }
	    break
	}
    }
}

# Replace the default implementation of tkTextInsert so that we can
# attach tags to user input

proc tkTextInsert {w s} {
    if {$s == ""} {
	return
    }
    catch {
	if {[$w compare sel.first <= insert]
		&& [$w compare sel.last >= insert]} {
	    $w tag remove sel sel.first promptEnd
	    $w delete sel.first sel.last
	}
    }
    if {[$w compare insert < promptEnd]} {
	$w mark set insert end	
    }
    $w insert insert $s {input stdin}
    $w see insert
}

# tkConsoleOutput --
#
# This routine is called directly by ConsolePutsCmd to cause a string
# to be displayed in the console.
#
# Arguments:
# dest -	The output tag to be used: either "stderr" or "stdout".
# string -	The string to be displayed.

proc tkConsoleOutput {dest string} {
    .console insert output $string $dest
    .console see insert
}

# tkConsoleExit --
#
# This routine is called by ConsoleEventProc when the main window of
# the application is destroyed.
#
# Arguments:
# None.

proc tkConsoleExit {} {
    exit
}

# now initialize the console

tkConsoleInit
