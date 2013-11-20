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

	my $ukey = join('|',  $bkey, $ip, $ck, $bk);
	if ($ic eq "I") {
		++$keys->{$ukey}->{I};
		$keys->{$ukey}->{ITID} = $tid;
		$keys->{$ukey}->{IRAW} = $raw;
	}
	elsif ($ic eq "C") {
		++$keys->{$ukey}->{C};
		$keys->{$ukey}->{CTID} = $tid;
		$keys->{$ukey}->{CRAW} = $raw;
	}
}

foreach my $ukey (keys %{$keys}) {
	if ($keys->{$ukey}->{I} == 1 and $keys->{$ukey}->{C} == 1) {
		if ($keys->{$ukey}->{ITID} == $keys->{$ukey}->{CTID}) {
			printf "%s OK XTID=%s\n", $ukey, $keys->{$ukey}->{ITID};
		}
		else {
			printf "%s ERR-TID. ITID: %s, CTID: %s\n", $ukey, $keys->{$ukey}->{ITID}, $keys->{$ukey}->{CTID};
			print STDERR $keys->{$ukey}->{IRAW};
			print STDERR $keys->{$ukey}->{CRAW};
		}
	}
	else {
		printf "%s ERR-I/C. I=%d, ITID=%s, C=%d, CTID=%s\n", $ukey, $keys->{$ukey}->{I},
				$keys->{$ukey}->{ITID}, $keys->{$ukey}->{C}, $keys->{$ukey}->{CTID};
		print STDERR $keys->{$ukey}->{IRAW};
		print STDERR $keys->{$ukey}->{CRAW}; 
	}
}


