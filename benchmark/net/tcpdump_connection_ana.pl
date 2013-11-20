use Time::Local; # 'timelocal_nocheck';

my $server = "172.28.14.254.1433";
my $max = 0.0;

#
# sessions->{STAT}->{CNT} = n(session count);
# sessions->{STAT}->{CLIENT-HOST.PORT}->{SYN} = 0 (after the first syn), 1(had received the first syn).
#			              ->{IDX} = session index 
#	  ->{DATA}->[idx] = ( dlg, dlg, ... )
#
my $sessions = {};

# 23118 03:21:26.890130 ==> 3.3ms
sub time2ms($)
{
	my $str = shift;
        my @hms = split(/[:\.]/, $str);

	# ymd does not make sense here.
	# we care time difference only.
        my $time = timelocal($hms[2], $hms[1], $hms[0], 1, 1, 2012); 
	# to xx.xx ms
        return $time * 1000.0 + $hms[3] / 1000.0;
}

sub process_one_dlg($$$$)
{
	my $tss = shift;
	my $src = shift;
	my $dst = shift;
	my $flg = shift;
	my $idx = 0;

	if ($flg eq "S" and $dst eq $server and $sessions->{$src}->{SYN} == 0) {
		#
		# this is the first syn (session starts)
		#
		$sessions->{STAT}->{$src}->{SYN} = 1;
		$idx = $sessions->{STAT}->{$src}->{IDX} = $sessions->{STAT}->{CNT}++;

		push @{$sessions->{DATA}->[$idx]}, $_;
	}
	elsif ($src eq $server) {
		$sessions->{STAT}->{$dst}->{SYN} = 0;	# reset it

		$idx = $sessions->{STAT}->{$dst}->{IDX};
		push @{$sessions->{DATA}->[$idx]}, $_;
	}
	elsif ($dst eq $server) {
		$sessions->{STAT}->{$src}->{SYN} = 0;	# reset it

		$idx = $sessions->{STAT}->{$src}->{IDX};
		push @{$sessions->{DATA}->[$idx]}, $_;
	}
	else {
		# ignore
		print STDERR "Non-related to $server: $_\n";
	}
}

sub scan_input()
{
	while (<STDIN>) {
		chomp;
		if (/([\d+:\.]+) IP ([\d+\.]+) > ([\d+\.]+): ([\w\d\.]+)/) {
			# tcpdump -nn 
			# 03:05:40.472640 IP 172.28.41.209.58850 > 172.28.14.254.1433: . ack 1 win 46 <nop,nop,timestamp 753750790 1683685073>
			my $tss = $1;
			my $src = $2;
			my $dst = $3;
			my $flg = $4;

			process_one_dlg($tss, $src, $dst, $flg);
		}
		elsif (/([\d+:\.]+) IP \(tos.*length: \d+\) ([\d+\.]+) > ([\d+\.]+): ([\w\d\.]+)/) {
			# tcpdump -nn -v
			# 03:36:16.904502 IP (tos 0x0, ttl  63, id 48359, offset 0, flags [DF], proto: TCP (6), length: 52) 172.28.14.254.1433 > 172.28.41.209.52866: ., cksum 0xee41 (correct), ack 2050 win 88 <nop,nop,timestamp 1685521878 755587499>
			my $tss = $1;
			my $src = $2;
			my $dst = $3;
			my $flg = $4;
	
			process_one_dlg($tss, $src, $dst, $flg);
		}
		elsif ($_ ne "") {
			# why here
			print STDERR "Unkown format: $_\n";
		}
	}
}

sub output_sessions()
{
	foreach my $dlg (@{$sessions->{DATA}}) {
		my @first = split(/\s+/, $sessions->{DATA}->[0]);
		my @last = split(/\s+/, $sessions->{DATA}->[-1]);

		my $ms = time2ms($last[0]) - time2ms($first[0]);
		if ($ms >= $max) {
			foreach my $msg (@{$dlg}) {
				print STDOUT "$msg\n";
			}

			printf STDOUT "\n**** session lasted %5.2fms ****\n\n\n", $ms;
		}
	}
}

if (@ARGV == 1) {
	$server = $ARGV[0];
}
elsif (@ARGV == 2) {
	$server = $ARGV[0];
	$max = $ARGV[1];
}
else {
	print STDERR "Usage: $ARGV0 server(ip.port) [ lasting-time(ms) ]\n";
	print STDERR "  When lasting-time is specified, it only prints sessions which last longer than that time.\n";
	print STDERR "  stdin must be the output of tcpdump run with flags of \"-nn [-v]\".\n";
	exit 1;
}

scan_input();
output_sessions();

