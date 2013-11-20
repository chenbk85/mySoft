my $keys = ();

while (<STDIN>) {
	chomp;
	my @fs = split /\|/;
	my $bk = $fs[0];
	my $ip = $fs[1];
	my $ck = $fs[2];
	my $bn = $fs[3];
	my $ts = $fs[4];
	my $ik = $fs[5];
	my $ic = $fs[6];

	my $uk = join('|',  $bk, $ip, $ck, $bn);
	$keys->{$uk}->{BKEY} = $bk;

	if ($ic eq "I") {
		++$keys->{$uk}->{I};
		$keys->{$uk}->{ITID} = $ts;
	}
	elsif ($ic eq "C") {
		++$keys->{$uk}->{C};
		$keys->{$uk}->{CTID} = $ts;
	}
}

foreach my $uk (keys %{$keys}) {
	if ($keys->{$uk}->{I} == 1 and $keys->{$uk}->{C} == 1) {
		if ($keys->{$uk}->{ITID} == $keys->{$uk}->{CTID}) {
			printf "%s OK TID=%s\n", $uk, $keys->{$uk}->{ITID};
		}
		else {
			printf "%s ERR-TID. ITID: %s, CTID: %s\n", $uk, $keys->{$uk}->{ITID}, $keys->{$uk}->{CTID};
			printf STDERR "%s\n", $keys->{$uk}->{BKEY};
		}
	}
	else {
		my $dup = "";
		if ($keys->{$uk}->{I} > 1 or $keys->{$uk}->{C} > 1) { $dup = ". DUP-I"; }

		printf "%s ERR-I/C. I=%d, ITID=%s, C=%d, CTID=%s%s\n", $uk, $keys->{$uk}->{I},
				$keys->{$uk}->{ITID}, $keys->{$uk}->{C}, $keys->{$uk}->{CTID},
				$dup;
		printf STDERR "%s\n", $keys->{$uk}->{BKEY};
	}
}

