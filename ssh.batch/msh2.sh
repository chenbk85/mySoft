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
log_user 0
set timeout 500

proc usage { argv0 } {
	send_user "                ************************************\n"
	send_user "                ***** batch ssh/rsync/scp tool *****\n"
	send_user "                ************************************\n"
	send_user "\n"
	send_user "Usage: $argv0 \[-t | --tool=ssh] \[options] \[command \[command-options]]\n"
	send_user "\n"
	send_user "    Auto login remote hosts one by one, \n"
	send_user "    or execute command on remote hosts one by one or parallelly, which calls\n"
	send_user "       ssh \[options] -l user -p port host \[command \[...]]\n"
	send_user "\n"
	send_user "Usage: $argv0 {-t | --tool=scp|rsync} \[options] \[source \[...] \[target] ]\n"
	send_user "\n"
	send_user "    Copy sources@local to target@remote-hosts, \n"
	send_user "    or source@remote-hosts to target@local. this tool calls\n"
	send_user "       rsync \[options] -e \"ssh -l user -p port\" source \[...] host:target, or\n"
	send_user "       scp \[options] -P port \[\[user@]host:]source \[...] \[\[user@]host:]target\n"
	send_user "\n"
	send_user "Usage: $argv0 --help | -help\n"
	send_user "\n"
	send_user "    Show this help message.\n"
	send_user "\n"
	send_user "Possible options:\n"
	send_user "    -t | --tool=ssh|rsync|scp ----- the background tool to be used.\n"
	send_user "    -h | --host=host -------------- add this hosts to host list\n"
	send_user "    -H | --hosts-file=hosts-file -- add hosts in \"hosts-file\" to host list\n"
	send_user "    -P | --port=port -------------- possible login port\n"
	send_user "    -u | --user=user -------------- possible login user\n"
	send_user "    -p | --password=password ------ possible login password\n"
	send_user "    -m | --parallel --------------- executed parallelly\n"
	send_user "    -E | --execute=shell-file ----- ssh execute shell-file on remote host\n"
	send_user "    -D | --debug-level=N ---------- set debug-level(exp_internal=N-1)\n"
	send_user "    -L | --line-prefix=N ---------- line-prefix: 0: none, 1: each output, 2: each line\n"
	send_user "    -o | --options=options -------- options for sub-commands.\n"
	send_user "\n"
	send_user "Options only available for rsync, scp, etc.:\n"
	send_user "    -r | --reverse ---------------- copy file from remote hosts to local.\n"
	send_user "    -C | --mk-cd-hdir=dir --------- make and change to host dir.\n"
	send_user "\n" 
	send_user "Copyright by beyondy, 2008-2010.\n"
	send_user "All rights reserved.\n"
	send_user "\n"
	send_user "Any bug(s), please report to email: RobertPython@163.com\n"
	send_user "\n"
	exit 1
}

if {([llength $argv] < 1)} { 
	usage $argv0
}

#
# operation const types
#
set op_ssh 0
set op_rsync 1
set op_scp 2
set op_rsh 3
set op_rcp 4

set action $op_ssh
set hosts {}
set ports {}
set users {}
set passwords {}
set default_users {}
set default_passwords {}
set default_ports {}
set commands {}
set is_command_file 0
set title ""
set debug 0
set verbose 0
set lineprefix 1
set interactive 2
set reverse 0
set hdir ""
set sources ""
set target ""
set hcount 0
set hcwidth 0
set hwidth 0
set parallel 0
set options ""
#set prompt "(%|#|\\$|>) *"	;# default prompt

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
set spawnids {}


#
# set default action type by argv0
#
set is_msh [regexp -lineanchor "msh(\[0-9]*)(\.sh)?$" $argv0]
if {$is_msh > 0} {
	set action $op_ssh
} else {
	set action $op_rsync
}

proc debug_output { s } {
	global debug
	if {$debug > 0} {
		send_user "===> $s\n"
	}
}

#
# convert action from string to macro
#
proc setup_action { s } {
	global action op_ssh op_rsync op_scp \
	       op_rcp op_rsh
	if {[string compare $s "ssh"] == 0} {
		set action $op_ssh
	} elseif {[string compare $s "rsync"] == 0} {
		set action $op_rsync
	} elseif {[string compare $s "scp"] == 0} {
		set action $op_scp
	} elseif {[string compare $s "rcp"] == 0} {
		set action $op_rcp
	} elseif {[string compare $s "rsh"] == 0} {
		set action $op_rsh
	} else {
		send_user "Invalid tool($s), it must be ssh, rsync, scp, rcp or rsh now.\n"
		exit 1
	}
}

#
# convert action from macro to string
#
proc action_to_string { action } {
	global op_ssh op_rsync op_scp \
	       op_rcp op_rsh
	if {$action == $op_ssh} {
		return "ssh"
	} elseif {$action == $op_rsync} {
		return "rsync"
	} elseif {$action == $op_scp} {
		return "scp"
	} elseif {$action == $op_rcp} {
		return "rcp"
	} elseif {$action == $op_rsh} {
		return "rsh"
	} else {
		return "unkn-op"
	}
}

#
# load default users,  passwords and ports
#
proc load_default_setting {} {
	global default_users \
	       default_passwords \
	       default_ports
	
	set profile "~/.msh.profile"
	if {[catch {open $profile r} ifp]} {
		debug_output "open $profile failed: $ifp"
		return
	}
	
	while { [gets $ifp line] >= 0 } {
		#
		# each line is one of comment, empty-line, setting
		# host-or-ip -p pass[,...] -u user[,...] -P port[,...]
		# skip comment and empty lines
		#
		set line [string trim $line]
		if {[string match "#*" $line] || [string length $line] == 0} {
			continue;
		}

		set fs [split $line]
		if {[string compare "user:" [lindex $fs 0]] == 0} {
			foreach e [lrange $fs 1 end] {
				lappend default_users "$e"
			}
# why not work?
#			lappend default_users [lrange $fs 1 end]
		} elseif {[string compare "password:" [lindex $fs 0]] == 0} {
			foreach e [lrange $fs 1 end] {
				lappend default_passwords $e
			}
		} elseif {[string compare "port:" [lindex $fs 0]] == 0} {
			foreach e [lrange $fs 1 end] {
				lappend default_ports $e
			}
		} else {
			#
			# TODO: to be supported
			#
		}
	}

	close $ifp
}

#
# check default setting
#
proc check_default_setting {} {
	global default_users \
	       default_passwords \
	       default_ports \
	       options \
	       action op_ssh op_rsh op_rsync op_scp op_rcp \
	       hdir

	if {[llength $default_users] == 0} {
		set default_users [list "user_00"]
	}
	if {[llength $default_passwords] == 0} {
		#set default_passwords [list "nobody"]
		set default_passwords [list "isd!@#user" "qzone_user_00"]
	}
	if {[llength $default_ports] == 0} {
		#set default_ports [list 22]
		set default_ports [list 36000 22]
	}

	if {$action == $op_ssh && [string length $options] < 1} {
		set options ""
	} elseif {$action == $op_rsh && [string length $options] < 1} {
		set options ""
	} elseif {$action == $op_rsync && [string length $options] < 1} {
		set options "-varz"
	} elseif {$action == $op_scp && [string length $options] < 1} {
		set options "-qpr"
	} elseif {$action == $op_rcp && [string length $options] < 1} {
		set options ""
	}

	if {[string compare $hdir "host"] == 0} {
		set hdir "\$host"
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
		send_error "$host is not in host list: $hosts\n";
		return "Unkn-$host"; 
	} elseif {$fixedwidth == 0} {
		return [format "\[%0${hcwidth}d:%${hwidth}s]" [expr { $index + 1 }] $host]
	} else {
		return [format "\[%d:%s]" [expr {$index + 1}] $host]
	}
}

#
# parse command file, append to command
#
proc parse_command_file { clist file } {
	upvar $clist commands

	if {[catch {open $file r} ifp]} {
		send_error "can not open file: $file, error: $ifp\n"
		return
	}

	set first 1

	while {[gets $ifp line] >= 0 } {
		if {$first == 1} {
			lappend commands "( $line\n"
			set first 0
		} else {
			lappend commands "$line\n"
		}
	}

	if {$first == 0} {
		lappend commands ")\n"
	}

	close $ifp
}

#
# parse host-file, add host(s) to hlist
#
proc parse_hosts_file { hlist file } {
	upvar $hlist hosts

	if {[catch {open $file r} ifp]} {
		send_error -- "can not open: $file, error: $ifp\n"
		return;
	}

	while { [gets $ifp line] >= 0 } {
		#
		# each line is one of comment, empty-line, hosts
		# host-or-ip -p pass[,...] -u user[,...] -P port[,...]
		# skip comment and empty lines
		#
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

	#
	# overwrite the previous one
	#
	if {1} {
		if {[info exists hostenv($host,users)] == 1} {
			send_error "$host alread in host-list\r"
			return ""
		}
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
	set hostenv($host,state) 0

	return $host	
}

#
# spawn ssh process
#
proc call_ssh_host { host } {
	global action op_ssh op_scp op_rsync op_rcp op_rsh
	global sources target reverse hdir options
	global commands is_command_file
	global hostenv children spawnids

	set loptions [lrange $hostenv($host,options) 0 end]
	set user [lindex $hostenv($host,users) 0]
	set password [lindex $hostenv($host,passwords) 0]
	set port [lindex $hostenv($host,ports) 0]
	set command [lindex $commands 0]

	if {$is_command_file != 0} {
		set command ""
	}

	set cmd ""
	set need_check_hdir 0

	if {$action == $op_ssh} {
		set cmd "spawn -noecho ssh $options $loptions -l $user -p $port $host $command"
	} elseif {$action == $op_rsync && $reverse == 0} {
		set cmd "spawn -noecho rsync $options $loptions -e \"ssh -l $user -p $port\" $sources $host:$target"
		set need_check_hdir 1
	} elseif {$action == $op_rsync && $reverse != 0} {
		set cmd "spawn -noecho rsync $options $loptions -e \"ssh -l $user -p $port\" $host:'$sources' $target"
		set need_check_hdir 1
	} elseif {$action == $op_scp && $reverse == 0} {
		set cmd "spawn -noecho scp $options $loptions -P $port $sources $user@$host:$target"
		set need_check_hdir 1
	} elseif {$action == $op_scp && $reverse != 0} {
		set cmd "spawn -noecho scp $options $loptions -P $port $user@$host:$sources $target"
		set need_check_hdir 1
	} else {
		send_error "un-support action-type: $action\n"
		return
	}	

	if {$need_check_hdir == 1 && [string length $hdir] > 0} {
		set cwd [pwd]

		if {[catch { eval "system mkdir -p $hdir"; eval "cd $hdir"} err]} {
			send_error "can not mkdir and cd to: $hdir, error: $err\n"
			return
		}

		debug_output "change to child-dir: [pwd]"
		eval $cmd

		cd $cwd
		debug_output "restore work-dir: [pwd]"
	} else {
		eval $cmd
	}

	set children($spawn_id) $host
	set hostenv($host,spawn_id) $spawn_id
	lappend spawnids $spawn_id

	debug_output "$spawn_id: $children($spawn_id): $cmd"
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
		send -i $sid "$cl"
	}
}

proc process_user_exit { why } {
	global spawnids
	foreach sid $spawnids {
		close -i $sid
#		kill ???
#		wait $sid
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

#
# process ssh output according by its auth-state
# 
#  return 0: message not processed
#         1: message is been processed
#
proc process_ssh_output { host str } {
	global hostenv lineprefix  \
	       hcount parallel	   \
	       interactive

	set retval 1
	if {$hostenv($host,state) == 0} {
		#
		# case 0: before auth OK
		#
		# auth [process_ssh_auth]
		return 0
	} elseif {$hostenv($host,state) == 1} {
		#
		# case 1: auth succ
		#
		if {$interactive == 1} {
			# logining [process_ssh_login]
			# to send command
			return 0
		}

		set retval 0
	}

	#
	# loged-in 
	# or authed OK in interactive mode
	# output whatever the peer sent
	#
	if {$hcount == 1 || $parallel == 0} {
		send_user -- "$str"
	} elseif {$lineprefix > 1} { 
		foreach line [split [regsub "\n$" $str ""] "\n"] {
			send_user "[host_id_str $host] $line\r\n"
		} 
	} elseif {$lineprefix == 1} {
		send_user "[host_id_str $host] $str"
	} else {
		send_user -- "$str"
	}

	return $retval
}

#
# called when host is exit
#
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

#
# as title, get current screen's width
#
proc get_screen_width {} {
	if {[catch {open "|stty size"} ifp]} {
		debug_output "stty size failed: $ifp"
		return 80
	}
	
	if { [gets $ifp line] >= 0 } {
		set w [lindex [split $line] 1]
	}

#	set w [lindex [stty size] 1]

	return $w
}

#
# print the titile for host
#
proc print_host_head { host } {
	global lineprefix
	if {$lineprefix == 0} { return; }

	set htitle [host_id_str $host 1]
	set hlen [expr {[string length $htitle] + 2}]
	set cols [get_screen_width]
	set left [expr {($cols - $hlen) / 2}]
	set right [expr {$cols - $hlen - $left}]

	send_user "[string repeat "*" $left] $htitle [string repeat "*" $right]\n"
}

#
# read input and sent them to each children
# return retry:
#	0: OK, do not retry
#	1: retry this again
#
proc do_expect_loop {} {
	global user_spawn_id expect_out
	global hostenv spawnids children
	global interactive commands

	set done 1
	set retry 0

	for {} {$done && [llength $spawnids] > 0} {} {
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

		} "*tcsetattr failed in ssh_rl_set_tty_modes_for_fd*" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				debug_output "ssh-bugs: $expect_out(buffer)"
				debug_output "$host: something wrong, retry it.\r\n"

				close -i $expect_out(spawn_id)

				set done [process_ssh_exit $host "eof"]
				set retry 1
			} else {
				exp_continue
			}

		} "*Host key not found from database*" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				# nothing
			}

			exp_continue

		} "*ssh-keygen*" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				# nothing
			}

			exp_continue
			
		} "*continue connecting (yes/no)? " {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send -i $expect_out(spawn_id) "yes\r"
			}

			exp_continue

		} "*Authentication successful.\r\r\n" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				set host $children($expect_out(spawn_id))
				if {$interactive == 0} {
					# wait for response
					set hostenv($host,state) 2
				} elseif {$interactive == 2} {
					# enter interactive
					interact -i $expect_out(spawn_id) 
					set done [process_ssh_exit $host "eof"]
				
					return 0
				} else {
					# need input command one by one
					set hostenv($host,state) 1
				}
			}

			exp_continue

		} "*Permission denied, please try again.\r\r\n" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send_user -- "$host: Authentication failed.\r\n"
			}

			exp_continue

		} "*Authentication failed.\r\r\n" {
			set host $children($expect_out(spawn_id))
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				send_user -- "$host: Authentication failed.\r\n"
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

		} "*Last login:*" {
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
				if {$interactive == 1} {
					debug_output "unknown response\[$host]: $expect_out(buffer)"
				} else {
					send_user -- $expect_out(buffer)
				}
			}

			exp_continue

		} eof {
			set host $children($expect_out(spawn_id))
			set done [process_ssh_exit $host "eof"]
		}
	} }

	return $retry
}

#
# read input and sent them to each children
# return retry:
#	0: OK, do not retry
#	1: retry this again
#
proc do_rsync_loop {} {
	global user_spawn_id expect_out
	global hostenv spawnids children

	set done 1
	set retry 0

	for {} {$done && [llength $spawnids] > 0} {} {
		expect {
		-i $user_spawn_id -re ".+" {
			process_user_input $expect_out(buffer)
			exp_continue
		} eof {
			process_user_exit { "eof" }
		}
#
# Possible output from rsync|scp
# TODO: handle more perfect...
#
		-i "$spawnids" "*assword:*" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)
			
			if {[process_ssh_output $host $expect_out(buffer)] == 0} {
				process_password_request $host
			}

			exp_continue

		} "*Authentication failed.*" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)

			close -i $expect_out(spawn_id)

			set done [process_ssh_exit $host "eof"]
			set retry 1

		} "*Permission denied*\r\n" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)

			exp_continue

		} "*No such file or directory*" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)

			exp_continue

		} "*rsync: *" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)

			exp_continue

		} "*100%*" {
			set host $children($expect_out(spawn_id))
			send_user "$expect_out(buffer)";

			exp_continue

		} timeout {
			set host $children($expect_out(spawn_id))
			send_user "timeout, exit\n";

		} eof {
			set host $children($expect_out(spawn_id))
			set done [process_ssh_exit $host "eof"]

		} "Received signal 1*" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)

			exp_continue
		} -re ".+" {
			set host $children($expect_out(spawn_id))
			send_user $expect_out(buffer)

			exp_continue
		}
	} }

	return $retry
}

#
# parse args
#
while {[llength $argv] > 0} {
	set flag [lindex $argv 0]
	switch -glob -- $flag \
	"-t" {
		setup_action [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--tool=*" {
		setup_action [string range [lindex $argv 0] 7 end]
		set argv [lrange $argv 1 end]
	} "-h" {
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
	} "-E" {
		parse_command_file commands [lindex $argv 1]
		set argv [lrange $argv 2 end]
		set is_command_file [expr {$is_command_file + 1}]
	} "--execute=*" {
		parse_command_file commands [string range [lindex $argv 0] 10 end]
		set argv [lrange $argv 1 end]
		set is_command_file [expr {$is_command_file + 1}]
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
	} "-r" {
		set reverse 1
		set argv [lrange $argv 1 end]
	} "--reverse" {
		set reverse 1
		set argv [lrange $argv 1 end]
	} "-C" {
		set hdir [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--mk-cd-hdir=*" {
		set hdir [string range [lindex $argv 0] 13 end]
		set argv [lrange $argv 1 end]
	} "-m" {
		set parallel 1
		set argv [lrange $argv 1 end]
	} "--parallel" {
		set parallel 1
		set argv [lrange $argv 1 end]
	} "-o" {
		set options [lindex $argv 1]
		set argv [lrange $argv 2 end]
	} "--options=*" {
		set options [string range [lindex $argv0] 10 end]
		set argv [lrange $argv 1 end]
	} "-*help" {
		usage $argv0
	} "--" {
		set argv [lrange $argv 1 end]
		break
	} "-*" {
		send_error "***** Invalid options: [lindex $argv 0]\n"
		usage $argv0
	} default {
		break
	}
}

#
# parse left argv
#
if {$action == $op_ssh || $action == $op_rsh} {
	#
	# ssh: command [command-options]
	#
	if {[llength $argv] > 0} {
		set title [concat $title [lindex $argv 0]]
		set largs ""
		foreach v $argv { set largs [concat $largs $v] }
		lappend commands $largs
	}

	if {[llength $commands] > 1} {
		lappend commands "exit\n"
	}
} else {
	#
	# rsync|scp: source [...] target
	#
	if {[llength $argv] == 0} {
		usage $argv0
	} elseif {[llength $argv] == 1} {
		set sources [lindex $argv 0]
		set target "."
	} else {
		#
		# TODO: source mybe formated as [[user@]host[#port]:]file-or-dir
		#
		foreach i [lrange $argv 0 end-1] {
			set sources [concat $sources $i]
		}

		set target [lindex $argv end]
	}
}
	

#
# set trace level
#
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

#
# No command executing, then need enter interactive mode
#
# interactive:
#	0: ssh command directly.
#	1: login & send command(s), and exit.
#	2: interactive.
#
if {$action == $op_ssh} {
	#
	# only need when ssh
	#
	if {[llength $commands] > 0} {
		set interactive [expr {$is_command_file != 0}]
	} elseif {$interactive && [llength $hosts] > 0 && $parallel} {
		send_error "TODO: interact parallelly with multiple hosts\n"
		exit 1
	}
}

#
# global setting check and init
#
load_default_setting
check_default_setting

#
# dump setting
#
debug_output "           action: [action_to_string $action]"
debug_output "            debug: $debug"
debug_output "            hosts: $hosts"
debug_output "hosts-count-width: $hcount, $hcwidth, $hwidth"
debug_output "            ports: $ports"
debug_output "            users: $users"
debug_output "        passwords: $passwords"
debug_output "    default-users: $default_users"
debug_output "    default-ports: $default_ports"
debug_output "default_passwords: $default_passwords"
debug_output "         parallel: $parallel"
debug_output "         commands: $commands"
debug_output "            title: $title"
debug_output " is command files: $is_command_file"
debug_output "         interact: $interactive"
debug_output "          sources: $sources"
debug_output "           target: $target"
debug_output "          reverse: $reverse"
debug_output "         host-dir: $hdir"
debug_output "          options: $options"

#
# check args
#
if {1} {
	if {[llength $hosts] == 0} {
		send_user "No host specified.\n";
		exit 1
	}
}

#
# setup traps
#
trap { 
	send_user "Catch a signal, stop all children and exit.\n"
	foreach sid $spawnids {
		close -i $sid
#		kill ???
#		wait -i $sid
	}

	exit 1
} { INT TERM HUP } 

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
		set maxretry 20
		if {$action == $op_rsync || $action == $op_scp || $action == $op_rcp} { set maxretry 3 }

		for {set j 0} {$j < 20 && $retry == 1} {incr j} {
			call_ssh_host $host
			if {$action == $op_ssh || $action == $op_rsh} {
				set retry [do_expect_loop]
			} elseif {$action == $op_scp \
				|| $action == $op_rsync \
					|| $action == $op_rcp} {
				set retry [do_rsync_loop]
			}
		}

		if {$retry == 1 && $action == $op_ssh} {
			send_user "$host: something wrong?\r\n"
			send_user "$host: why always response \"tcsetattr failed in ssh_rl_set_tty_modes_for_fd: fd 1: Interrupted system call.\"\r\n";
		}
	}
} else {
	set_xterm_title $title

	foreach i $hosts {
		set host [lindex [split $i] 0]
		call_ssh_host $host
	} 

	if {$action == $op_ssh || $action == $op_rsh} {
		set retry [do_expect_loop]
		# TODO: how retry
	} elseif {$action == $op_scp \
		|| $action == $op_rsync \
			|| $action == $op_rcp} {
		set retry [do_rsync_loop]
		# TODO: how retry
	}
}

set_xterm_title "what..."
exit 0

