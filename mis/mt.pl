while (<STDIN>) {
	if (/<fld id="([\d\w]+)"/) {
		print "$1: ";
	}
	elsif (/<columntype>([\d\w]+)/) {
		print "$1\n";
	}
}
