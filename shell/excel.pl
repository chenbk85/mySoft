use Text::CSV;

my $file = $ARGV[0];
my $csv = Text::CSV->new({binary => 1});
open my $io, "<", $file or die "open $file failed: $!";
while (my $r = $csv->getline($io)) {
	for my $f (@$r) {
		printf "%s|", undef;
	}

	printf "\n";
}


