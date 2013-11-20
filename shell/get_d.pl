sub get_double($)
{
	my $s = shift;

	if (!defined($s)) {
		return undef;
	}
	elsif ($s =~ /^\s*\(([0-9\.]+)\)\s*$/) {
		return -$1;
	}
	elsif ($s =~ /^\s*\-?[0-9\.]+\s*$/) {
		return $s;
	}
	
	print "invalid: $s\n";
	return undef;
}

my $d1 = get_double(undef);
my $d2 = get_double("(123.3)");
my $d3 = get_double("-23");
my $d4 = get_double("!DIV0");

print "d1=$d1, d2=$d2, d3=$d3, d4=$d4\n";
printf "%f, %f, %f, %f\n", $d1, $d2, $d3, $d4;

sub div($$)
{
	my $a = shift;
	my $b = shift;
	my $ret;
	eval { $ret=$a/$b; };# warn $@ if $@;
	return $ret;
#	print "error\n"; return undef;
}

my $d5 = div(10, $d1);
my $d6 = div(10,2);
my $d7 = div(11,0);
print "d5=$d5, d6=$d6, d7=$d7\n";

my @arr = (1,2,3);
my $c1 = int(@arr);
my $c2 = $#arr;
print "c1=$c1, c2=$c2\n";
