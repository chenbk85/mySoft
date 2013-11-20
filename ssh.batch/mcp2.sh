#!/bin/sh
# \
exec expect -- "$0" ${1+"$@"}
# Copyright by beyondy, 2008-2010
# All rights reserved.
#
# 1, Sep 1, 2008
#    shit too many machine to be sshed in.
#
#strace 4

proc usage { argv0 } {
	send_user "usage: $argv0 \[options] source \[ ... ] \[ target ]\n"
	send_user "   or: $argv0 --help | -help\n"
	send_user "\n"
	send_user "   copy source(@local host) to remote-hosts at target, \n"
	send_user "   or source(@remote-hosts) to local at target. this tool call\n"
	send_user "      rsync -qaze \"ssh -l user -p port\" source \[...] host:target\n"
	send_user "   or scp -q -P port \[\[user@]host:]source \[...] \[\[user@]host:]target\n"
	send_user "\n"
	send_user "   Possible Options:\n"
	send_user "       -h | --host=host -------------- add this hosts to host list\n"
	send_user "       -H | --hosts-file=hosts-file -- add hosts in \"hosts-file\" to host list\n"
	send_user "       -s | --remote-is-source ------- copy file(s) from host(s), default to host(s)\n"
	send_user "       -t | --tool=rsync|scp --------- use scp or rsync for coping files\n"
	send_user "       -P | --port=port -------------- possible login port\n"
	send_user "       -u | --user=user -------------- possible login user\n"
	send_user "       -p | --password=password ------ possible login password\n"
	send_user "       -m | --parallel --------------- executed parallelly\n"
	send_user "       -D | --debug-level=N ---------- set debug-level(exp_internal=N-1)\n"
	send_user "       -L | --line-prefix=N ---------- line-prefix: 0: none, 1: each output, 2: each line\n"
	send_user "\n" 
	send_user "   Copyright by beyondy, 2008-2010.\n"
	send_user "   All rights reserved.\n"
	send_user "\n"
	send_user "   Any bug, please report to email: RobertPython@163.com\n"
	send_user "\n"
	exit 1
}

if {[llength $argv] < 2} {
	usage $argv0
}

set hosts {}
set ports {}
set users {}
set passwords {}
set default_users { "user_00" }
set default_passwords { "isd!@#user" "qzone_user_00" }
set default_ports { 36000 22 }
set remote_is_source 0
set tool "rsync"
set title ""
set debug 0
set verbose 0
set lineprefix 1
set hcount 0
set hcwidth 0
set hwidth 0
set parallel 0
set prompt "(%|#|\\$|>) *"	;# default prompt

#
# hostenv(index/ulist/plist/Plist/options/spawn-id/pid)
# children(spawn-id)=host
#
global hostenv
global children
global spawnids

proc debug_output { s } {
	global debug
	if {$debug > 0} {
		send_user "===> $s\n"
	}
}

#
# set xterm's title
#
proc set_xterm_title { title } {
	send_user -- "\x1b];$title\x7";
}

#
# convert host to a string: [index:host]
#
proc host_id_str { host {fixedwidth 0} } {
	global hosts hcount hcwidth hwidth
	set index [lsearch -regex $hosts "$host\s*.*"]
	if { $index < 0 } {
		send_user "$host is not in host list: $hosts\n";
		return "Unkn-$host"; 
	} elseif {$fixedwidth == 0} {
		return [format "\[%0${hcwidth}d:%${hwidth}s]" [expr { $index + 1 }] $host]
	} else {
		return [format "\[%d:%s]" [expr {$index + 1}] $host]
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
proc parse_host_options { hoptions ulist plist Plist tDir } {
	upvar $hoptions options \
	      $ulist users \
	      $plist passwords \
	      $Plist ports \
	      $tDir target

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
		} "-d" {
			set target [lindex $toptions 1]
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
	global users passwords ports target
	global hostenv children
	global default_users default_passwords default_ports

	set ulist {}
	set plist {}
	set Plist {}
	set tdir ""

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
	parse_host_options options ulist plist Plist tdir

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

	if {[llength $target] > 0} {
		set tdir $target
	} elseif {[llength $tdir] == 0} {
		set tdir "."
	}

	debug_output "$host \[$options] -u $ulist -p $plist -P $Plist -d $tdir"

	set hostenv($host,options) $options
	set hostenv($host,users) $ulist
	set hostenv($host,passwords) $plist
	set hostenv($host,ports) $Plist
	set hostenv($host,target) $tdir
	set hostenv($host,uindex) 0
	set hostenv($host,pindex) 0
	set hostenv($host,Pindex) 0
	set hostenv($host,state)  0
}

#
# spawn ssh process
#
proc call_ssh_host { host } {
	global hostenv children spawnids

	set options [lrange $hostenv($host,options) 0 end]
	set user [lindex $hostenv($host,users) 0]
	set password [lindex $hostenv($host,passwords) 0]
	set port [lindex $hostenv($host,ports) 0]
	set src [lindex $src 0]

	set commands ""

	send_user "      rsync -qaze \"ssh -l user -p port\" source \[...] host:target\n"
	send_user "   or scp -q -P port \[\[user@]host:]source \[...] \[\[user@]host:]target\n"

	if {[string compare $tool "rsync"] == 0} {
		if {$remote_is_source == 0} {
			# copy to remote
			set commands "spawn -noecho rsync -qaze \"ssh -l $user -p $port\" [concat $source] $host:$target"
		} else {
			# copy from remote
			set commands "spawn -noecho rsync -qaze \"ssh -l $user -p $port\" $host:'[concat $source]' $target"
		}
	} else {
		if {$remote_is_source == 0} {
			set commands "scp -qp -P $port [concat $source] $user@$host:$target"
		} else {
			set commands "scp -qp -P $port $user@$host:$source $target"
		}
	}

	eval $commands

	set children($spawn_id) $host
	set hostenv($host,spawn_id) $spawn_id
	lappend spawnids $spawn_id

	debug_output "$spawn_id: $children($spawn_id), $commands"
}

proc add_extra_host { i } {
	set host [setup_host_env $i]
	if {[string length $host] == 0} {
		return;
	}
	else {
		call_ssh_host $host
	}
}

proc process_user_input { s } {
	global spawnids
	foreach sid $spawnids {
		send -i $sid -- $s;
	}
}

proc send_ssh_commands { sid commands } {
	upvar $commands clist
	foreach cl $clist {
		send -i $sid "$cl\n"
	}
}

proc process_user_exit { why } {
	global spawnids
	foreach sid $spawnids {
		close -i $sid
	}

	exit 0
}

proc process_password_request { host } {
	global hostenv expect_out

	if {[llength $hostenv($host,passwords)] == 0} {
		#
		# TODO: prompt for password
		#
		send -i $expect_out(spawn_id) "\r"
		return
	}

	set password [lindex $hostenv($host,passwords) $hostenv($host,pindex)]
	incr hostenv($host,pindex)
	if {$hostenv($host,pindex) == [llength $hostenv($host,passwords)]} {
		set hostenv($host,pindex) 0
	}

	send -i $expect_out(spawn_id) "$password\r"
}

proc process_connection_failed { host } {
	global hostenv expect_out
	#
	# TODO: try next user/port, etc
	# 
	return
}

proc process_ssh_output { host str } {
	global hostenv lineprefix  \
	       hcount parallel	   \
	       interactive

	set retval 1
	if {$hostenv($host,state) == 0} {
		# auth [process_ssh_auth]
		return 0
	} elseif {$hostenv($host,state) == 1} {
		# auth succ
		if {$interactive == 1} {
			# logining [process_ssh_login]
			return 0
		}

		set retval 0
	}

	# loged-in 
	# or authed OK in interactive mode
	if {$hcount == 1 || $parallel == 0} {
		send_user -- "$str"
	} elseif {$lineprefix > 1} { 
		foreach line [split $str "\n"] {
			send_user "[host_id_str $host] $line\r\n"
		} 
	} elseif {$lineprefix == 1} {
		send_user "[host_id_str $host] $str"
	} else {
		send_user -- "$str"
	}

	return $retval
}

proc process_ssh_exit { host status} {
	global hostenv spawnids

	set sid $hostenv($host,spawn_id)
	set index [lsearch $spawnids $sid]
	if {$index != -1} { 
		set spawnids [lreplace $spawnids $index $index]
	}

	wait -i $sid
	debug_output "$host\[$sid] exit\n"
	return [llength $spawnids]
}

proc print_host_head { host } {
	global lineprefix
	if {$lineprefix == 0} { return; }

	set htitle [host_id_str $host 1]
	set hlen [expr {[string length $htitle] + 2}]
	set cols 128
#[lindex [stty size] 1]
	set left [expr {($cols - $hlen) / 2}]
	set right [expr {$cols - $hlen - $left}]

	send_user "[string repeat "*" $left] $htitle [string repeat "*" $right]\n"
}

#
# read input and sent them to each children
#
proc do_expect_loop {} {
	global user_spawn_id expect_out
	global hostenv spawnids children
	global prompt
	global interactive commands

	set done 1
#	stty raw -echo

	for {} {$done} {} {
		expect {
		-i $user_spawn_id -re ".+" {
			process_user_input $expect_out(buffer)
			exp_continue
		} eof {
			process_user_exit { "eof" }
		}
#
# Possible output from ssh
# TODO: handle more perfect...
#
		-i "$spawnids" "*assword: " {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				process_password_request $host
			}

			exp_continue

		} "*Host key not found from database*" {

			exp_continue

		} "*continue connecting (yes/no)? " {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send -i $expect_out(spawn_id) "yes\r"
			}

			exp_continue

		} "*successful.\r\r\n" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				set host $children($expect_out(spawn_id))
				set hostenv($host,state) 1
			}

			exp_continue

		} "*Authentication failed.\r\r\n" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send_user "$host: Authentication failed.\r\n"
			}

			exp_continue

		} "*No further authentication methods available.*" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send_user "$host: No further authentication methods available.\r\n";
			}

			exp_continue

		} "*Destination Unreachable\r\r\n" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send_user "$host: Destination Unreachable.\r\n"
			}

			exp_continue

		} "*Unreachable" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send_user "$host: Unreachable\r\n"
			}

			exp_continue

		} -re $prompt {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				set hostenv($host,state) 2

				if {$interactive == 1 && [llength $commands] > 1} {
					send_ssh_commands $expect_out(spawn_id) commands
				} elseif {$interactive == 2} {
					# must be one process this time
					send_user $expect_out(buffer)

					interact -i $expect_out(spawn_id) 
					set done [process_ssh_exit $host "eof"]

					return
				}
			}

			exp_continue

		} -re ".+" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				debug_output "unknown response\[$host]: $expect_out(buffer)"
			}

			exp_continue

		} eof {
			set host $children($expect_out(spawn_id))
			set done [process_ssh_exit $host "eof"]
		}
	} }

#	stty -row echo
}

#
# read input and sent them to each children
#
proc do_interact_loop {} {
	global user_spawn_id expect_out
	global hostenv spawnids children
	global prompt
	global interactive commands


	set done 1

}

while {[llength $argv] > 0} {
	set flag [lindex $argv 0]
	switch -glob -- $flag \
	"-h" {
		lappend hosts [lindex $argv 1]
		set title [concat $title [lindex $argv 1]] 
		set argv [lrange $argv 2 end]
	} "--host=*" {
		lappend hosts [string range [lindex $argv 0] 7 end]
		set title [concat $title [string range [lindex $argv 0] 7 end]]
		set argv [lrange $argv 1 end]
	} "-H" {
		parse_hosts_file hosts [lindex $argv 1]
		set title [concat $title [lindex $argv 1]]
		set argv [lrange $argv 2 end]
	} "--hosts-file=*" {
		parse_hosts_file hosts [string range [lindex $argv 0] 13 end]
		set title [concat $title [string range [lindex $argv 0] 13 end]]
		set argv [lrange $argv 1 end]
	} "-s" {
		set remote_is_source 1
		set argv [lrange $argv 1 end]
	} "--remote-is-source" {
		set remote_is_source 1
		set argv [lrange $argv 1 end]
	} "-t" {
		set tool [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--tool=*" {
		set tool [string range [lindex $argv 0] 7 end]
		set argv [lrange $arv 1 end]
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

if {[llength $argv] > 1} {
	set sources [lrange $argv 0 -1]
	set target [lindex $argv end]
	
} elseif {[llength $argv] == 1} {
	set target "."
} else {
	usage $argv0
}

#
# global setting check and init
#
if {1} {
	if {[llength $hosts] == 0} {
		send_user "No host specified.\n";
		exit 1
	}

#	if {[llength $users] == 0} { lappend users "user_00" }
#	if {[llength $passwords] == 0} { lappend passwords "isd!@#user" "qzone_user_00" }
#	if {[llength $ports] == 0} { lappend ports 36000 22 } 
}

log_user 0
set timeout -1
if {$debug > 1} { exp_internal [expr { $debug - 1}]; }

#
# setup some variables according to arguments
#
set hcount [llength $hosts]

set k $hcount
while { $k > 0 } {
	incr hcwidth
	set k [expr { $k / 10 }]
}

foreach i $hosts {
	set host [lindex [split $i] 0]
	if {[string length $host] > $hwidth} {
		set hwidth [string length $host]
	}
}

debug_output "    debug: $debug"
debug_output "    Hosts: $hosts"
debug_output "RemoteSrc: $remote_is_source"
debug_output "     Tool: $tool"
debug_output " HostsC-W: $hcount, $hcwidth, $hwidth"
debug_output "    Ports: $ports"
debug_output "    Users: $users"
debug_output "Passwords: $passwords"
debug_output " Parallel: $parallel"
debug_output "    Title: $title"

trap { 
	foreach sid $spawnids {
		close -i $sid
	}

	exit 1
} { SIGINT SIGTERM SIGHUP } 

#
# init hosts map
#
foreach i $hosts {
	setup_host_env $i
}

#
# start ssh
#
if {$parallel == 0} {
	foreach i $hosts {
		set host [lindex [split $i] 0]
		set_xterm_title $host
		print_host_head $host

		set retry 1
		for {set j 0} {$j < 20 && $retry == 1} {incr j} {
			call_ssh_host $host
			set retry [do_expect_loop]
		}

		if {$retry == 1} {
			send_user "$host: something wrong?\r\n"
			send_user "$host: why always response \"tcsetattr failed in ssh_rl_set_tty_modes_for_fd: fd 1: Interrupted system call.\"\r\n";
		}
	}
} else {
	set_xterm_title $title

	foreach i $hosts {
#		sleep 1
		set host [lindex [split $i] 0]
		call_ssh_host $host
	} 

	do_expect_loop
}

set_xterm_title "what..."

