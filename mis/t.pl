
if ("a" eq "A") {
	print "OK\n";
}

$a = [];
my $i = @{$a};
print "i=$i\n";
$a->[$i]->{O} = 0;

my $i = @{$a};
$a->[$i]->{O} = 1;

foreach my $k (@{$a}) {
	printf "%s\n", $k->{O};
}

my $a1 = [];
my @a2 = ();
my $h1 = {};
my %h2 = ();

print "a1=$a1, a2=$a2, h1=$h1, h2=$h2\n";
