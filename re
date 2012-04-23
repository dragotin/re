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
# Do not forget to edit the 
#  $path -> the path where the reports are saved
#  template -> The template for new files between teh ENDL s
#  $vars -> tags that should be replaced in the template such 
#           as the SENDER var
#
# Copyright 2011-2012 Klaas Freitag <freitag@opensuse.org>
#

use strict;
use Date::Calc qw(:all);
use Getopt::Std;

my %vars;
use vars qw ( $opt_l $opt_w );

# edit here:
my $path = "/home/john/weekly_reports";
$vars{SENDER} = "John Doe";

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

my $reportFilename = "cw_" . $startyear ."_" . $weekofyear . ".txt";
my $file = "$path/$reportFilename";

$vars{WEEK} = $weekofyear;
$vars{YEAR} = $startyear;

unless( -e $file ) {
  if( open FILE, ">$file" ) {
    print FILE template( \%vars );
    close FILE;
  } else {
    print STDERR "$!\n";
  }
}

my $vi = $ENV{EDITOR} || `which vim`;
chop $vi;
print " $vi $file\n";
my @args = ($vi, $file);
system(@args);


