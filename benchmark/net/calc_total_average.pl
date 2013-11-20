my $head = 0;
my $time = "";
my $data = {};

sub print_average()
{
	if ($head == 0) {
		print "time\ttotal\tdns\tconnect\tsend\tfirst-byte\ttransfer\tclose\tcount\n";
		$head = 1;
	}

	foreach my $k (keys %{$data->{AVG}}) {
		$data->{AVG}->{$k} = $data->{AVG}->{$k} / $data->{COUNT};
	}

	printf "%s\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t%5.2f\t%d\n", $time,
		$data->{AVG}->{TOTAL}, $data->{AVG}->{DNS}, $data->{AVG}->{CONNECT}, 
		$data->{AVG}->{SEND}, $data->{AVG}->{BYTE0}, $data->{AVG}->{TRANSFER},
		$data->{AVG}->{CLOSE}, $data->{COUNT};
	$data = {};
}

while (<STDIN>) {
	chomp;
	if (/loop: \d+ ([\d\-]+ [\d:]+)/) {
		if ($time ne "") { print_average(); }
		$time = $1;
	}
	elsif (/^range/) {
		# ignore
	}
	elsif (/^\d+\[/) {
		my @fds = split;
		#my $range = $fds[0,1];
		my $count = $fds[2];
		#my $percent = $fds[3];
		my $total = $fds[4];
		my $dns = $fds[5];
		my $connect = $fds[6];
		my $send = $fds[7];
		my $byte0 = $fds[8];
		my $transfer = $fds[9];
		my $close = $fds[10];

		$data->{COUNT} += $count;
		$data->{AVG}->{TOTAL} += $count * $total;
		$data->{AVG}->{DNS} += $count * $dns;
		$data->{AVG}->{CONNECT} += $count * $connect;
		$data->{AVG}->{SEND} += $count * $send;
		$data->{AVG}->{BYTE0} += $count * $byte0;
		$data->{AVG}->{TRANSFER} += $count * $transfer;
		$data->{AVG}->{CLOSE} += $count * $close;
	}
}

if ($time ne "") { print_average(); }
