use Data::Dumper;

my $h = { "h" => "Hello", "a" => [0, 1, 2, 3] };
my $a = ['a', 'b'];
my $s = "hello";
my @A = (1,2,3,4);
my %H = ('A' => 0, "B" => 1);

print Dumper($h);
print Dumper($a);
print Dumper($s);
print Dumper(\@A);
print Dumper(\%H);
