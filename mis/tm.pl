use Time::Local; # 'timelocal_nocheck';

my $year = 2010;
my $mon = 11;
my $mday = 1;
my $hour = 10;
my $min = 5;
my $sec = 1;

sub date2ms($$)
{
	my @ymd = split(/-/, shift);
	my @hms = split(/[:,]/, shift);

	my $time = timelocal($hms[2], $hms[1], $hms[0], $ymd[2], $ymd[1] - 1, $ymd[0]);	
	return $time * 1000 + $hms[3];
}

my $t1 = date2ms("2010-05-16", "02:40:18,208");

print "time-local=$t1\n";
