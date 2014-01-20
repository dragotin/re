#!/usr/bin/perl
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# This is a helper for the week report. Just call it to open the correct
# week file.
#
# Do not forget to edit the .rerc file in your home directory. Find a
# template for .rerc in the re source dir.
#
# Copyright 2011-2013 Klaas Freitag <freitag@opensuse.org>
#

use strict;
use Date::Calc qw(:all);
use Getopt::Std;
use User::pwent;

use vars qw ( $opt_l $opt_w %vars $tmpl);

# edit here:
my $path = "/home/john/weekly_reports";
$path = $ENV{HOME} . "/weekly_reports" if( $ENV{HOME} );

my $rcfile = "$ENV{HOME}/.rerc";

if( -r "$rcfile" ) {

    my %localvars = do  $rcfile;
    warn "Could not parse $rcfile: $!\n" unless %localvars;
    warn "Could not do $rcfile: $@\n" if $@;

    $path = $localvars{RE_BASE} if( $localvars{RE_BASE} );
    $tmpl = $localvars{TEMPLATE} if( $localvars{TEMPLATE} );
    %vars = %localvars;
}

if (!$vars{SENDER}) {
    my $pw = getpwnam($ENV{USER}) || die "Could not retrieve current user name!\n";
    my ($fullname) = split(/\s*,\s*/, $pw->gecos);
    $vars{SENDER} = "$fullname";
}

sub template( $ )
{
  my ($params) = @_;
  my $t =<<ENDL

Work report SENDER cw WEEK/YEAR

[RED]

[AMBER]

[GREEN]


ENDL
;

  $t = $tmpl if( $tmpl );

# never never never edit below this.

  while( my ($key, $value) = each(%$params)) {
    $t =~ s/$key/$value/gmi;
  }
  return $t;
}

### Main starts here

getopts('lw:');

my ($startyear, $startmonth, $startday) = Today();
my $weekofyear = (Week_of_Year ($startyear,$startmonth,$startday))[0];

if( $opt_l ) {
  $weekofyear--;
  if( $weekofyear == 0 ) {
    print STDERR "Better make holiday than writing work reports!\n";
    exit 1;
  }
}

if( $opt_w ) {
  $weekofyear = $opt_w;
}

unless( $weekofyear =~/^\d+$/ && 0+$weekofyear > 0 && 0+$weekofyear < 54 ) {
  print STDERR "Better give me a good week number, not $weekofyear!\n";
  exit 1;
}

# ===

my $reportFilename = "cw_" . $weekofyear . ".txt";
my $dir = "$path/$startyear";
my $file = "$dir/$reportFilename";

$vars{WEEK} = $weekofyear;
$vars{YEAR} = $startyear;

unless( -e $file ) {
  unless(-e $dir) {
    mkdir "$dir"
  }
  if( open FILE, ">$file" ) {
    print FILE template( \%vars );
    close FILE;
  } else {
    print STDERR "Failed to open report file to write: $!\n";
  }
}

my $vi = $ENV{EDITOR} || `which vim`;
chop $vi;
print " $vi $file\n";
my @args = ($vi, $file);
system(@args);


