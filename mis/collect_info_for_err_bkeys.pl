my $errBkeys = ();

sub load_err_bkeys($)
{
	my $file = shift;
	open fp, "<", $file || die "can not open $file to get all error bkeys: $!\n";
	while (<fp>) {
		chomp;
		++$errBkeys->{$_};
	}

	close(fp);

	my $count = keys %{$errBkeys};
	print STDERR "err kfile: $file, count: $count\n";
}

sub filter_raw_info($)
{
	my $file = shift;
	print STDERR "process $file ... ";

	open fp, "<", $file || die "can not open $file to filter raw location information: $!\n";
	while (<fp>) {
		my @fs = split /\|/;
		my $bk = $fs[0];
		my $ip = $fs[1];
		my $ck = $fs[2];
		my $bn = $fs[3];
		my $ts = $fs[4];
		my $ik = $fs[5];
		my $ic = $fs[6];

		if ($errBkeys->{$bk} > 0) {
			print STDOUT $_;
		}
	}

	close(fp);
	print STDERR "OK\n";
}

load_err_bkeys(shift @ARGV);

print "+1: $ARGV[1]\n";
print "-1: $ARGV[-1]\n";

foreach my $file (@ARGV) {
	filter_raw_info($file);
}

