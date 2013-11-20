my $a = [1, 2];
foreach my $i (@{$a}) {
	print "$i ";
}; print "\n";

$a->[10] = 10;
foreach my $i (@{$a}) {
	print "$i ";
}; print "\n";
