use Data::Dumper;
use HTTP::Lite;
use HTTP::Request;
use URI::Escape;
use JSON;

my $URL = "http://s3qa.morningstar.com/v2/securities/suggestion02";
my @cases = ("ibm", "goog", "a", "b", "c", "f", "msft", "microsoft");

sub get_ac_result($$)
{
	my $q = shift;
	my $u = $URL . "?q=" . uri_escape($q);
	my $h = HTTP::Lite->new;
	my $r = $h->request($u) or die "Unable to request HTTP($u): $!";
	my $s = $h->body();
	my $j = JSON->new->utf8->decode($s);	
}

sub check_order($)
{
	my $j = shift;
	print Dumper($j);
	foreach my $data (@{$j->{"data"}}) {
		#printf "SecId: %s, OS01W: %s\n", $data->{"SecId"}, $data->{"OS01W"};
	}
}

foreach my $c (@cases) {
	my $r = get_ac_result($c);
	check_order($r);
}


#print Dumper(\@INC);
