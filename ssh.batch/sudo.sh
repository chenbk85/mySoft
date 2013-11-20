#!/bin/sh
# \
exec expect -- "$0" ${1+"$@"}
# Copyright by beyondy, 2008-2010
# All rights reserved.
#
log_user 0
set timeout 5

set user ""
set password ""
set commands ""
set debug 0
set estring ""

proc usage { argv0 } {
	send_user "Usage: $argv0 \[-u username] [-D debug-level] -p password \[--] command \[command-options]\n"
	exit 1
}

proc debug_output { s } {
	global debug
	if {$debug > 0} {
		send_user "===> $s\n"
	}
}

if {[llength $argv] < 1} {
	usage $argv0
}

while {[llength $argv] > 0} {
	set flag [lindex $argv 0]
	switch -glob -- $flag \
	"-u" {
		set user [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "-p" {
		set password [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "-D" {
		set debug [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--" {
		set argv [lrange $argv 2 end]
		break
	} "*" {
		break
	}
}

foreach i $argv {
	set commands [concat $commands $i]
}

if {$debug > 1} { exp_internal [expr { $debug - 1}]; }

set estring "spawn -noecho sudo $commands"
if {[string length $user] > 0} {
	set estring "spawn -noecho sudo -u $user $commands"
}

debug_output "          user: $user"
debug_output "      password: $password"
debug_output "command-string: $commands"
debug_output "   eval-string: $estring"

eval $estring
for {} { 1 } {} {
        expect "*assword:" {
                send "$password\r"
                exp_continue
	} "*incorrect password attempts\r\n" {
		send_user -- "invalid password, exit"
		break
        } ".*" {
                send_user -- $expect_out(buffer)
                exp_continue
        } eof {
                break
        }
}

