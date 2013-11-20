my $keys = ();

while (<STDIN>) {
	my $raw = $_;
	chomp;
	my @fs = split /\|/;
	my $bkey = $fs[0];
	my $ip = $fs[1];
	my $ck = $fs[2];
	my $bk = $fs[3];
	my $tid = $fs[4];
	my $ikey = $fs[5];
	my $ic = $fs[6];

	my $ukey = $bkey;
	if ($ic eq "I") {
		my $i = @{$keys->{$ukey}->{I}};
		$keys->{$ukey}->{I}->[$i]->{IP} = $ip;
		$keys->{$ukey}->{I}->[$i]->{CK} = $ck;
		$keys->{$ukey}->{I}->[$i]->{BK} = $bk;
		$keys->{$ukey}->{I}->[$i]->{TID} = $tid;
		$keys->{$ukey}->{I}->[$i]->{IKEY} = $ikey;
	}
	elsif ($ic eq "C") {
		my $i = @{$keys->{$ukey}->{I}};
		$keys->{$ukey}->{I}->[$i]->{IP} = $ip;
		$keys->{$ukey}->{I}->[$i]->{CK} = $ck;
		$keys->{$ukey}->{I}->[$i]->{BK} = $bk;
		$keys->{$ukey}->{I}->[$i]->{TID} = $tid;
		$keys->{$ukey}->{I}->[$i]->{IKEY} = $ikey;
	}
}

foreach my $ukey (keys %{$keys}) {
	printf "K: %s\n", $ukey;
	printf "\tIndex:\n";
	foreach my $i (@{$keys->{$ukey}->{I}}) {
		printf "\t\t%s %s %s %s %s\n", $i->{IP}, $i->{CK}, $i->{BK}, $i->{TID}, $i->{IKEY};
	}
	printf "\tChunks:\n";
	foreach my $i (@{$keys->{$ukey}->{C}}) {
		printf "\t\t%s %s %s %s %s\n", $i->{IP}, $i->{CK}, $i->{BK}, $i->{TID}, $i->{IKEY};
	}
	printf "\n";
}


