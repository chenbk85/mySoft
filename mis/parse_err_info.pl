my $keys = ();
my $acts = 1293811200 * 1000;	# chicago Jan 1, 2011 00:00:00 (ms)

sub output_error_info($$$$$$$)
{
	my $bkey = shift;
	my $kinfo = shift;
	my $ktype = shift;
	my $locs_ok = shift;
	my $locs_ts = shift;
	my $locs_idx = shift;
	my $locs_chk = shift;

	my $file = "./$ktype/err.history";
	if ($kinfo->{TS} >= $acts) { $file = "./$ktype/err.active"; }

	open FP ">>", $file || die "can not open $file for writing: $!";
	printf FP "%s MD5=%s TS=%d ==> OK=%d TSX=%d IDX=%d CHK=%d %s\n", $bkey, $kinfo->{IKEY},
			$kinfo->{TS}, int(@{$locs_ok}), int(@{$locs_ts}), int(@{$locs_idx}), int(@{$locs_chk}), $kinfo->{MSG};
	if (int(@{$locs_ok}) > 0) {
		printf FP "\t OK: %s\n", $locs_ok->[0];
		my $j = int(@{$locs_ok});
		for (my $i = 0; $i < $j; ++$i)
		printf FP "\t     %s\n", $locs_ok->[$i];
	}
	if (int(@{$locs_ts}) > 0) {
		printf FP "\tTSX: %s\n", $locs_ts->[0];
		my $j = int(@{$locs_ts});
		for (my $i = 0; $i < $j; ++$i)
		printf FP "\t     %s\n", $locs_ts->[$i];
	}
	if (int(@{$locs_idx}) > 0) {
		printf FP "\tIDX: %s\n", $locs_idx->[0];
		my $j = int(@{$locs_idx});
		for (my $i = 0; $i < $j; ++$i)
		printf FP "\t     %s\n", $locs_idx->[$i];
	}
	if (int(@{$locs_chk}) > 0) {
		printf FP "\tCHK: %s\n", $locs_chk->[0];
		my $j = int(@{$locs_chk});
		for (my $i = 0; $i < $j; ++$i)
		printf FP "\t     %s\n", $locs_chk->[$i];
	}

	close(FP);	
}

sub output_right_info($$$$)
{
	my $bkey = shift;
	my $kinfo = shift;
	my $ktype = shift;
	my $locs_ok = shift;

	my $file = "./$ktype/ok.history";
	if ($kinfo->{TS} >= $acts) { $file = "./$ktype/ok.active"; }

	open FP ">>", $file || die "can not open $file for writing: $!";
	printf FP "%s MD5=%s TS=%d ==> OK=%d %s\n", $bkey, $kinfo->{IKEY},
			$kinfo->{TS}, int(@{$locs_ok}), $kinfo->{MSG};
	foreach my $i (@{$locs_ok}) {
		printf FP "\t%s\n", $i;
	}
}

sub process_keyinfo($$$)
{
	my $bk = shift;
	my $kinfo = shift;
	my $ktype = shift;

	my @locs_ok = [];	# ok
	my @locs_ts = [];	# only ts is different
	my @locs_idx = [];	# loc info only exists in index
	my @locs_chk = [];	# in chunk
	my $ecnt = 0;

	if (defined($kinfo->{MSG})) { 
		$kinfo->{MSG} = sprintf "ERR:%s", $kinfo->{MSG};
		++$ecnt;
	}

	foreach my $uk (keys %{$kinfo->{LOCS}) {
		if ($kinfo->{LOCS}->{$uk}->{I} == 1 and $kinfo->{LOCS}->{$uk}->{C} == 1) {
			if ($kinfo->{LOCS}->{$uk}->{ITID} == $kinfo->{LOCS}->{$uk}->{CTID}) {
				my $info = sprintf "%s TID=%d", $uk, $kinfo->{LOCS}->{$uk}->{ITID};
				push @locs_ok, $info;
			}
			elsif ($kinfo->{LOCS}->{$uk}->{ITID} > $kinfo->{LOCS}->{$uk}->{CTID}) {
				my $info = sprintf "%s ITID=%d, CTID=%d", $uk, 
						$kinfo->{LOCS}->{$uk}->{ITID}, $kinfo->{LOCS}->{$uk}->{CTID};
				push @locs_ts, $info;
			}
			else {
				my $info = sprintf "%s CTID=%d, ITID=%d", $uk, 
						$kinfo->{LOCS}->{$uk}->{CTID}, $kinfo->{LOCS}->{$uk}->{ITID};
				push @locs_ts, $info;
			}
		}
		elsif ($kinfo->{LOCS}->{$uk}->{I} == 0 and $kinfo->{LOCS}->{$uk}->{C} > 0) {
			my $info = sprintf "%s CTID=%d", $uk, $kinfo->{LOCS}->{$uk}->{CTID};
			push @locs_chk, $info
		}
		elsif ($kinfo->{LOCS}->{$uk}->{I} > 0) and $kinfo->{LOCS}->{$uk}->{C} == 0) {
			my $info = sprintf "%s ITID=%d", $uk, $kinfo->{LOCS}->{$uk}->{ITID};
			push @locs_idx, $info
		}
		else {
			if ($kinfo->{LOCS}->{$uk}->{I} > 0) {
				my $info = sprintf "%s ITID=%d, ICNT=%d", $uk, $kinfo->{LOCS}->{$uk}->{ITID}, 
							$kinfo->{LOCS}->{$uk}->{I};
				push @locs_idx, $info
			}

			if ($kinfo->{LOCS}->{$uk}->{C} > 0) {
				my $info = sprintf "%s CTID=%d, CCNT=%d", $uk, $kinfo->{LOCS}->{$uk}->{CTID},
							$kinfo->{LOCS}->{$uk}->{C};
				push @locs_chk, $info
			}
		}

		$ecnt += int(@locs_idx) + int(@locs_chk) + int(@locs_ts);
		if ($ecnt > 0 or int(@locs_ok) < 1) {
			output_error_info($uk, $kinfo, $ktype, \@locs_ok, \@locs_ts, \@locs_idx, \@locs_chk);
		}
		else {
			output_right_info($uk, $kinfo, $ktype, \@locs_ok);
		}
	}
}

#
# load & parse all location infors for error keys
#
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

	#
	# bkey -> { IKEY => md5-key,
	#	    TS => most recent timestamp,
	#	    MSG => error message,
	#	    LOCS => { (ip, ck, bn) => (I, ITID, C,CTID) }
	#	  }
	#

	my $uk = join('|',  $ip, $ck, $bn);
	if (defined($keys->{$bk}->{IKEY})) {
		if ($keys->{$bk}->{IKEY} ne $ik) {
			$keys->{$bk}->{MSG} = $keys->{$bk}->{MSG} . " NE-MD5-KEY";
			$keys->{$bk}->{IKEY} = $ik;
		}
	}
	else {
		$keys->{$bk}->{IKEY} = $ik;
	}

	if ($ts > $keys->{$bk}->{TS}) {
		$keys->{$bk}->{TS} = $ts;
	}

	if ($ic eq "I") {
		++$keys->{$bk}->{LOCS}->{$uk}->{I};
		$keys->{$bk}->{LOCS}->{$uk}->{ITID} = $ts;
	}
	elsif ($ic eq "C") {
		++$keys->{$bk}->{LOCS}->{$uk}->{C};
		$keys->{$bk}->{LOCS}->{$uk}->{CTID} = $ts;
	}
	else {
		$keys->{$bk}->{MSG} = $keys->{$bk}->{MSG} . " UKN-TYPE";
	}
}

#
# parse & output
#
foreach my $bk (keys %{$keys}) {
	my $kt = substr($bk, 32, 2);
	if ($kt eq "54") {
		# transaction
		process_keyinfo($bk, $keys->{$bk}, "transaction");
	}
	elsif ($kt eq "50") {
		# position
		process_keyinfo($bk, $keys->{$bk}, "position");
	}
	elsif ($bk eq "53") {
		# portfolio
		process_keyinfo($bk, $keys->{$bk}, "portfoliots");
	}
	else {
		# others
		process_keyinfo($bk, $keys->{$bk}, "other");
	}
}
