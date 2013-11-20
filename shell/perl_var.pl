my $v1 = "hello";
my @a1 = ( "a", "b", "c" );
my %h1 = ( "A" => 1, "B" => 2 );
my $va = [ "1", "2", "3" ];
my $vh = { "1" => "A", "2" => "B" };

print "v1=${v1}\n";
print "a1=("; foreach my $i (@a1) { print "$i "; } print ")\n";
print "h1=("; foreach my $k (keys %h1) { print "$k -> ", $h1{$k}; } print ")\n";
print "va=("; foreach my $i (@{$va}) { print "$i "; } print ")\n";
print "vh=("; foreach my $k (keys %{$vh}) { print "$k -> ", $vh->{$k}; } print ")\n";
