# GDB GUI setup for GDB, the GNU debugger.
# Copyright 1994, 1995, 1996
# Free Software Foundation, Inc.

# Written by Stu Grossman <grossman@cygnus.com> of Cygnus Support.

# This file is part of GDB.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

set cfile Blank
set wins($cfile) .src.text
set current_label {}
set cfunc NIL
set line_numbers 1
set breakpoint_file(-1) {[garbage]}
set disassemble_with_source nosource
set expr_update_list(0) 0

#option add *Foreground Black
#option add *Background White
#option add *Font -*-*-medium-r-normal--18-*-*-*-m-*-*-1

proc echo string {puts stdout $string}

# Assign elements from LIST to variables named in ARGS.  FIXME replace
# with TclX version someday.
proc lassign {list args} {
  set len [expr {[llength $args] - 1}]
  while {$len >= 0} {
    upvar [lindex $args $len] local
    set local [lindex $list $len]
    decr len
  }
}

#
# Local procedure:
#
#	decr (var val) - compliment to incr
#
# Description:
#
#
proc decr {var {val 1}} {
  upvar $var num
  set num [expr {$num - $val}]
  return $num
}

#
# Center a window on the screen.
#
proc center_window toplevel {
  # Withdraw and update, to ensure geometry computations are finished.
  wm withdraw $toplevel
  update idletasks

  set x [expr {[winfo screenwidth $toplevel] / 2
	       - [winfo reqwidth $toplevel] / 2
	       - [winfo vrootx $toplevel]}]
  set y [expr {[winfo screenheight $toplevel] / 2
	       - [winfo reqheight $toplevel] / 2
	       - [winfo vrooty $toplevel]}]
  wm geometry $toplevel +${x}+${y}
  wm deiconify $toplevel
}

#
# Rearrange the bindtags so the widget comes after the class.  I was
# always for Ousterhout putting the class bindings first, but no...
#
proc bind_widget_after_class {widget} {
  set class [winfo class $widget]
  set newList {}
  foreach tag [bindtags $widget] {
    if {$tag == $widget} {
      # Nothing.
    } {
      lappend newList $tag
      if {$tag == $class} {
	lappend newList $widget
      }
    }
  }
  bindtags $widget $newList
}

if {[info exists env(EDITOR)]} then {
  set editor $env(EDITOR)
} else {
  set editor emacs
}

# GDB callbacks
#
#  These functions are called by GDB (from C code) to do various things in
#  TK-land.  All start with the prefix `gdbtk_tcl_' to make them easy to find.
#

#
# GDB Callback:
#
#	gdbtk_tcl_fputs (text) - Output text to the command window
#
# Description:
#
#	GDB calls this to output TEXT to the GDB command window.  The text is
#	placed at the end of the text widget.  Note that output may not occur,
#	due to buffering.  Use gdbtk_tcl_flush to cause an immediate update.
#

proc gdbtk_tcl_fputs {arg} {
  .cmd.text insert end "$arg"
  .cmd.text see end
}

proc gdbtk_tcl_fputs_error {arg} {
  .cmd.text insert end "$arg"
  .cmd.text see end
}

#
# GDB Callback:
#
#	gdbtk_tcl_flush () - Flush output to the command window
#
# Description:
#
#	GDB calls this to force all buffered text to the GDB command window.
#

proc gdbtk_tcl_flush {} {
  .cmd.text see end
  update idletasks
}

#
# GDB Callback:
#
#	gdbtk_tcl_query (message) - Create a yes/no query dialog box
#
# Description:
#
#	GDB calls this to create a yes/no dialog box containing MESSAGE.  GDB
#	is hung while the dialog box is active (ie: no commands will work),
#	however windows can still be refreshed in case of damage or exposure.
#

proc gdbtk_tcl_query {message} {
  # FIXME We really want a Help button here.  But Tk's brain-damaged
  # modal dialogs won't really allow it.  Should have async dialog
  # here.
  set result [tk_dialog .query "gdb : query" "$message" questhead 0 Yes No]
  return [expr {!$result}]
}

#
# GDB Callback:
#
#	gdbtk_start_variable_annotation (args ...) - 
#
# Description:
#
#	Not yet implemented.
#

proc gdbtk_tcl_start_variable_annotation {valaddr ref_type stor_cl
					  cum_expr field type_cast} {
  echo "gdbtk_tcl_start_variable_annotation $valaddr $ref_type $stor_cl $cum_expr $field $type_cast"
}

#
# GDB Callback:
#
#	gdbtk_end_variable_annotation (args ...) - 
#
# Description:
#
#	Not yet implemented.
#

proc gdbtk_tcl_end_variable_annotation {} {
	echo gdbtk_tcl_end_variable_annotation
}

#
# GDB Callback:
#
#	gdbtk_tcl_breakpoint (action bpnum file line) - Notify the TK
#	interface of changes to breakpoints.
#
# Description:
#
#	GDB calls this to notify TK of changes to breakpoints.  ACTION is one
#	of:
#		create		- Notify of breakpoint creation
#		delete		- Notify of breakpoint deletion
#		modify		- Notify of breakpoint modification
#

# file line pc type enabled disposition silent ignore_count commands cond_string thread hit_count

proc gdbtk_tcl_breakpoint {action bpnum} {
	set bpinfo [gdb_get_breakpoint_info $bpnum]
	set file [lindex $bpinfo 0]
	set line [lindex $bpinfo 1]
	set pc [lindex $bpinfo 2]
	set enable [lindex $bpinfo 4]

	if {$action == "modify"} {
		if {$enable == "1"} {
			set action enable
		} else {
			set action disable
		}
	}

	${action}_breakpoint $bpnum $file $line $pc
}

proc create_breakpoints_window {} {
	global bpframe_lasty

	if {[winfo exists .breakpoints]} {raise .breakpoints ; return}

	build_framework .breakpoints "Breakpoints" ""

# First, delete all the old view menu entries

	.breakpoints.menubar.view.menu delete 0 last

# Get rid of label

	destroy .breakpoints.label

# Replace text with a canvas and fix the scrollbars

	destroy .breakpoints.text
	scrollbar .breakpoints.scrollx -orient horizontal \
		-command {.breakpoints.c xview} -relief sunken
	canvas .breakpoints.c -relief sunken -bd 2 \
		-cursor hand2 \
	        -yscrollcommand {.breakpoints.scroll set} \
	        -xscrollcommand {.breakpoints.scrollx set}
	.breakpoints.scroll configure -command {.breakpoints.c yview}

	pack .breakpoints.scrollx -side bottom -fill x -in .breakpoints.info
	pack .breakpoints.c -side left -expand yes -fill both \
		-in .breakpoints.info

	set bpframe_lasty 0

# Create a frame for each breakpoint

	foreach bpnum [gdb_get_breakpoint_list] {
		add_breakpoint_frame $bpnum
	}
}

# Create a frame for bpnum in the .breakpoints canvas

proc add_breakpoint_frame bpnum {
  global bpframe_lasty
  global enabled
  global disposition

  if {![winfo exists .breakpoints]} return

  set bpinfo [gdb_get_breakpoint_info $bpnum]

  lassign $bpinfo file line pc type enabled($bpnum) disposition($bpnum) \
    silent ignore_count commands cond thread hit_count

  set f .breakpoints.c.$bpnum

  if {![winfo exists $f]} {
    frame $f -relief sunken -bd 2

    label $f.id -text "#$bpnum     $file:$line    ($pc)" \
      -relief flat -bd 2 -anchor w
    frame $f.hit_count
    label $f.hit_count.label -text "Hit count:" -relief flat \
      -bd 2 -anchor w -width 11
    label $f.hit_count.val -text $hit_count -relief flat \
      -bd 2 -anchor w
    checkbutton $f.hit_count.enabled -text Enabled \
      -variable enabled($bpnum) -anchor w -relief flat

    pack $f.hit_count.label $f.hit_count.val -side left
    pack $f.hit_count.enabled -side right

    frame $f.thread
    label $f.thread.label -text "Thread: " -relief flat -bd 2 \
      -width 11 -anchor w
    entry $f.thread.entry -bd 2 -relief sunken -width 10
    $f.thread.entry insert end $thread
    pack $f.thread.label -side left
    pack $f.thread.entry -side left -fill x

    frame $f.cond
    label $f.cond.label -text "Condition: " -relief flat -bd 2 \
      -width 11 -anchor w
    entry $f.cond.entry -bd 2 -relief sunken
    $f.cond.entry insert end $cond
    pack $f.cond.label -side left
    pack $f.cond.entry -side left -fill x -expand yes

    frame $f.ignore_count
    label $f.ignore_count.label -text "Ignore count: " \
      -relief flat -bd 2 -width 11 -anchor w
    entry $f.ignore_count.entry -bd 2 -relief sunken -width 10
    $f.ignore_count.entry insert end $ignore_count
    pack $f.ignore_count.label -side left
    pack $f.ignore_count.entry -side left -fill x

    frame $f.disps

    label $f.disps.label -text "Disposition: " -relief flat -bd 2 \
      -anchor w -width 11

    radiobutton $f.disps.delete -text Delete \
      -variable disposition($bpnum) -anchor w -relief flat \
      -command "gdb_cmd \"delete break $bpnum\"" \
      -value delete

    radiobutton $f.disps.disable -text Disable \
      -variable disposition($bpnum) -anchor w -relief flat \
      -command "gdb_cmd \"disable break $bpnum\"" \
      -value disable

    radiobutton $f.disps.donttouch -text "Leave alone" \
      -variable disposition($bpnum) -anchor w -relief flat \
      -command "gdb_cmd \"enable break $bpnum\"" \
      -value donttouch

    pack $f.disps.label $f.disps.delete $f.disps.disable \
      $f.disps.donttouch -side left -anchor w
    text $f.commands -relief sunken -bd 2 -setgrid true \
      -cursor hand2 -height 3 -width 30

    foreach line $commands {
      $f.commands insert end "${line}\n"
    }

    pack $f.id -side top -anchor nw -fill x
    pack $f.hit_count $f.cond $f.thread $f.ignore_count $f.disps \
      $f.commands -side top -fill x -anchor nw
  }

  set tag [.breakpoints.c create window 0 $bpframe_lasty -window $f -anchor nw]
  update
  set bbox [.breakpoints.c bbox $tag]

  set bpframe_lasty [lindex $bbox 3]

  .breakpoints.c configure -width [lindex $bbox 2]
}

# Delete a breakpoint frame

proc delete_breakpoint_frame bpnum {
	global bpframe_lasty

	if {![winfo exists .breakpoints]} return

# First, clear the canvas

	.breakpoints.c delete all

# Now, repopulate it with all but the doomed breakpoint

	set bpframe_lasty 0
	foreach bp [gdb_get_breakpoint_list] {
		if {$bp != $bpnum} {
			add_breakpoint_frame $bp
		}
	}
}

proc asm_win_name {funcname} {
	if {$funcname == "*None*"} {return .asm.text}

	regsub -all {\.} $funcname _ temp

	return .asm.func_${temp}
}

#
# Local procedure:
#
#	create_breakpoint (bpnum file line pc) - Record breakpoint info in TK land
#
# Description:
#
#	GDB calls this indirectly (through gdbtk_tcl_breakpoint) to notify TK
#	land of breakpoint creation.  This consists of recording the file and
#	line number in the breakpoint_file and breakpoint_line arrays.  Also,
#	if there is already a window associated with FILE, it is updated with
#	a breakpoint tag.
#

proc create_breakpoint {bpnum file line pc} {
	global wins
	global breakpoint_file
	global breakpoint_line
	global pos_to_breakpoint
	global pos_to_bpcount
	global cfunc
	global pclist

# Record breakpoint locations

	set breakpoint_file($bpnum) $file
	set breakpoint_line($bpnum) $line
	set pos_to_breakpoint($file:$line) $bpnum
	if {![info exists pos_to_bpcount($file:$line)]} {
		set pos_to_bpcount($file:$line) 0
	}
	incr pos_to_bpcount($file:$line)
	set pos_to_breakpoint($pc) $bpnum
	if {![info exists pos_to_bpcount($pc)]} {
		set pos_to_bpcount($pc) 0
	}
	incr pos_to_bpcount($pc)
	
# If there's a window for this file, update it

	if {[info exists wins($file)]} {
		insert_breakpoint_tag $wins($file) $line
	}

# If there's an assembly window, update that too

	set win [asm_win_name $cfunc]
	if {[winfo exists $win]} {
		insert_breakpoint_tag $win [pc_to_line $pclist($cfunc) $pc]
	}

# Update the breakpoints window

	add_breakpoint_frame $bpnum
}

#
# Local procedure:
#
#	delete_breakpoint (bpnum file line pc) - Delete breakpoint info from TK land
#
# Description:
#
#	GDB calls this indirectly (through gdbtk_tcl_breakpoint) to notify TK
#	land of breakpoint destruction.  This consists of removing the file and
#	line number from the breakpoint_file and breakpoint_line arrays.  Also,
#	if there is already a window associated with FILE, the tags are removed
#	from it.
#

proc delete_breakpoint {bpnum file line pc} {
	global wins
	global breakpoint_file
	global breakpoint_line
	global pos_to_breakpoint
	global pos_to_bpcount
	global cfunc pclist

# Save line number and file for later

	set line $breakpoint_line($bpnum)

	set file $breakpoint_file($bpnum)

# Reset breakpoint annotation info

	if {$pos_to_bpcount($file:$line) > 0} {
		decr pos_to_bpcount($file:$line)

		if {$pos_to_bpcount($file:$line) == 0} {
			catch "unset pos_to_breakpoint($file:$line)"

			unset breakpoint_file($bpnum)
			unset breakpoint_line($bpnum)

# If there's a window for this file, update it

			if {[info exists wins($file)]} {
				delete_breakpoint_tag $wins($file) $line
			}
		}
	}

# If there's an assembly window, update that too

	if {$pos_to_bpcount($pc) > 0} {
		decr pos_to_bpcount($pc)

		if {$pos_to_bpcount($pc) == 0} {
			catch "unset pos_to_breakpoint($pc)"

			set win [asm_win_name $cfunc]
			if {[winfo exists $win]} {
				delete_breakpoint_tag $win [pc_to_line $pclist($cfunc) $pc]
			}
		}
	}

	delete_breakpoint_frame $bpnum
}

#
# Local procedure:
#
#	enable_breakpoint (bpnum file line pc) - Record breakpoint info in TK land
#
# Description:
#
#	GDB calls this indirectly (through gdbtk_tcl_breakpoint) to notify TK
#	land of a breakpoint being enabled.  This consists of unstippling the
#	specified breakpoint indicator.
#

proc enable_breakpoint {bpnum file line pc} {
	global wins
	global cfunc pclist
	global enabled

	if {[info exists wins($file)]} {
		$wins($file) tag configure $line -fgstipple {}
	}

# If there's an assembly window, update that too

	set win [asm_win_name $cfunc]
	if {[winfo exists $win]} {
		$win tag configure [pc_to_line $pclist($cfunc) $pc] -fgstipple {}
	}

# If there's a breakpoint window, update that too

	if {[winfo exists .breakpoints]} {
		set enabled($bpnum) 1
	}
}

#
# Local procedure:
#
#	disable_breakpoint (bpnum file line pc) - Record breakpoint info in TK land
#
# Description:
#
#	GDB calls this indirectly (through gdbtk_tcl_breakpoint) to notify TK
#	land of a breakpoint being disabled.  This consists of stippling the
#	specified breakpoint indicator.
#

proc disable_breakpoint {bpnum file line pc} {
	global wins
	global cfunc pclist
	global enabled

	if {[info exists wins($file)]} {
		$wins($file) tag configure $line -fgstipple gray50
	}

# If there's an assembly window, update that too

	set win [asm_win_name $cfunc]
	if {[winfo exists $win]} {
		$win tag configure [pc_to_line $pclist($cfunc) $pc] -fgstipple gray50
	}

# If there's a breakpoint window, update that too

	if {[winfo exists .breakpoints]} {
		set enabled($bpnum) 0
	}
}

#
# Local procedure:
#
#	insert_breakpoint_tag (win line) - Insert a breakpoint tag in WIN.
#
# Description:
#
#	GDB calls this indirectly (through gdbtk_tcl_breakpoint) to insert a
#	breakpoint tag into window WIN at line LINE.
#

proc insert_breakpoint_tag {win line} {
	$win configure -state normal
	$win delete $line.0
	$win insert $line.0 "B"
	$win tag add $line $line.0
	$win tag add delete $line.0 "$line.0 lineend"
	$win tag add margin $line.0 "$line.0 lineend"

	$win configure -state disabled
}

#
# Local procedure:
#
#	delete_breakpoint_tag (win line) - Remove a breakpoint tag from WIN.
#
# Description:
#
#	GDB calls this indirectly (through gdbtk_tcl_breakpoint) to remove a
#	breakpoint tag from window WIN at line LINE.
#

proc delete_breakpoint_tag {win line} {
	$win configure -state normal
	$win delete $line.0
	if {[string range $win 0 3] == ".src"} then {
		$win insert $line.0 "\xa4"
	} else {
		$win insert $line.0 " "
	}
	$win tag delete $line
	$win tag add delete $line.0 "$line.0 lineend"
	$win tag add margin $line.0 "$line.0 lineend"
	$win configure -state disabled
}

proc gdbtk_tcl_busy {} {
	if {[winfo exists .src]} {
		.src.start configure -state disabled
		.src.stop configure -state normal
		.src.step configure -state disabled
		.src.next configure -state disabled
		.src.continue configure -state disabled
		.src.finish configure -state disabled
		.src.up configure -state disabled
		.src.down configure -state disabled
		.src.bottom configure -state disabled
	}
	if {[winfo exists .asm]} {
		.asm.stepi configure -state disabled
		.asm.nexti configure -state disabled
		.asm.continue configure -state disabled
		.asm.finish configure -state disabled
		.asm.up configure -state disabled
		.asm.down configure -state disabled
		.asm.bottom configure -state disabled
	}
	return
}

proc gdbtk_tcl_idle {} {
	if {[winfo exists .src]} {
		.src.start configure -state normal
		.src.stop configure -state disabled
		.src.step configure -state normal
		.src.next configure -state normal
		.src.continue configure -state normal
		.src.finish configure -state normal
		.src.up configure -state normal
		.src.down configure -state normal
		.src.bottom configure -state normal
	}

	if {[winfo exists .asm]} {
		.asm.stepi configure -state normal
		.asm.nexti configure -state normal
		.asm.continue configure -state normal
		.asm.finish configure -state normal
		.asm.up configure -state normal
		.asm.down configure -state normal
		.asm.bottom configure -state normal
	}
	return
}

#
# Local procedure:
#
#	pc_to_line (pclist pc) - convert PC to a line number.
#
# Description:
#
#	Convert PC to a line number from PCLIST.  If exact line isn't found,
#	we return the first line that starts before PC.
#
proc pc_to_line {pclist pc} {
	set line [lsearch -exact $pclist $pc]

	if {$line >= 1} { return $line }

	set line 1
	foreach linepc [lrange $pclist 1 end] {
		if {$pc < $linepc} { decr line ; return $line }
		incr line
	}
	return [expr {$line - 1}]
}

#
# Menu:
#
#	file popup menu - Define the file popup menu.
#
# Description:
#
#	This menu just contains a bunch of buttons that do various things to
#	the line under the cursor.
#
# Items:
#
#	Edit - Run the editor (specified by the environment variable EDITOR) on
#	       this file, at the current line.
#	Breakpoint - Set a breakpoint at the current line.  This just shoves
#		a `break' command at GDB with the appropriate file and line
#		number.  Eventually, GDB calls us back (at gdbtk_tcl_breakpoint)
#		to notify us of where the breakpoint needs to show up.
#

menu .file_popup -cursor hand2 -tearoff 0
.file_popup add command -label "Not yet set" -state disabled
.file_popup add separator
.file_popup add command -label "Edit" \
  -command {exec $editor +$selected_line $selected_file &}
.file_popup add command -label "Set breakpoint" \
  -command {gdb_cmd "break $selected_file:$selected_line"}

# Use this procedure to get the GDB core to execute the string `cmd'.  This is
# a wrapper around gdb_cmd, which will catch errors, and send output to the
# command window.  It will also cause all of the other windows to be updated.

proc interactive_cmd {cmd} {
	catch {gdb_cmd "$cmd"} result
	.cmd.text insert end $result
	.cmd.text see end
	update_ptr
}

#
# Bindings:
#
#	file popup menu - Define the file popup menu bindings.
#
# Description:
#
#	This defines the binding for the file popup menu.  It simply
#       unhighlights the line under the cursor.
#

bind .file_popup <Any-ButtonRelease-1> {
  global selected_win
  # Unhighlight the selected line
  $selected_win tag delete breaktag
}

#
# Local procedure:
#
#	file_popup_menu (win x y xrel yrel) - Popup the file popup menu.
#
# Description:
#
#	This procedure is invoked as a result of a command binding in the
#	listing window.  It does several things:
#		o - It highlights the line under the cursor.
#		o - It pops up the file popup menu which is intended to do
#		    various things to the aforementioned line.
#		o - Grabs the mouse for the file popup menu.
#

# Button 1 has been pressed in a listing window.  Pop up a menu.

proc file_popup_menu {win x y xrel yrel} {
	global wins
	global win_to_file
	global file_to_debug_file
	global highlight
	global selected_line
	global selected_file
	global selected_win

# Map TK window name back to file name.

	set file $win_to_file($win)

	set pos [$win index @$xrel,$yrel]

# Record selected file and line for menu button actions

	set selected_file $file_to_debug_file($file)
	set selected_line [lindex [split $pos .] 0]
	set selected_win $win

# Highlight the selected line

	eval $win tag config breaktag $highlight
	$win tag add breaktag "$pos linestart" "$pos linestart + 1l"

# Post the menu near the pointer, (and grab it)

	.file_popup entryconfigure 0 -label "$selected_file:$selected_line"
	tk_popup .file_popup $x $y
}

#
# Local procedure:
#
#	listing_window_button_1 (win x y xrel yrel) - Handle button 1 in listing window
#
# Description:
#
#	This procedure is invoked as a result of holding down button 1 in the
#	listing window.  The action taken depends upon where the button was
#	pressed.  If it was in the left margin (the breakpoint column), it
#	sets or clears a breakpoint.  In the main text area, it will pop up a
#	menu.
#

proc listing_window_button_1 {win x y xrel yrel} {
	global wins
	global win_to_file
	global file_to_debug_file
	global highlight
	global selected_line
	global selected_file
	global selected_win
	global pos_to_breakpoint

# Map TK window name back to file name.

	set file $win_to_file($win)

	set pos [split [$win index @$xrel,$yrel] .]

# Record selected file and line for menu button actions

	set selected_file $file_to_debug_file($file)
	set selected_line [lindex $pos 0]
	set selected_col [lindex $pos 1]
	set selected_win $win

# If we're in the margin, then toggle the breakpoint

	if {$selected_col < 8} {
		set pos_break $selected_file:$selected_line
		set pos $file:$selected_line
		set tmp pos_to_breakpoint($pos)
		if {[info exists $tmp]} {
			set bpnum [set $tmp]
			gdb_cmd "delete	$bpnum"
		} else {
			gdb_cmd "break $pos_break"
		}
		return
	}

# Post the menu near the pointer, (and grab it)

	.file_popup entryconfigure 0 -label "$selected_file:$selected_line"

        tk_popup .file_popup $x $y
}

#
# Local procedure:
#
#	asm_window_button_1 (win x y xrel yrel) - Handle button 1 in asm window
#
# Description:
#
#	This procedure is invoked as a result of holding down button 1 in the
#	assembly window.  The action taken depends upon where the button was
#	pressed.  If it was in the left margin (the breakpoint column), it
#	sets or clears a breakpoint.  In the main text area, it will pop up a
#	menu.
#

proc asm_window_button_1 {win x y xrel yrel} {
	global wins
	global win_to_file
	global file_to_debug_file
	global highlight
	global selected_line
	global selected_file
	global selected_win
	global pos_to_breakpoint
	global pclist
	global cfunc

	set pos [split [$win index @$xrel,$yrel] .]

# Record selected file and line for menu button actions

	set selected_line [lindex $pos 0]
	set selected_col [lindex $pos 1]
	set selected_win $win

# Figure out the PC

	set pc [lindex $pclist($cfunc) $selected_line]

# If we're in the margin, then toggle the breakpoint

	if {$selected_col < 11} {
		set tmp pos_to_breakpoint($pc)
		if {[info exists $tmp]} {
			set bpnum [set $tmp]
			gdb_cmd "delete	$bpnum"
		} else {
			gdb_cmd "break *$pc"
		}
		return
	}

# Post the menu near the pointer, (and grab it)

#	.file_popup entryconfigure 0 -label "$selected_file:$selected_line"
#	.file_popup post [expr $x-[winfo width .file_popup]/2] [expr $y-10]
#	grab .file_popup
}

#
# Local procedure:
#
#	do_nothing - Does absolutely nothing.
#
# Description:
#
#	This procedure does nothing.  It is used as a placeholder to allow
#	the disabling of bindings that would normally be inherited from the
#	parent widget.  I can't think of any other way to do this.
#

proc do_nothing {} {}

#
# Local procedure:
#
#	not_implemented_yet - warn that a feature is unavailable
#
# Description:
#
#	This procedure warns that something doesn't actually work yet.
#

proc not_implemented_yet {message} {
	tk_dialog .unimpl "gdb : unimpl" \
		"$message: not implemented in the interface yet" \
		warning 0 "OK"
}

##
# Local procedure:
#
#	create_expr_window - Create expression display window
#
# Description:
#
#	Create the expression display window.
#

set expr_num 0
set delete_expr_num 0

# Set delete_expr_num, and set -state of Delete button.
proc expr_update_button {num} {
  global delete_expr_num
  set delete_expr_num $num
  if {$num > 0} then {
    set state normal
  } else {
    set state disabled
  }
  .expr.buts.delete configure -state $state
}

proc add_expr {expr} {
  global expr_update_list
  global expr_num

  incr expr_num

  set e .expr.exprs
  set f e$expr_num

  checkbutton $e.updates.$f -text "" -relief flat \
    -variable expr_update_list($expr_num)
  text $e.expressions.$f -width 20 -height 1
  $e.expressions.$f insert 0.0 $expr
  bind $e.expressions.$f <1> "update_expr $expr_num"
  text $e.values.$f -width 20 -height 1

  # Set up some bindings.
  foreach frame {updates expressions values} {
    bind $e.$frame.$f <FocusIn> "expr_update_button $expr_num"
    bind $e.$frame.$f <FocusOut> "expr_update_button 0"
  }

  update_expr $expr_num

  pack $e.updates.$f -side top
  pack $e.expressions.$f -side top -expand yes -fill x
  pack $e.values.$f -side top -expand yes -fill x
}

proc delete_expr {} {
  global delete_expr_num
  if {$delete_expr_num > 0} then {
    set e .expr.exprs
    set f e${delete_expr_num}

    destroy $e.updates.$f $e.expressions.$f $e.values.$f

    # FIXME should we unset an element of expr_update_list here?
  }
}

proc update_expr {expr_num} {
  global expr_update_list

  set e .expr.exprs
  set f e${expr_num}

  set expr [$e.expressions.$f get 0.0 end]
  $e.values.$f delete 0.0 end
  if {! [catch {gdb_eval $expr} val]} {
    $e.values.$f insert 0.0 $val
  } {
    # FIXME consider flashing widget here.
  }
}

proc update_exprs {} {
	global expr_update_list

	foreach expr_num [array names expr_update_list] {
		if {$expr_update_list($expr_num)} {
			update_expr $expr_num
		}
	}
}

proc create_expr_window {} {

	if {[winfo exists .expr]} {raise .expr ; return}

	toplevel .expr
	wm title .expr "GDB Expressions"
	wm iconname .expr "Expressions"

	frame .expr.entryframe -borderwidth 2 -relief raised
	label .expr.entryframe.entrylab -text "Expression: "
	entry .expr.entryframe.entry -borderwidth 2 -relief sunken
	bind .expr.entryframe.entry <Return> {
	  add_expr [.expr.entryframe.entry get]
	  .expr.entryframe.entry delete 0 end
	}

	pack .expr.entryframe.entrylab -side left
	pack .expr.entryframe.entry -side left -fill x -expand yes

	frame .expr.buts -borderwidth 2 -relief raised

	button .expr.buts.delete -text Delete -command delete_expr \
	  -state disabled

	button .expr.buts.close -text Close -command {destroy .expr}
	button .expr.buts.help -text Help -state disabled

	pack .expr.buts.delete -side left
	pack .expr.buts.help .expr.buts.close -side right

	pack .expr.buts -side bottom -fill x
	pack .expr.entryframe -side bottom -fill x

	frame .expr.exprs -borderwidth 2 -relief raised

	# Use three subframes so columns will line up.  Easier than
	# dealing with BLT for a table geometry manager.  Someday Tk
	# will have one, use it then.  FIXME this messes up keyboard
	# traversal.
	frame .expr.exprs.updates -borderwidth 0 -relief flat
	frame .expr.exprs.expressions -borderwidth 0 -relief flat
	frame .expr.exprs.values -borderwidth 0 -relief flat

	label .expr.exprs.updates.label -text Update
	pack .expr.exprs.updates.label -side top -anchor w
	label .expr.exprs.expressions.label -text Expression
	pack .expr.exprs.expressions.label -side top -anchor w
	label .expr.exprs.values.label -text Value
	pack .expr.exprs.values.label -side top -anchor w

	pack .expr.exprs.updates -side left
	pack .expr.exprs.values .expr.exprs.expressions \
	  -side right -expand 1 -fill x

	pack .expr.exprs -side top -fill both -expand 1 -anchor w
}

#
# Local procedure:
#
#	display_expression (expression) - Display EXPRESSION in display window
#
# Description:
#
#	Display EXPRESSION and its value in the expression display window.
#

proc display_expression {expression} {
	create_expr_window

	add_expr $expression
}

#
# Local procedure:
#
#	create_file_win (filename) - Create a win for FILENAME.
#
# Return value:
#
#	The new text widget.
#
# Description:
#
#	This procedure creates a text widget for FILENAME.  It returns the
#	newly created widget.  First, a text widget is created, and given basic
#	configuration info.  Second, all the bindings are setup.  Third, the
#	file FILENAME is read into the text widget.  Fourth, margins and line
#	numbers are added.
#

proc create_file_win {filename debug_file} {
	global breakpoint_file
	global breakpoint_line
	global line_numbers

# Replace all the dirty characters in $filename with clean ones, and generate
# a unique name for the text widget.

	regsub -all {\.} $filename {} temp
	set win .src.text$temp

# Open the file, and read it into the text widget

	if {[catch "open $filename" fh]} {
# File can't be read.  Put error message into .src.nofile window and return.

		catch {destroy .src.nofile}
		text .src.nofile -height 25 -width 88 -relief sunken \
			-borderwidth 2 -yscrollcommand ".src.scroll set" \
			-setgrid true -cursor hand2
		.src.nofile insert 0.0 $fh
		.src.nofile configure -state disabled
		bind .src.nofile <1> do_nothing
		bind .src.nofile <B1-Motion> do_nothing
		return .src.nofile
	}

# Actually create and do basic configuration on the text widget.

	text $win -height 25 -width 88 -relief sunken -borderwidth 2 \
		-yscrollcommand ".src.scroll set" -setgrid true -cursor hand2

# Setup all the bindings

	bind $win <Enter> {focus %W}
	bind $win <1> do_nothing
	bind $win <B1-Motion> do_nothing

	bind $win <Key-Alt_R> do_nothing
	bind $win <Key-Alt_L> do_nothing
	bind $win <Key-Prior> "$win yview {@0,0 - 10 lines}"
	bind $win <Key-Next> "$win yview {@0,0 + 10 lines}"
	bind $win <Key-Up> "$win yview {@0,0 - 1 lines}"
	bind $win <Key-Down> "$win yview {@0,0 + 1 lines}"
	bind $win <Key-Home> {update_listing [gdb_loc]}
	bind $win <Key-End> "$win see end"

	bind $win n {interactive_cmd next}
	bind $win s {interactive_cmd step}
	bind $win c {interactive_cmd continue}
	bind $win f {interactive_cmd finish}
	bind $win u {interactive_cmd up}
	bind $win d {interactive_cmd down}

	$win delete 0.0 end
	$win insert 0.0 [read $fh]
	close $fh

# Add margins (for annotations) and a line number to each line (if requested)

	set numlines [$win index end]
	set numlines [lindex [split $numlines .] 0]
	if {$line_numbers} {
		for {set i 1} {$i <= $numlines} {incr i} {
			$win insert $i.0 [format "   %4d " $i]
			$win tag add source $i.8 "$i.0 lineend"
			}
	} else {
		for {set i 1} {$i <= $numlines} {incr i} {
			$win insert $i.0 "        "
			$win tag add source $i.8 "$i.0 lineend"
			}
	}

# Add the breakdots

	foreach i [gdb_sourcelines $debug_file] {
		$win delete $i.0
		$win insert $i.0 "\xa4"
		$win tag add margin $i.0 $i.8
		}

	$win tag bind margin <1> {listing_window_button_1 %W %X %Y %x %y}
	$win tag bind source <1> {
		%W mark set anchor "@%x,%y wordstart"
		set last [%W index "@%x,%y wordend"]
		%W tag remove sel 0.0 anchor
		%W tag remove sel $last end
		%W tag add sel anchor $last
		}
#	$win tag bind source <Double-Button-1> {
#		%W mark set anchor "@%x,%y wordstart"
#		set last [%W index "@%x,%y wordend"]
#		%W tag remove sel 0.0 anchor
#		%W tag remove sel $last end
#		%W tag add sel anchor $last
#		echo "Selected [selection get]"
#		}
	$win tag bind source <B1-Motion> {
		%W tag remove sel 0.0 anchor
		%W tag remove sel $last end
		%W tag add sel anchor @%x,%y
		}
	$win tag bind sel <1> do_nothing
	$win tag bind sel <Double-Button-1> {display_expression [selection get]}
	$win tag raise sel


# Scan though the breakpoint data base and install any destined for this file

	foreach bpnum [array names breakpoint_file] {
		if {$breakpoint_file($bpnum) == $filename} {
			insert_breakpoint_tag $win $breakpoint_line($bpnum)
			}
		}

# Disable the text widget to prevent user modifications

	$win configure -state disabled
	return $win
}

#
# Local procedure:
#
#	create_asm_win (funcname pc) - Create an assembly win for FUNCNAME.
#
# Return value:
#
#	The new text widget.
#
# Description:
#
#	This procedure creates a text widget for FUNCNAME.  It returns the
#	newly created widget.  First, a text widget is created, and given basic
#	configuration info.  Second, all the bindings are setup.  Third, the
#	function FUNCNAME is read into the text widget.
#

proc create_asm_win {funcname pc} {
	global breakpoint_file
	global breakpoint_line
	global pclist
	global disassemble_with_source

# Replace all the dirty characters in $filename with clean ones, and generate
# a unique name for the text widget.

	set win [asm_win_name $funcname]

# Actually create and do basic configuration on the text widget.

	text $win -height 25 -width 80 -relief sunken -borderwidth 2 \
		-setgrid true -cursor hand2 -yscrollcommand ".asm.scroll set"

# Setup all the bindings

	bind $win <Enter> {focus %W}
	bind $win <1> {asm_window_button_1 %W %X %Y %x %y}
	bind $win <B1-Motion> do_nothing

	bind $win <Key-Alt_R> do_nothing
	bind $win <Key-Alt_L> do_nothing

	bind $win n {interactive_cmd nexti}
	bind $win s {interactive_cmd stepi}
	bind $win c {interactive_cmd continue}
	bind $win f {interactive_cmd finish}
	bind $win u {interactive_cmd up}
	bind $win d {interactive_cmd down}

# Disassemble the code, and read it into the new text widget

	$win insert end [gdb_disassemble $disassemble_with_source $pc]

	set numlines [$win index end]
	set numlines [lindex [split $numlines .] 0]
	decr numlines

# Delete the first and last lines, cuz these contain useless info

#	$win delete 1.0 2.0
#	$win delete {end - 1 lines} end
#	decr numlines 2

# Add margins (for annotations) and note the PC for each line

	catch "unset pclist($funcname)"
	lappend pclist($funcname) Unused
	for {set i 1} {$i <= $numlines} {incr i} {
		scan [$win get $i.0 "$i.0 lineend"] "%s " pc
		lappend pclist($funcname) $pc
		$win insert $i.0 "    "
		}

# Scan though the breakpoint data base and install any destined for this file

#	foreach bpnum [array names breakpoint_file] {
#		if {$breakpoint_file($bpnum) == $filename} {
#			insert_breakpoint_tag $win $breakpoint_line($bpnum)
#			}
#		}

# Disable the text widget to prevent user modifications

	$win configure -state disabled
	return $win
}

#
# Local procedure:
#
#	update_listing (linespec) - Update the listing window according to
#				    LINESPEC.
#
# Description:
#
#	This procedure is called from various places to update the listing
#	window based on LINESPEC.  It is usually invoked with the result of
#	gdb_loc.
#
#	It will move the cursor, and scroll the text widget if necessary.
#	Also, it will switch to another text widget if necessary, and update
#	the label widget too.
#
#	LINESPEC is a list of the form:
#
#	{ DEBUG_FILE FUNCNAME FILENAME LINE }, where:
#
#	DEBUG_FILE - is the abbreviated form of the file name.  This is usually
#		     the file name string given to the cc command.  This is
#		     primarily needed for breakpoint commands, and when an
#		     abbreviated for of the filename is desired.
#	FUNCNAME - is the name of the function.
#	FILENAME - is the fully qualified (absolute) file name.  It is usually
#		   the same as $PWD/$DEBUG_FILE, where PWD is the working dir
#		   at the time the cc command was given.  This is used to
#		   actually locate the file to be displayed.
#	LINE - The line number to be displayed.
#
#	Usually, this procedure will just move the cursor one line down to the
#	next line to be executed.  However, if the cursor moves out of range
#	or into another file, it will scroll the text widget so that the line
#	of interest is in the middle of the viewable portion of the widget.
#

proc update_listing {linespec} {
	global pointers
	global wins cfile
	global current_label
	global win_to_file
	global file_to_debug_file
	global .src.label

# Rip the linespec apart

        lassign $linespec debug_file funcname filename line

# Sometimes there's no source file for this location

	if {$filename == ""} {set filename Blank}

# If we want to switch files, we need to unpack the current text widget, and
# stick in the new one.

	if {$filename != $cfile} then {
		pack forget $wins($cfile)
		set cfile $filename

# Create a text widget for this file if necessary

		if {![info exists wins($cfile)]} then {
			set wins($cfile) [create_file_win $cfile $debug_file]
			if {$wins($cfile) != ".src.nofile"} {
				set win_to_file($wins($cfile)) $cfile
				set file_to_debug_file($cfile) $debug_file
				set pointers($cfile) 1.1
				}
			}

# Pack the text widget into the listing widget, and scroll to the right place

		pack $wins($cfile) -side left -expand yes -in .src.info \
			-fill both -after .src.scroll

# Make the scrollbar point at the new text widget

		.src.scroll configure -command "$wins($cfile) yview"

	        $wins($cfile) see "${line}.0 linestart"
		}

# Update the label widget in case the filename or function name has changed

	if {$current_label != "$filename.$funcname"} then {
		set tail [expr [string last / $filename] + 1]
		set .src.label "[string range $filename $tail end] : ${funcname}()"
#		.src.label configure -text "[string range $filename $tail end] : ${funcname}()"
		set current_label $filename.$funcname
		}

# Update the pointer, scrolling the text widget if necessary to keep the
# pointer in an acceptable part of the screen.

	if {[info exists pointers($cfile)]} then {
		$wins($cfile) configure -state normal
		set pointer_pos $pointers($cfile)
		$wins($cfile) configure -state normal
		$wins($cfile) delete $pointer_pos "$pointer_pos + 2 char"
		$wins($cfile) insert $pointer_pos "  "

		set pointer_pos [$wins($cfile) index $line.1]
		set pointers($cfile) $pointer_pos

		$wins($cfile) delete $pointer_pos "$pointer_pos + 2 char"
		$wins($cfile) insert $pointer_pos "->"
	        $wins($cfile) see "${line}.0 linestart"
		$wins($cfile) configure -state disabled
		}
}

#
# Local procedure:
#
#	create_asm_window - Open up the assembly window.
#
# Description:
#
#	Create an assembly window if it doesn't exist.
#

proc create_asm_window {} {
	global cfunc

	if {[winfo exists .asm]} {raise .asm ; return}

	set cfunc *None*
	set win [asm_win_name $cfunc]

	build_framework .asm Assembly "*NIL*"

# First, delete all the old menu entries

	.asm.menubar.view.menu delete 0 last

	.asm.text configure -yscrollcommand ".asm.scroll set"

	frame .asm.row1
	frame .asm.row2

	button .asm.stepi -width 6 -text Stepi \
		-command {interactive_cmd stepi}
	button .asm.nexti -width 6 -text Nexti \
		-command {interactive_cmd nexti}
	button .asm.continue -width 6 -text Cont \
		-command {interactive_cmd continue}
	button .asm.finish -width 6 -text Finish \
		-command {interactive_cmd finish}
	button .asm.up -width 6 -text Up -command {interactive_cmd up}
	button .asm.down -width 6 -text Down \
		-command {interactive_cmd down}
	button .asm.bottom -width 6 -text Bottom \
		-command {interactive_cmd {frame 0}}

	pack .asm.stepi .asm.continue .asm.up .asm.bottom -side left -padx 3 -pady 5 -in .asm.row1
	pack .asm.nexti .asm.finish .asm.down -side left -padx 3 -pady 5 -in .asm.row2

	pack .asm.row2 .asm.row1 -side bottom -anchor w -before .asm.info

	update

	update_assembly [gdb_loc]

# We do this update_assembly to get the proper value of disassemble-from-exec.

# exec file menu item
	.asm.menubar.view.menu add radiobutton -label "Exec file" \
		-variable disassemble-from-exec -value 1
# target memory menu item
	.asm.menubar.view.menu add radiobutton -label "Target memory" \
		-variable disassemble-from-exec -value 0

# Disassemble with source
	.asm.menubar.view.menu add checkbutton -label "Source" \
		-variable disassemble_with_source -onvalue source \
		-offvalue nosource -command {
			foreach asm [info command .asm.func_*] {
				destroy $asm
				}
			set cfunc NIL
			update_assembly [gdb_loc]
		}
}

proc reg_config_menu {} {
    	catch {destroy .reg.config}
	toplevel .reg.config
	wm geometry .reg.config +300+300
	wm title .reg.config "Register configuration"
	wm iconname .reg.config "Reg config"
	set regnames [gdb_regnames]
	set num_regs [llength $regnames]

	frame .reg.config.buts

	button .reg.config.done -text " Done " -command "
		recompute_reg_display_list $num_regs
		populate_reg_window
		update_registers all
		destroy .reg.config "

	button .reg.config.update -text Update -command "
		recompute_reg_display_list $num_regs
		populate_reg_window
		update_registers all "

	pack .reg.config.buts -side bottom -fill x

	pack .reg.config.done -side left -fill x -expand yes -in .reg.config.buts
	pack .reg.config.update -side right -fill x -expand yes -in .reg.config.buts

# Since there can be lots of registers, we build the window with no more than
# 32 rows, and as many columns as needed.

# First, figure out how many columns we need and create that many column frame
# widgets

	set ncols [expr ($num_regs + 31) / 32]

	for {set col 0} {$col < $ncols} {incr col} {
		frame .reg.config.col$col
		pack .reg.config.col$col -side left -anchor n
	}

# Now, create the checkbutton widgets and pack them in the appropriate columns

	set col 0
	set row 0
	for {set regnum 0} {$regnum < $num_regs} {incr regnum} {
		set regname [lindex $regnames $regnum]
		checkbutton .reg.config.col$col.$row -text $regname -pady 0 \
			-variable regena($regnum) -relief flat -anchor w -bd 1

		pack .reg.config.col$col.$row -side top -fill both

		incr row
		if {$row >= 32} {
			incr col
			set row 0
		}
	}
}

#
# Local procedure:
#
#	create_registers_window - Open up the register display window.
#
# Description:
#
#	Create the register display window, with automatic updates.
#

proc create_registers_window {} {
	global reg_format

	if {[winfo exists .reg]} {raise .reg ; return}

# Create an initial register display list consisting of all registers

	if {![info exists reg_format]} {
		global reg_display_list
		global changed_reg_list
		global regena

		set reg_format {}
		set num_regs [llength [gdb_regnames]]
		for {set regnum 0} {$regnum < $num_regs} {incr regnum} {
			set regena($regnum) 1
		}
		recompute_reg_display_list $num_regs
		set changed_reg_list $reg_display_list
	}

	build_framework .reg Registers

# First, delete all the old menu entries

	.reg.menubar.view.menu delete 0 last

# Hex menu item
	.reg.menubar.view.menu add radiobutton -label Hex \
		-command {set reg_format x ; update_registers all}

# Decimal menu item
	.reg.menubar.view.menu add radiobutton -label Decimal \
		-command {set reg_format d ; update_registers all}

# Octal menu item
	.reg.menubar.view.menu add radiobutton -label Octal \
		-command {set reg_format o ; update_registers all}

# Natural menu item
	.reg.menubar.view.menu add radiobutton -label Natural \
		-command {set reg_format {} ; update_registers all}

# Config menu item
	.reg.menubar.view.menu add separator

	.reg.menubar.view.menu add command -label Config -command {
		reg_config_menu }

	destroy .reg.label

# Install the reg names

	populate_reg_window
	update_registers all
}

# Convert regena into a list of the enabled $regnums

proc recompute_reg_display_list {num_regs} {
	global reg_display_list
	global regmap
	global regena

	catch {unset reg_display_list}

	set line 1
	for {set regnum 0} {$regnum < $num_regs} {incr regnum} {

		if {[set regena($regnum)] != 0} {
			lappend reg_display_list $regnum
			set regmap($regnum) $line
			incr line
		}
	}
}

# Fill out the register window with the names of the regs specified in
# reg_display_list.

proc populate_reg_window {} {
	global max_regname_width
	global reg_display_list

	.reg.text configure -state normal

	.reg.text delete 0.0 end

	set regnames [eval gdb_regnames $reg_display_list]

# Figure out the longest register name

	set max_regname_width 0

	foreach reg $regnames {
		set len [string length $reg]
		if {$len > $max_regname_width} {set max_regname_width $len}
	}

	set width [expr $max_regname_width + 15]

	set height [llength $regnames]

	if {$height > 60} {set height 60}

	.reg.text configure -height $height -width $width

	foreach reg $regnames {
		.reg.text insert end [format "%-*s \n" $max_regname_width ${reg}]
	}

	.reg.text yview 0
	.reg.text configure -state disabled
}

#
# Local procedure:
#
#	update_registers - Update the registers window.
#
# Description:
#
#	This procedure updates the registers window.
#

proc update_registers {which} {
	global max_regname_width
	global reg_format
	global reg_display_list
	global changed_reg_list
	global highlight
	global regmap

	set margin [expr $max_regname_width + 1]
	set win .reg.text
	set winwidth [lindex [$win configure -width] 4]
	set valwidth [expr $winwidth - $margin]

	$win configure -state normal

	if {$which == "all"} {
		set lineindex 1
		foreach regnum $reg_display_list {
			set regval [gdb_fetch_registers $reg_format $regnum]
			set regval [format "%-*s" $valwidth $regval]
			$win delete $lineindex.$margin "$lineindex.0 lineend"
			$win insert $lineindex.$margin $regval
			incr lineindex
		}
		$win configure -state disabled
		return
	}

# Unhighlight the old values

	foreach regnum $changed_reg_list {
		$win tag delete $win.$regnum
	}

# Now, highlight the changed values of the interesting registers

	set changed_reg_list [eval gdb_changed_register_list $reg_display_list]

	set lineindex 1
	foreach regnum $changed_reg_list {
		set regval [gdb_fetch_registers $reg_format $regnum]
		set regval [format "%-*s" $valwidth $regval]

		set lineindex $regmap($regnum)
		$win delete $lineindex.$margin "$lineindex.0 lineend"
		$win insert $lineindex.$margin $regval
		$win tag add $win.$regnum $lineindex.0 "$lineindex.0 lineend"
		eval $win tag configure $win.$regnum $highlight
	}

	$win configure -state disabled
}

#
# Local procedure:
#
#	update_assembly - Update the assembly window.
#
# Description:
#
#	This procedure updates the assembly window.
#

proc update_assembly {linespec} {
	global asm_pointers
	global wins cfunc
	global current_label
	global win_to_file
	global file_to_debug_file
	global current_asm_label
	global pclist
	global .asm.label

# Rip the linespec apart

	lassign $linespec debug_file funcname filename line pc

	set win [asm_win_name $cfunc]

# Sometimes there's no source file for this location

	if {$filename == ""} {set filename Blank}

# If we want to switch funcs, we need to unpack the current text widget, and
# stick in the new one.

	if {$funcname != $cfunc } {
		set oldwin $win
		set cfunc $funcname

		set win [asm_win_name $cfunc]

# Create a text widget for this func if necessary

		if {![winfo exists $win]} {
			create_asm_win $cfunc $pc
			set asm_pointers($cfunc) 1.1
			set current_asm_label NIL
			}

# Pack the text widget, and scroll to the right place

		pack forget $oldwin
		pack $win -side left -expand yes -fill both \
			-after .asm.scroll
		.asm.scroll configure -command "$win yview"
		set line [pc_to_line $pclist($cfunc) $pc]
	        $win see "${line}.0 linestart"
		update
		}

# Update the label widget in case the filename or function name has changed

	if {$current_asm_label != "$pc $funcname"} then {
		set .asm.label "$pc $funcname"
		set current_asm_label "$pc $funcname"
		}

# Update the pointer, scrolling the text widget if necessary to keep the
# pointer in an acceptable part of the screen.

	if {[info exists asm_pointers($cfunc)]} then {
		$win configure -state normal
		set pointer_pos $asm_pointers($cfunc)
		$win configure -state normal
		$win delete $pointer_pos "$pointer_pos + 2 char"
		$win insert $pointer_pos "  "

# Map the PC back to a line in the window		

		set line [pc_to_line $pclist($cfunc) $pc]

		if {$line == -1} {
			echo "Can't find PC $pc"
			return
			}

		set pointer_pos [$win index $line.1]
		set asm_pointers($cfunc) $pointer_pos

		$win delete $pointer_pos "$pointer_pos + 2 char"
		$win insert $pointer_pos "->"
	        $win yview "${line}.0 linestart"
		$win configure -state disabled
		}
}

#
# Local procedure:
#
#	update_ptr - Update the listing window.
#
# Description:
#
#	This routine will update the listing window using the result of
#	gdb_loc.
#

proc update_ptr {} {
	update_listing [gdb_loc]
	if {[winfo exists .asm]} {
		update_assembly [gdb_loc]
	}
	if {[winfo exists .reg]} {
		update_registers changed
	}
	if {[winfo exists .expr]} {
		update_exprs
	}
	if {[winfo exists .autocmd]} {
		update_autocmd
	}
}

# Make toplevel window disappear

wm withdraw .

proc files_command {} {
  toplevel .files_window

  wm minsize .files_window 1 1
  #	wm overrideredirect .files_window true
  listbox .files_window.list -geometry 30x20 -setgrid true \
    -yscrollcommand {.files_window.scroll set} -relief sunken \
    -borderwidth 2
  scrollbar .files_window.scroll -orient vertical \
    -command {.files_window.list yview} -relief sunken
  button .files_window.close -text Close -command {destroy .files_window}
  .files_window.list configure -selectmode single

  # Get the file list from GDB, sort it, and format it as one entry per line.
  set lastSeen {};			# Value that won't appear in
					# list.
  set fileList {}
  foreach file [lsort [gdb_listfiles]] {
    if {$file != $lastSeen} then {
      lappend fileList $file
      set lastSeen $file
    }
  }
  set filelist [join [lsort [gdb_listfiles]] "\n"]

  # Insert the file list into the widget

  eval .files_window.list insert 0 $filelist

  pack .files_window.close -side bottom -fill x -expand no -anchor s
  pack .files_window.scroll -side right -fill both
  pack .files_window.list -side left -fill both -expand yes
  bind .files_window.list <Any-ButtonRelease-1> {
    set file [%W get [%W curselection]]
    gdb_cmd "list $file:1,0"
    update_listing [gdb_loc $file:1]
    destroy .files_window
  }
}

button .files -text Files -command files_command

proc apply_filespec {label default command} {
    set filename [FSBox $label $default]
    if {$filename != ""} {
	if {[catch {gdb_cmd "$command $filename"} retval]} {
	    tk_dialog .filespec_error "gdb : $label error" \
	      "Error in command \"$command $filename\"" error \
	      0 Dismiss
	    return
	}
    update_ptr
    }
}

# Run editor.
proc run_editor {editor file} {
  # FIXME should use index of line in middle of window, not line at
  # top.
  global wins
  set lineNo [lindex [split [$wins($file) index @0,0] .] 0]
  exec $editor +$lineNo $file
}

# Setup command window
proc build_framework {win {title GDBtk} {label {}}} {
	global ${win}.label

	toplevel ${win}
	wm title ${win} $title
	wm minsize ${win} 1 1

	frame ${win}.menubar

	menubutton ${win}.menubar.file -padx 12 -text File \
		-menu ${win}.menubar.file.menu -underline 0

	menu ${win}.menubar.file.menu
	${win}.menubar.file.menu add command -label File... \
		-command {apply_filespec File a.out file}
	${win}.menubar.file.menu add command -label Target... \
		-command { not_implemented_yet "target" }
	${win}.menubar.file.menu add command -label Edit \
	        -command {run_editor $editor $cfile}
	${win}.menubar.file.menu add separator
	${win}.menubar.file.menu add command -label "Exec File..." \
		-command {apply_filespec {Exec File} a.out exec-file}
	${win}.menubar.file.menu add command -label "Symbol File..." \
		-command {apply_filespec {Symbol File} a.out symbol-file}
	${win}.menubar.file.menu add command -label "Add Symbol File..." \
		-command { not_implemented_yet "menu item, add symbol file" }
	${win}.menubar.file.menu add command -label "Core File..." \
		-command {apply_filespec {Core File} core core-file}

	${win}.menubar.file.menu add separator
	${win}.menubar.file.menu add command -label Close \
		-command "destroy ${win}"
	${win}.menubar.file.menu add separator
	${win}.menubar.file.menu add command -label Quit \
		-command {interactive_cmd quit}

	menubutton ${win}.menubar.commands -padx 12 -text Commands \
		-menu ${win}.menubar.commands.menu -underline 0

	menu ${win}.menubar.commands.menu
	${win}.menubar.commands.menu add command -label Run \
		-command {interactive_cmd run}
	${win}.menubar.commands.menu add command -label Step \
		-command {interactive_cmd step}
	${win}.menubar.commands.menu add command -label Next \
		-command {interactive_cmd next}
	${win}.menubar.commands.menu add command -label Continue \
		-command {interactive_cmd continue}
	${win}.menubar.commands.menu add separator
	${win}.menubar.commands.menu add command -label Stepi \
		-command {interactive_cmd stepi}
	${win}.menubar.commands.menu add command -label Nexti \
		-command {interactive_cmd nexti}

	menubutton ${win}.menubar.view -padx 12 -text Options \
		-menu ${win}.menubar.view.menu -underline 0

	menu ${win}.menubar.view.menu
	${win}.menubar.view.menu add command -label Hex \
		-command {echo Hex}
	${win}.menubar.view.menu add command -label Decimal \
		-command {echo Decimal}
	${win}.menubar.view.menu add command -label Octal \
		-command {echo Octal}

	menubutton ${win}.menubar.window -padx 12 -text Window \
		-menu ${win}.menubar.window.menu -underline 0

	menu ${win}.menubar.window.menu
	${win}.menubar.window.menu add command -label Command \
		-command create_command_window
	${win}.menubar.window.menu add separator
	${win}.menubar.window.menu add command -label Source \
		-command create_source_window
	${win}.menubar.window.menu add command -label Assembly \
		-command create_asm_window
	${win}.menubar.window.menu add separator
	${win}.menubar.window.menu add command -label Registers \
		-command create_registers_window
	${win}.menubar.window.menu add command -label Expressions \
		-command create_expr_window
	${win}.menubar.window.menu add command -label "Auto Command" \
		-command create_autocmd_window
	${win}.menubar.window.menu add command -label Breakpoints \
		-command create_breakpoints_window

#	${win}.menubar.window.menu add separator
#	${win}.menubar.window.menu add command -label Files \
#		-command { not_implemented_yet "files window" }

	menubutton ${win}.menubar.help -padx 12 -text Help \
		-menu ${win}.menubar.help.menu -underline 0

	menu ${win}.menubar.help.menu
	${win}.menubar.help.menu add command -label "with GDBtk" \
		-command {echo "with GDBtk"}
	${win}.menubar.help.menu add command -label "with this window" \
		-command {echo "with this window"}
	${win}.menubar.help.menu add command -label "Report bug" \
		-command {exec send-pr}

	pack	${win}.menubar.file \
		${win}.menubar.view \
		${win}.menubar.window -side left
	pack	${win}.menubar.help -side right

	frame ${win}.info
	text ${win}.text -height 25 -width 80 -relief sunken -borderwidth 2 \
		-setgrid true -cursor hand2 -yscrollcommand "${win}.scroll set"

	set ${win}.label $label
	label ${win}.label -textvariable ${win}.label -borderwidth 2 -relief sunken

	scrollbar ${win}.scroll -orient vertical -command "${win}.text yview" \
		-relief sunken

	bind $win <Key-Alt_R> do_nothing
	bind $win <Key-Alt_L> do_nothing

	pack ${win}.label -side bottom -fill x -in ${win}.info
	pack ${win}.scroll -side right -fill y -in ${win}.info
	pack ${win}.text -side left -expand yes -fill both -in ${win}.info

	pack ${win}.menubar -side top -fill x
	pack ${win}.info -side top -fill both -expand yes
}

proc create_source_window {} {
	global wins
	global cfile

	if {[winfo exists .src]} {raise .src ; return}

	build_framework .src Source "*No file*"

# First, delete all the old view menu entries

	.src.menubar.view.menu delete 0 last

# Source file selection
	.src.menubar.view.menu add command -label "Select source file" \
		-command files_command

# Line numbers enable/disable menu item
	.src.menubar.view.menu add checkbutton -variable line_numbers \
		-label "Line numbers" -onvalue 1 -offvalue 0 -command {
		foreach source [array names wins] {
			if {$source == "Blank"} continue
			destroy $wins($source)
			unset wins($source)
			}
		set cfile Blank
		update_listing [gdb_loc]
		}

	frame .src.row1
	frame .src.row2

	button .src.start -width 6 -text Start -command \
		{interactive_cmd {break main}
		 interactive_cmd {enable delete $bpnum}
		 interactive_cmd run }
	button .src.stop -width 6 -text Stop -fg red -activeforeground red \
		-state disabled -command gdb_stop
	button .src.step -width 6 -text Step \
		-command {interactive_cmd step}
	button .src.next -width 6 -text Next \
		-command {interactive_cmd next}
	button .src.continue -width 6 -text Cont \
		-command {interactive_cmd continue}
	button .src.finish -width 6 -text Finish \
		-command {interactive_cmd finish}
	button .src.up -width 6 -text Up \
		-command {interactive_cmd up}
	button .src.down -width 6 -text Down \
		-command {interactive_cmd down}
	button .src.bottom -width 6 -text Bottom \
		-command {interactive_cmd {frame 0}}

	pack .src.start .src.step .src.continue .src.up .src.bottom \
		-side left -padx 3 -pady 5 -in .src.row1
	pack .src.stop .src.next .src.finish .src.down -side left -padx 3 \
		-pady 5 -in .src.row2

	pack .src.row2 .src.row1 -side bottom -anchor w -before .src.info

	$wins($cfile) insert 0.0 "  This page intentionally left blank."
	$wins($cfile) configure -width 88 -state disabled \
		-yscrollcommand ".src.scroll set"
}

proc update_autocmd {} {
	global .autocmd.label
	global accumulate_output

	catch {gdb_cmd "${.autocmd.label}"} result
	if {!$accumulate_output} { .autocmd.text delete 0.0 end }
	.autocmd.text insert end $result
	.autocmd.text see end
}

proc create_autocmd_window {} {
  global .autocmd.label

  if {[winfo exists .autocmd]} {raise .autocmd ; return}

  build_framework .autocmd "Auto Command" ""

  # First, delete all the old view menu entries

  .autocmd.menubar.view.menu delete 0 last

  # Accumulate output option

  .autocmd.menubar.view.menu add checkbutton \
    -variable accumulate_output \
    -label "Accumulate output" -onvalue 1 -offvalue 0

  # Now, create entry widget with label

  frame .autocmd.entryframe

  entry .autocmd.entry -borderwidth 2 -relief sunken
  bind .autocmd.entry <Key-Return> {
    set .autocmd.label [.autocmd.entry get]
    .autocmd.entry delete 0 end
  }

  label .autocmd.entrylab -text "Command: "

  pack .autocmd.entrylab -in .autocmd.entryframe -side left
  pack .autocmd.entry -in .autocmd.entryframe -side left -fill x -expand yes

  pack .autocmd.entryframe -side bottom -fill x -before .autocmd.info
}

# Return the longest common prefix in SLIST.  Can be empty string.

proc find_lcp slist {
# Handle trivial cases where list is empty or length 1
	if {[llength $slist] <= 1} {return [lindex $slist 0]}

	set prefix [lindex $slist 0]
	set prefixlast [expr [string length $prefix] - 1]

	foreach str [lrange $slist 1 end] {
		set test_str [string range $str 0 $prefixlast]
		while {[string compare $test_str $prefix] != 0} {
			decr prefixlast
			set prefix [string range $prefix 0 $prefixlast]
			set test_str [string range $str 0 $prefixlast]
		}
		if {$prefixlast < 0} break
	}
	return $prefix
}

# Look through COMPLETIONS to generate the suffix needed to do command
# completion on CMD.

proc find_completion {cmd completions} {
# Get longest common prefix
	set lcp [find_lcp $completions]
	set cmd_len [string length $cmd]
# Return suffix beyond end of cmd
	return [string range $lcp $cmd_len end]
}

proc create_command_window {} {
	global command_line
	global saw_tab

	set saw_tab 0
	if {[winfo exists .cmd]} {raise .cmd ; return}

	build_framework .cmd Command "* Command Buffer *"

        # Put focus on command area.
        focus .cmd.text

	set command_line {}

	gdb_cmd {set language c}
	gdb_cmd {set height 0}
	gdb_cmd {set width 0}

        # Tk uses the Motifism that Delete means delete forward.  I
	# hate this, and I'm not gonna take it any more.
        set bsBinding [bind Text <BackSpace>]
        bind .cmd.text <Delete> "delete_char %W ; $bsBinding; break"
	bind .cmd.text <BackSpace> {delete_char %W}
	bind .cmd.text <Control-c> gdb_stop
	bind .cmd.text <Control-u> {delete_line %W ; break}
	bind .cmd.text <Any-Key> {
	  set saw_tab 0
	  %W insert end %A
	  %W see end
	  append command_line %A
	  break
	}
	bind .cmd.text <Key-Return> {
	  set saw_tab 0
	  %W insert end \n
	  interactive_cmd $command_line

	  # %W see end
	  # catch "gdb_cmd [list $command_line]" result
	  # %W insert end $result
	  set command_line {}
	  # update_ptr
	  %W insert end "(gdb) "
	  %W see end
	  break
	}
	bind .cmd.text <Button-2> {
	  %W insert end [selection get]
	  %W see end
	  append command_line [selection get]
	  break
	}
	bind .cmd.text <Key-Tab> {
	  set choices [gdb_cmd "complete $command_line"]
	  set choices [string trimright $choices \n]
	  set choices [split $choices \n]

	  # Just do completion if this is the first tab
	  if {!$saw_tab} {
	    set saw_tab 1
	    set completion [find_completion $command_line $choices]
	    append command_line $completion
	    # Here is where the completion is actually done.  If there
	    # is one match, complete the command and print a space.
	    # If two or more matches, complete the command and beep.
	    # If no match, just beep.
	    switch [llength $choices] {
	      0	{}
	      1	{
		%W insert end "$completion "
		append command_line " "
		return
	      }

	      default {
		%W insert end $completion
	      }
	    }
	    bell
	    %W see end
	  } else {
	    # User hit another consecutive tab.  List the choices.
	    # Note that at this point, choices may contain commands
	    # with spaces.  We have to lop off everything before (and
	    # including) the last space so that the completion list
	    # only shows the possibilities for the last token.
	    set choices [lsort $choices]
	    if {[regexp ".* " $command_line prefix]} {
	      regsub -all $prefix $choices {} choices
	    }
	    %W insert end "\n[join $choices { }]\n(gdb) $command_line"
	    %W see end
	  }
	  break
	}
}

proc delete_char {win} {
  global command_line
  set tmp [expr [string length $command_line] - 2]
  set command_line [string range $command_line 0 $tmp]
}

proc delete_line {win} {
  global command_line

  $win delete {end linestart + 6 chars} end
  $win see insert
  set command_line {}
}

#
# fileselect.tcl --
# simple file selector.
#
# Mario Jorge Silva			          msilva@cs.Berkeley.EDU
# University of California Berkeley                 Ph:    +1(510)642-8248
# Computer Science Division, 571 Evans Hall         Fax:   +1(510)642-5775
# Berkeley CA 94720                                 
# 
#
# Copyright 1993 Regents of the University of California
# Permission to use, copy, modify, and distribute this
# software and its documentation for any purpose and without
# fee is hereby granted, provided that this copyright
# notice appears in all copies.  The University of California
# makes no representations about the suitability of this
# software for any purpose.  It is provided "as is" without
# express or implied warranty.
#


# names starting with "fileselect" are reserved by this module
# no other names used.
# Hack - FSBox is defined instead of fileselect for backwards compatibility


# this is the proc that creates the file selector box
# purpose - comment string
# defaultName - initial value for name
# cmd - command to eval upon OK
# errorHandler - command to eval upon Cancel
# If neither cmd or errorHandler are specified, the return value
# of the FSBox procedure is the selected file name.

proc FSBox {{purpose "Select file:"} {defaultName ""} {cmd ""} {errorHandler 
""}} {
    global fileselect
    set w .fileSelect
    if {[Exwin_Toplevel $w "Select File" FileSelect]} {
	# path independent names for the widgets
	
	set fileselect(list) $w.file.sframe.list
	set fileselect(scroll) $w.file.sframe.scroll
	set fileselect(direntry) $w.file.f1.direntry
	set fileselect(entry) $w.file.f2.entry
	set fileselect(ok) $w.but.ok
	set fileselect(cancel) $w.but.cancel
	set fileselect(msg) $w.label
	
	set fileselect(result) ""	;# value to return if no callback procedures

	# widgets
	Widget_Label $w label {top fillx pady 10 padx 20} -anchor w -width 24
	Widget_Frame $w file Dialog {left expand fill} -bd 10
	
	Widget_Frame $w.file f1 Exmh {top fillx}
	Widget_Label $w.file.f1 label {left} -text "Dir"
	Widget_Entry $w.file.f1 direntry {right fillx expand}  -width 30
	
	Widget_Frame $w.file sframe

	scrollbar $w.file.sframe.yscroll -relief sunken \
		-command [list $w.file.sframe.list yview]
	listbox $w.file.sframe.list -relief sunken \
		-yscroll [list $w.file.sframe.yscroll set] -setgrid 1
	pack append $w.file.sframe \
		$w.file.sframe.yscroll {right filly} \
		$w.file.sframe.list {left expand fill} 
	
	Widget_Frame $w.file f2 Exmh {top fillx}
	Widget_Label $w.file.f2 label {left} -text Name
	Widget_Entry $w.file.f2 entry {right fillx expand}
	
	# buttons
	$w.but.quit configure -text Cancel \
		-command [list fileselect.cancel.cmd $w]
	
	Widget_AddBut $w.but ok OK \
		[list fileselect.ok.cmd $w $cmd $errorHandler] {left padx 1}
	
	Widget_AddBut $w.but list List \
		[list fileselect.list.cmd $w] {left padx 1}    
	Widget_CheckBut $w.but listall "List all" fileselect(pattern)
	$w.but.listall configure -onvalue "{*,.*}" -offvalue "*" \
	    -command {fileselect.list.cmd $fileselect(direntry)}
	$w.but.listall deselect

	# Set up bindings for the browser.
	foreach ww [list $w $fileselect(entry)] {
	    bind $ww <Return> [list $fileselect(ok) invoke]
	    bind $ww <Control-c> [list $fileselect(cancel) invoke]
	}
	bind $fileselect(direntry) <Return> [list fileselect.list.cmd %W]
	bind $fileselect(direntry) <Tab> [list fileselect.tab.dircmd]
	bind $fileselect(entry) <Tab> [list fileselect.tab.filecmd]

        $fileselect(list) configure -selectmode single

	bind $fileselect(list) <Button-1> {
	    # puts stderr "button 1 release"
	    $fileselect(entry) delete 0 end
	    $fileselect(entry) insert 0 [%W get [%W nearest %y]]
	}
    
	bind $fileselect(list) <Key> {
	    $fileselect(entry) delete 0 end
	    $fileselect(entry) insert 0 [%W get [%W nearest %y]]
	}
    
	bind $fileselect(list) <Double-ButtonPress-1> {
	    # puts stderr "double button 1"
	    $fileselect(entry) delete 0 end
	    $fileselect(entry) insert 0 [%W get [%W nearest %y]]
	    $fileselect(ok) invoke
	}
    
	bind $fileselect(list) <Return> {
	    $fileselect(entry) delete 0 end
	    $fileselect(entry) insert 0 [%W get [%W nearest %y]]
	    $fileselect(ok) invoke
	}
    }
    set fileselect(text) $purpose
    $fileselect(msg) configure -text $purpose
    $fileselect(entry) delete 0 end
    $fileselect(entry) insert 0 [file tail $defaultName]

    if {[info exists fileselect(lastDir)] && ![string length $defaultName]} {
	set dir $fileselect(lastDir)
    } else {
	set dir [file dirname $defaultName]
    }
    set fileselect(pwd) [pwd]
    fileselect.cd $dir
    $fileselect(direntry) delete 0 end
    $fileselect(direntry) insert 0 [pwd]/

    $fileselect(list) delete 0 end
    $fileselect(list) insert 0 "Big directory:"
    $fileselect(list) insert 1 $dir
    $fileselect(list) insert 2 "Press Return for Listing"

    fileselect.list.cmd $fileselect(direntry) startup

    # set kbd focus to entry widget

#    Exwin_ToplevelFocus $w $fileselect(entry)

    # Wait for button hits if no callbacks are defined

    if {"$cmd" == "" && "$errorHandler" == ""} {
	# wait for the box to be destroyed
	update idletask
	grab $w
	tkwait variable fileselect(result)
	grab release $w

	set path $fileselect(result)
	set fileselect(lastDir) [pwd]
	fileselect.cd $fileselect(pwd)
	return [string trimright [string trim $path] /]
    }
    fileselect.cd $fileselect(pwd)
    return ""
}

proc fileselect.cd { dir } {
    global fileselect
    if {[catch {cd $dir} err]} {
	fileselect.yck $dir
	cd
    }
}
# auxiliary button procedures

proc fileselect.yck { {tag {}} } {
    global fileselect
    $fileselect(msg) configure -text "Yck! $tag"
}

proc fileselect.ok {} {
    global fileselect
    $fileselect(msg) configure -text $fileselect(text)
}

proc fileselect.cancel.cmd {w} {
    global fileselect
    set fileselect(result) {}
    destroy $w
}

proc fileselect.list.cmd {w {state normal}} {
    global fileselect
    set seldir [$fileselect(direntry) get]
    if {[catch {glob $seldir} dir]} {
	fileselect.yck "glob failed"
	return
    }
    if {[llength $dir] > 1} {
	set dir [file dirname $seldir]
	set pat [file tail $seldir]
    } else {
	set pat $fileselect(pattern)
    }
    fileselect.ok
    update idletasks
    if {[file isdirectory $dir]} {
	fileselect.getfiles $dir $pat $state
	focus $fileselect(entry)
    } else {
	fileselect.yck "not a dir"
    }
}

proc fileselect.ok.cmd {w cmd errorHandler} {
    global fileselect
    set selname [$fileselect(entry) get]
    set seldir [$fileselect(direntry) get]

    if {[string match /* $selname]} {
	set selected $selname
    } else {
	if {[string match ~* $selname]} {
	    set selected $selname
	} else {
	    set selected $seldir/$selname
	}
    }

    # some nasty file names may cause "file isdirectory" to return an error
    if {[catch {file isdirectory $selected} isdir]} {
	fileselect.yck "isdirectory failed"
	return
    }
    if {[catch {glob $selected} globlist]} {
	if {![file isdirectory [file dirname $selected]]} {
	    fileselect.yck "bad pathname"
	    return
	}
	set globlist $selected
    }
    fileselect.ok
    update idletasks

    if {[llength $globlist] > 1} {
	set dir [file dirname $selected]
	set pat [file tail $selected]
	fileselect.getfiles $dir $pat
	return
    } else {
	set selected $globlist
    }
    if {[file isdirectory $selected]} {
	fileselect.getfiles $selected $fileselect(pattern)
	$fileselect(entry) delete 0 end
	return
    }

    if {$cmd != {}} {
	$cmd $selected
    } else {
	set fileselect(result) $selected
    }
    destroy $w
}

proc fileselect.getfiles { dir {pat *} {state normal} } {
    global fileselect
    $fileselect(msg) configure -text Listing...
    update idletasks

    set currentDir [pwd]
    fileselect.cd $dir
    if {[catch {set files [lsort [glob -nocomplain $pat]]} err]} {
	$fileselect(msg) configure -text $err
	$fileselect(list) delete 0 end
	update idletasks
	return
    }
    switch -- $state {
	normal {
	    # Normal case - show current directory
	    $fileselect(direntry) delete 0 end
	    $fileselect(direntry) insert 0 [pwd]/
	}
	opt {
	    # Directory already OK (tab related)
	}
	newdir {
	    # Changing directory (tab related)
	    fileselect.cd $currentDir
	}
	startup {
	    # Avoid listing huge directories upon startup.
	    $fileselect(direntry) delete 0 end
	    $fileselect(direntry) insert 0 [pwd]/
	    if {[llength $files] > 32} {
		fileselect.ok
		return
	    }
	}
    }

    # build a reordered list of the files: directories are displayed first
    # and marked with a trailing "/"
    if {[string compare $dir /]} {
	fileselect.putfiles $files [expr {($pat == "*") ? 1 : 0}]
    } else {
	fileselect.putfiles $files
    }
    fileselect.ok
}

proc fileselect.putfiles {files {dotdot 0} } {
    global fileselect

    $fileselect(list) delete 0 end
    if {$dotdot} {
	$fileselect(list) insert end "../"
    }
    foreach i $files {
        if {[file isdirectory $i]} {
	    $fileselect(list) insert end $i/
	} else {
	    $fileselect(list) insert end $i
	}
    }
}

proc FileExistsDialog { name } {
    set w .fileExists
    global fileExists
    set fileExists(ok) 0
    {
	message $w.msg -aspect 1000
	pack $w.msg -side top -fill both -padx 20 -pady 20
	$w.but.quit config -text Cancel -command {FileExistsCancel}
	button $w.but.ok -text OK -command {FileExistsOK}
	pack $w.but.ok -side left
	bind $w.msg <Return> {FileExistsOK}
    }
    $w.msg config -text "Warning: file exists
$name
OK to overwrite it?"

    set fileExists(focus) [focus]
    focus $w.msg
    grab $w
    tkwait variable fileExists(ok)
    grab release $w
    destroy $w
    return $fileExists(ok)
}

proc FileExistsCancel {} {
    global fileExists
    set fileExists(ok) 0
}

proc FileExistsOK {} {
    global fileExists
    set fileExists(ok) 1
}

proc fileselect.getfiledir { dir {basedir [pwd]} } {
    global fileselect

    set path [$fileselect(direntry) get]
    set returnList {}

    if {$dir != 0} {
	if {[string index $path 0] == "~"} {
	    set path $path/
	}
    } else {
	set path [$fileselect(entry) get]
    }
    if {[catch {set listFile [glob -nocomplain $path*]}]} {
	return  $returnList
    }
    foreach el $listFile {
	if {$dir != 0} {
	    if {[file isdirectory $el]} {
		lappend returnList [file tail $el]
	    }
	} elseif {![file isdirectory $el]} {
	    lappend returnList [file tail $el]
	}	    
    }
    
    return $returnList
}

proc fileselect.gethead { list } {
    set returnHead ""

    for {set i 0} {[string length [lindex $list 0]] > $i}\
	{incr i; set returnHead $returnHead$thisChar} {
	    set thisChar [string index [lindex $list 0] $i]
	    foreach el $list {
		if {[string length $el] < $i} {
		    return $returnHead
		}
		if {$thisChar != [string index $el $i]} {
		    return $returnHead
		}
	    }
	}
    return $returnHead
}

# FIXME this function is a crock.  Can write tilde expanding function
# in terms of glob and quote_glob; do so.
proc fileselect.expand.tilde { } {
    global fileselect

    set entry [$fileselect(direntry) get]
    set dir [string range $entry 1 [string length $entry]]

    if {$dir == ""} {
	return
    }

    set listmatch {}

    ## look in /etc/passwd
    if {[file exists /etc/passwd]} {
	if {[catch {set users [exec cat /etc/passwd | sed s/:.*//]} err]} {
	    puts "Error\#1 $err"
	    return
	}
	set list [split $users "\n"]
    }
    if {[lsearch -exact $list "+"] != -1} {
	if {[catch {set users [exec ypcat passwd | sed s/:.*//]} err]} {
	    puts "Error\#2 $err"
	    return
	}
	set list [concat $list [split $users "\n"]]
    }
    $fileselect(list) delete 0 end
    foreach el $list {
	if {[string match $dir* $el]} {
	    lappend listmatch $el
	    $fileselect(list) insert end $el
	}
    }
    set addings [fileselect.gethead $listmatch]
    if {$addings == ""} {
	return
    }
    $fileselect(direntry) delete 0 end
    if {[llength $listmatch] == 1} {
	$fileselect(direntry) insert 0 [file dirname ~$addings/]
	fileselect.getfiles [$fileselect(direntry) get]
    } else {
	$fileselect(direntry) insert 0 ~$addings
    }
}

proc fileselect.tab.dircmd { } {
    global fileselect

    set dir [$fileselect(direntry) get]
    if {$dir == ""} {
	$fileselect(direntry) delete 0 end
	    $fileselect(direntry) insert 0 [pwd]
	if {[string compare [pwd] "/"]} {
	    $fileselect(direntry) insert end /
	}
	return
    }
    if {[catch {set tmp [file isdirectory [file dirname $dir]]}]} {
	if {[string index $dir 0] == "~"} {
	    fileselect.expand.tilde
	}
	return
    }
    if {!$tmp} {
	return
    }
    set dirFile [fileselect.getfiledir 1 $dir]
    if {![llength $dirFile]} {
	return
    }
    if {[llength $dirFile] == 1} {
	$fileselect(direntry) delete 0 end
	$fileselect(direntry) insert 0 [file dirname $dir]
	if {[string compare [file dirname $dir] /]} {
	    $fileselect(direntry) insert end /[lindex $dirFile 0]/
	} else {
	    $fileselect(direntry) insert end [lindex $dirFile 0]/
	}
	fileselect.getfiles [$fileselect(direntry) get] \
	    "[file tail [$fileselect(direntry) get]]$fileselect(pattern)" opt
	return
    }
    set headFile [fileselect.gethead $dirFile]
    $fileselect(direntry) delete 0 end
    $fileselect(direntry) insert 0 [file dirname $dir]
    if {[string compare [file dirname $dir] /]} {
	$fileselect(direntry) insert end /$headFile
    } else {
	$fileselect(direntry) insert end $headFile
    }
    if {$headFile == "" && [file isdirectory $dir]} {
	fileselect.getfiles $dir\
	    "[file tail [$fileselect(direntry) get]]$fileselect(pattern)" opt
    } else {
	fileselect.getfiles [file dirname $dir]\
	    "[file tail [$fileselect(direntry) get]]*" newdir
    }
}

proc fileselect.tab.filecmd { } {
    global fileselect

    set dir [$fileselect(direntry) get]
    if {$dir == ""} {
	set dir [pwd]
    }
    if {![file isdirectory $dir]} {
	error "dir $dir doesn't exist"
    }
    set listFile [fileselect.getfiledir 0 $dir]
    puts $listFile
    if {![llength $listFile]} {
	return
    }
    if {[llength $listFile] == 1} {
	$fileselect(entry) delete 0 end
	$fileselect(entry) insert 0 [lindex $listFile 0]
	return
    }
    set headFile [fileselect.gethead $listFile]
    $fileselect(entry) delete 0 end
    $fileselect(entry) insert 0 $headFile
    fileselect.getfiles $dir "[$fileselect(entry) get]$fileselect(pattern)" opt
}

proc Exwin_Toplevel { path name {class Dialog} {dismiss yes}} {
    global exwin
    if {[catch {wm state $path} state]} {
	set t [Widget_Toplevel $path $name $class]
	if {![info exists exwin(toplevels)]} {
	    set exwin(toplevels) [option get . exwinPaths {}]
	}
	set ix [lsearch $exwin(toplevels) $t]
	if {$ix < 0} {
	    lappend exwin(toplevels) $t
	}
	if {$dismiss == "yes"} {
	    set f [Widget_Frame $t but Menubar {top fill}]
	    Widget_AddBut $f quit "Dismiss" [list Exwin_Dismiss $path]
	}
	return 1
    } else {
	if {$state != "normal"} {
	    catch {
		wm geometry $path $exwin(geometry,$path)
#		Exmh_Debug Exwin_Toplevel $path $exwin(geometry,$path)
	    }
	    wm deiconify $path
	} else {
	    catch {raise $path}
	}
	return 0
    }
}

proc Exwin_Dismiss { path {geo ok} } {
    global exwin
    case $geo {
	"ok" {
	    set exwin(geometry,$path) [wm geometry $path]
	}
	"nosize" {
	    set exwin(geometry,$path) [string trimleft [wm geometry $path] 0123456789x]
	}
	default {
	    catch {unset exwin(geometry,$path)}
	}
    }
    wm withdraw $path
}

proc Widget_Toplevel { path name {class Dialog} {x {}} {y {}} } {
    set self [toplevel $path -class $class]
    set usergeo [option get $path position Position]
    if {$usergeo != {}} {
	if {[catch {wm geometry $self $usergeo} err]} {
#	    Exmh_Debug Widget_Toplevel $self $usergeo => $err
	}
    } else {
	if {($x != {}) && ($y != {})} {
#	    Exmh_Debug Event position $self +$x+$y
	    wm geometry $self +$x+$y
	}
    }
    wm title $self $name
    wm group $self .
    return $self
}

proc Widget_Frame {par child {class GDB} {where {top expand fill}} args } {
    if {$par == "."} {
	set self .$child
    } else {
	set self $par.$child
    }
    eval {frame $self -class $class} $args
    pack append $par $self $where
    return $self
}

proc Widget_AddBut {par but txt cmd {where {right padx 1}} } {
    # Create a Packed button.  Return the button pathname
    set cmd2 [list button $par.$but -text $txt -command $cmd]
    if {[catch $cmd2 t]} {
	puts stderr "Widget_AddBut (warning) $t"
	eval $cmd2 {-font fixed}
    }
    pack append $par $par.$but $where
    return $par.$but
}

proc Widget_CheckBut {par but txt var {where {right padx 1}} } {
    # Create a check button.  Return the button pathname
    set cmd [list checkbutton $par.$but -text $txt -variable $var]
    if {[catch $cmd t]} {
	puts stderr "Widget_CheckBut (warning) $t"
	eval $cmd {-font fixed}
    }
    pack append $par $par.$but $where
    return $par.$but
}

proc Widget_Label { frame {name label} {where {left fill}} args} {
    set cmd [list label $frame.$name ]
    if {[catch [concat $cmd $args] t]} {
	puts stderr "Widget_Label (warning) $t"
	eval $cmd $args {-font fixed}
    }
    pack append $frame $frame.$name $where
    return $frame.$name
}

proc Widget_Entry { frame {name entry} {where {left fill}} args} {
    set cmd [list entry $frame.$name ]
    if {[catch [concat $cmd $args] t]} {
	puts stderr "Widget_Entry (warning) $t"
	eval $cmd $args {-font fixed}
    }
    pack append $frame $frame.$name $where
    return $frame.$name
}

# End of fileselect.tcl.

#
# Create a copyright window and center it on the screen.  Arrange for
# it to disappear when the user clicks it, or after a suitable period
# of time.
#
proc create_copyright_window {} {
  toplevel .c
  message .c.m -text [gdb_cmd {show version}] -aspect 500 -relief raised
  pack .c.m

  bind .c.m <1> {destroy .c}
  # "suitable period" currently means "15 seconds".
  after 15000 {
    if {[winfo exists .c]} then {
      destroy .c
    }
  }

  wm transient .c .
  center_window .c
}

# FIXME need to handle mono here.  In Tk4 that is more complicated.
set highlight "-background red2 -borderwidth 2 -relief sunken"

# Setup the initial windows
create_source_window
create_command_window

# Make this last so user actually sees it.
create_copyright_window
# Refresh.
update

if {[file exists ~/.gdbtkinit]} {
  source ~/.gdbtkinit
}
