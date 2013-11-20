#!/bin/sh
# \
exec expect -- "$0" ${1+"$@"}
# Copyright by beyondy, 2008-2010
# All rights reserved.
#
# 1, Sep 1, 2008
#    shit too many machine to be sshed in.
#

#
# setup expect env
#
#set strace 4
log_user 0
remove_nulls

proc usage { argv0 } {
	send_user "Usage: $argv0 \[options] local-path remote-path\n"
	send_user "  Possible options:\n"
	send_user "    -u | --user=user ----------------- possible login user\n"
	send_user "    -p | --password=password --------- possible password\n"
	send_user "    -P | --port=ports ---------------- possible port\n"
	send_user "    -H | --hosts-file=hosts-file ----- add target host(s) from file\n"
	send_user "    -h | --host=host ----------------- add target host(s)\n" 
	send_user "    -D | --debug-level=N ------------- debug level\n"
	send_user "    -m | --parallel ------------------ parallellly\n"
	send_user "    -r | --reverse ------------------- copy file from remote hosts.\n"
	send_user "    -help | --help ------------------- show this message.\n"
	exit 0
}

if {[llength $argv] < 3} {
	usage $argv0
}

#
# global vars
#
set sources ""
set target "."
set hosts {}
set users {}
set passwords {}
set default_users { "user_00" "user_00" }
set default_passwords { "isd!@#user" "qzone_user_00" }
set default_ports { "36000" "22" }
set ports {}
set parallel 0
set reverse 0
set debug 0


#
# hostenv($host,options/users/passwords/ports/uindex/pindex/Pindex/state)
#    where state in case
#	0:	no connected
#	1:	auth ok
#	2:	interactive
# children(spawn-id)=host
#
global hostenv
global children
global spawnids

#
# output debug message
#
proc debug_output { s } {
	global debug
	if {$debug > 0} {
		send_user "===> $s\n"
	}
}

#
# parse host-file, add host(s) to hlist
#
proc parse_hosts_file { hlist file } {
	upvar $hlist hosts
	set ifp [open $file r]
	while { [gets $ifp line] >= 0 } {
		# each line is one of comment, empty-line, hosts
		# host-or-ip -p pass[,...] -u user[,...] -P port[,...]
		# skip comment and empty lines
		set host [string trim $line]
		if {[string match "#*" $host] || [string length $host] == 0} {
			continue;
		}

		lappend hosts $host
	}

	close $ifp
}

#
# parse host's possible options, extract -p password -u user -P port and
# leave anything else to options
#
proc parse_host_options { hoptions ulist plist Plist } {
	upvar $hoptions options \
	      $ulist users \
	      $plist passwords \
	      $Plist ports

	set toptions [lrange $options 0 end]
	set options {}

	while {[llength $toptions] > 0} {
		set flag [lindex $toptions 0]
		switch -glob -- $flag \
		"-u" {
			lappend users [lindex $toptions 1]
			set toptions [lrange $toptions 2 end]
		} "-p" {
			lappend passwords [lindex $toptions 1]
			set toptions [lrange $toptions 2 end]
		} "-P" {
			lappend ports [lindex $toptions 1]
			set toptions [lrange $toptions 2 end]
		} default {
			lappend options $flag
			set toptions [lrange $toptions 1 end]
		}
	}
}

#
# setup hostenv($host,....) by i, which mybe formated as host [options]
#
proc setup_host_env { i } {
	global users passwords ports
	global hostenv children
	global default_users default_passwords default_ports

	set ulist {}
	set plist {}
	set Plist {}

	set hcommands [split $i]
	set host [lindex $hcommands 0]
	set options [lrange $hcommands 1 end]

	if {[info exists hostenv($host,users)] == 1} {
		send_user "$host alread in host-list\r"
		return ""
	}

	#
	# parse special options
	#
	parse_host_options options ulist plist Plist

	#
	# if not specified for this host, set them to default.
	#  command optiosn first, then file option, last default
	#
	if {[llength $users] > 0} {
		set ulist [lrange $users 0 end]
	} elseif {[llength $ulist] == 0} {
		set ulist [lrange $default_users 0 end]
	}

	if {[llength $passwords] > 0} {
		set plist [lrange $passwords 0 end]
	} elseif {[llength $plist] == 0} { 
		set plist [lrange $default_passwords 0 end] 
	}

	if {[llength $ports] > 0} {
		set Plist [lrange $ports 0 end] 
	} elseif {[llength $Plist] == 0} { 
		set Plist [lrange $default_ports 0 end]
	}

	debug_output "$host \[$options] -u $ulist -p $plist -P $Plist"

	set hostenv($host,options) $options
	set hostenv($host,users) $ulist
	set hostenv($host,passwords) $plist
	set hostenv($host,ports) $Plist
	set hostenv($host,uindex) 0
	set hostenv($host,pindex) 0
	set hostenv($host,Pindex) 0
	set hostenv($host,state)  0
}

while {[llength $argv] > 0} {
	set flag [lindex $argv 0]
	switch -glob -- $flag \
	"-h" {
		lappend hosts [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--host=*" {
		lappend hosts [string range [lindex $argv 0] 7 end]
		set argv [lrange $argv 1 end]
	} "-H" {
		parse_hosts_file hosts [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--hosts-file=*" {
		parse_hosts_file hosts [string range [lindex $argv 0] 13 end]
		set argv [lrange $argv 1 end]
	} "-P" {
		lappend ports [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--port=*" {
		lappend ports [string range [lindex $argv 0] 7 end]
		set argv [lrange $argv 1 end]
	} "-u" {
		lappend users [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--user=*" {
		lappend users [string range [lindex $argv 0] 7 end]
		set argv [lrange $argv 1 end]
	} "-p" {
		lappend passwords [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--password=*" {
		lappend passwords [string range [lindex $argv 0] 11 end]
		set argv [lrange $argv 1 end]
	} "-D" {
		set debug [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--debug-level=*" {
		set debug [string range [lindex $argv 0] 14 end]
		set argv [lrange $argv 1 end]
	} "-r" {
		set reverse 1
		set argv [lrange $argv 1 end]
	} "--reverse" {
		set reverse 1
		set argv [lrange $argv 1 end]
	} "-L" {
		set lineprefix [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--line-prefix=*" {
		set lineprefix [string range [lindex $argv 0] 14 end]
		set argv [lrange $argv 1 end]
	} "-m" {
		set parallel 1
		set argv [lrange $argv 1 end]
	} "--parallel" {
		set parallel 1
		set argv [lrange $argv 1 end]
	} "-*help" {
		usage $argv0
	} default {
		break
	}
}

#
# parse left args
#
if {[llength $argv] == 0} {
	usage $argv0
} elseif {[llength $argv] == 1} {
	set sources [lindex $argv 0]
	set target "."
} else {
	foreach i [lrange $argv 0 [expr [llength $argv] - 2]] {
		set sources [concat $sources $i]
	}

	set target [lindex $argv end]
}

debug_output "    debug: $debug"
debug_output "    Hosts: $hosts"
debug_output "    Ports: $ports"
debug_output "    Users: $users"
debug_output "Passwords: $passwords"
debug_output " Parallel: $parallel"
debug_output "  sources: $sources"
debug_output "   target: $target"
debug_output "  reverse: $reverse"

foreach i $hosts {
	#
	# setup host env
	#
	set host [lindex [split $i] 0]
	setup_host_env $i

	set authfail 1
	set retry 0
	set index 0
	set maxretry [llength $hostenv($host,users)]

	for {} {$retry < $maxretry && $authfail == 1} {} {
		set pindex 0
		set user [lindex $hostenv($host,users) $index]
		set port [lindex $hostenv($host,ports) $index]

		incr retry
		incr index
		set authfail 0

		if {$reverse == 0} {
	#		set commands "spawn -noecho scp -qp $sources $user@$host#$port:$target"
			set commands "spawn -noecho rsync -aqz $sources $user@$host#$port:$target"
		} else {
	#		set commands "spawn -noecho scp -qp $user@$host#$port:$sources $target"
			set commands "spawn -noecho rsync -aqz $user@$host#$port:'$sources' $target"
		}

		send_user "$commands\n"
		eval $commands

		expect -- "password: " {
			set password [lindex $hostenv($host,passwords) $pindex]
			incr pindex
			if {$pindex == [llength $hostenv($host,passwords)]} { set pindex 0 }
			send "$password\r"
			exp_continue
		} "*Authentication failed.*" {
			set authfail 1
			send_user "Authentication failed.\n"
		} "*No such file or directory*" {
			send_user $expect_out(buffer)
			exp_continue
		} "*rsync: *" {
			send_user $expect_out(buffer)
			exp_continue
		} "*100%*" {
			send_user "$expect_out(buffer)";
			exp_continue
		} timeout {
			send_user "timeout, exit\n";
			#break
		} eof {
			#OK
		} "Received signal 1*" {
			#send_user "signal 1\r\n";
			exp_continue
		} -re ".+" {
			#send_user $expect_out(buffer)
			exp_continue
		}
	}
}

