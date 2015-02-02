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
# Copyright 2014 Juergen Weigert <jw@onncloud.com>
#
# 2014-06-16, jw: --help output improved.
#                 command line usage added. Examples:
#  $ re completed reading the newspaper
#  $ re RED spilled some coffee
#
# 2014-11-24, jw: Section name parser improved:
#              sub find_sections() added. RED/AMBER/GREEN no longer hard coded.
#              Section name prefix with colon is sufficient, to match section.
#
# 2015-01-31, jw: PROPAGATE started.
#
# If we have a section name [SUPPORT], we can add a record to that section by saying 
#  $ re SUP: Help Carlos

use strict;
use Date::Calc qw(:all);
use Getopt::Std;
use User::pwent;
use Data::Dumper;

$Getopt::Std::STANDARD_HELP_VERSION++;
use vars qw ( $VERSION $opt_l $opt_w %vars $tmpl);
$VERSION = '1.2';

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

$vars{BULLET} = '-' unless defined $vars{BULLET};
$vars{SECTION_MARKER} = '[' unless defined $vars{SECTION_MARKER};
$vars{ADD_BLANK_LINES} = 0  unless defined $vars{ADD_BLANK_LINES};
$vars{DEFAULT_SECTION} = 'GREEN'  unless defined $vars{DEFAULT_SECTION};

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
    $t =~ s/\b$key\b/$value/gm;
  }
  return $t;
}

sub insert_record
{
  my ($file, $section, $record) = @_;

  $record = $vars{BULLET} . ' ' . $record unless $record =~ m{^[\-\+\*\s\Q$vars{BULLET}\E]};
  # print "section=$section, record=$record\n";

  my $done = 0;
  my @rag;
  open(my $ifd, "<", $file) or die "cannot open $file: $!\n";
  while (defined(my $line = <$ifd>)) {
    chomp $line;
    if ($done == 1) {
      push @rag, "" if $vars{ADD_BLANK_LINES} and $line !~ m{^\s*$};
      $done++; 
    }

    push @rag, $line;

    if ($line =~ m{^\s*\Q$vars{SECTION_MARKER}\E\s*\Q$section\E\b}) {
      push @rag, "" if $vars{ADD_BLANK_LINES};
      push @rag, " " . $record;
      $done = 1;
    }
  }
  close $ifd;
  unless ($done) {
    push @rag, "" if $vars{ADD_BLANK_LINES};
    push @rag, " " . $record;
  }
  my $text = join("\n", @rag);
  print "$text\n";
  open(my $ofd, ">", $file) or die "cannot open > $file: $!\n";
  print $ofd $text;
  close $ofd or die "write failed to $file: $!\n";
}

sub propagate_section
{
  my ($section, $prev, $file) = @_;
  my $text = "FIXME: propagate_section('$section','$prev') not impl.\n";
  if ( -f $prev )
    {
      # TODO: pull text from section
    }
  else
    {
      $text = "PROPAGATE ERROR: cannot open file '$prev' ($!).\nPlease see yourself for items that should be propagated here.\n";
    }
  insert_record($file, $section, $text);
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
my $prev = "$dir/cw_" . ($weekofyear-1) . ".txt";

$vars{WEEK} = $weekofyear;
$vars{YEAR} = $startyear;
$vars{TIMESTAMP} = scalar localtime;


unless( -e $file ) {
  unless(-e $dir) {
    mkdir "$dir"
  }

  if( open FILE, ">$file" ) {
    print FILE template( \%vars );
    close FILE;
    if ($vars{PROPAGATE})
      {
        for my $section (split(/[,\s]+/, $vars{PROPAGATE}))
	  {
	    propagate_section($section, $prev, $file);
	  }
      }
  } else {
    print STDERR "Failed to open report file to write: $!\n";
  }
}

if (@ARGV) {
  my $section = $vars{DEFAULT_SECTION};
  my $record = join(' ', @ARGV);
  my @sections = find_sections($file, $vars{SECTION_MARKER});
  my $section_pat = join('|', map { "\Q$_\E" } @sections);
  
  if ($record =~ m{^([A-Z_]+):})
    {
      my $sect = $1;
      my $section_pat2 = join("#", @sections);
      if ("#$section_pat2#" =~ m{#(\Q$sect\E.*?)#})
        {
	  # expand a section prefix to full name
	  $section = $1;
	  $record =~ s{^\Q$sect\E:?\s*}{};
	  print "section=$section\n";
	}
    }
  $section = $1 if $record =~ s{^($section_pat):?\s?}{};

  insert_record($file, $section, $record);
  exit 0;
}

my $vi = $ENV{EDITOR} || `which vim`;
chomp $vi;
print "+ $vi $file\n";
my @args = ($vi, $file);
system(@args);

sub find_sections
{
  my ($file, $marker) = @_;

  my %section = ();
  open(my $ifd, "<", $file) or die "cannot open $file: $!\n";
  while (defined(my $line = <$ifd>)) 
    {
      chomp $line;
      if ($line =~ m{^\s*\Q$vars{SECTION_MARKER}\E\b\s*([\w_]+)}) 
        {
          $section{$1}++;
	}
    }
  return ('RED', 'AMBER', 'GREEN') unless keys %section; 
  return (keys %section);
}

sub HELP_MESSAGE()
{
  my ($ofd, $p, $v, $s) = @_;
  print $ofd "\nUsage: $0 [-l | -w NN] [message ...]\n";
  print $ofd Getopt::Std::help_mess($s);
  print $ofd qq{
$0 -l opens the report from last week.
$0 -w <number> opens the week number of the current year.

If an optional message is provided, the message is added to 
one of the sections of the report. The default section is GREEN. 
If the first word of the message is an all uppercase RED, AMBER, 
or GREEN then this is used as a section name.
If the message text does not start with one of '*', '+', '-', or ' '
then a '* ' prefix will be added in attempt to produce a bullet list.

When called without a message, $0 will start an EDITOR.
};

}
