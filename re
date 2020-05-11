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
# Copyright 2011-2016 Klaas Freitag <freitag@opensuse.org>
# Copyright 2014-2016 Juergen Weigert <jw@owncloud.com>
#
# 2014-06-16, jw: --help output improved.
#                 command line usage added. Examples:
#  $ re completed reading the newspaper
#  $ re RED spilled some coffee
#

use strict;
use Date::Calc qw(:all);
use Getopt::Std;
use User::pwent;
use File::Path qw(make_path);
use IPC::Run qw( run timeout );

$Getopt::Std::STANDARD_HELP_VERSION++;
use vars qw ( $VERSION $opt_l $opt_w $opt_s $opt_o $opt_c %vars );
$VERSION = '1.4';

#
# edit options in the personal config file in $HOME/.rerc
#

sub init_config()
{
    # Initialize the base path where the reports are stored:
    my $path = $ENV{HOME} . "/weekly_reports" if( $ENV{HOME} );

    # Check if the personal config file ~/.rerc exists and read
    # it. Otherwise check for the global one in /etc/rerc
    my $rcfile = "$ENV{HOME}/.rerc";

    $rcfile = '/etc/rerc' unless ( -r "$rcfile" );

    $rcfile = './rerc' unless ( -r "$rcfile" ); # last resort, dev only.

    if( -r "$rcfile" ) {
        my %localvars = do  $rcfile;
        warn "Could not parse $rcfile: $!\n" unless %localvars;
        warn "Could not do $rcfile: $@\n" if $@;

        $path = $localvars{RE_BASE} if( $localvars{RE_BASE} );
        $vars{TEMPLATE} = $localvars{TEMPLATE} if( $localvars{TEMPLATE} );
        %vars = %localvars;
    } else {
        print "WARN: Could not read config file!\n";
    }
   $vars{BASE_PATH} = $path;
 
    # check path and create if needed
    make_path( $path ) unless ( -e $path && -d $path );

    # More initializations
    $vars{BULLET} = '-' unless defined $vars{BULLET};
    $vars{ADD_BLANK_LINES} = 0  unless defined $vars{ADD_BLANK_LINES};
    
    # Try to read the sender value from the systems users settings
    # if they were not set in the config
    if (!$vars{SENDER}) {
        my $pw = getpwnam($ENV{USER}) || die "Could not retrieve current user name!\n";
        my ($fullname) = split(/\s*,\s*/, $pw->gecos);
        $vars{SENDER} = "$fullname";
    }
}

sub ownCloud_sync()
{
    my $owncloudcmd = $vars{OWNCLOUD_CMD_BINARY} || '/usr/bin/owncloudcmd';
    return 0 unless ( -x $owncloudcmd );

    my $result = 0;

    my $url = $vars{OWNCLOUD_WEBDAV_URL};
    if( $url ) {
        my @args;
        @args = ( $owncloudcmd, '-n', '--non-interactive' );
        push @args, '--trust' if( $vars{OWNCLOUD_TRUST_SSL} );
        push @args, $vars{BASE_PATH};
        push @args, $url;

        print "Syncing with ownCloud...\n";

        my $in;
        my $out;
        my $err;
        $result = run \@args, \$in, \$out, \$err, timeout( 120 );

        if( $vars{OWNCLOUD_DEBUG} ) {
            my $dbg = $out;
            $dbg .= $err;
            print STDERR "ownCloud cmd log:\n$dbg\n========\n";
        }
    } else {
        print STDERR "ERR: Can not sync to ownCloud because the URL is empty!\n";
    }
    return $result; # return success to repeat it afterwards
}


sub template( $ )
{
  my ($params) = @_;
  my $t =<<'ENDL'

Work report @SENDER@ cw @WEEK@/@YEAR@

[RED]

[AMBER]

[GREEN]


ENDL
;

  $t = $vars{TEMPLATE} if( $vars{TEMPLATE} );

  my $legacy = 1;
  while( my ($key, $value) = each(%$params)) {
    if ($t =~ s/\@$key\@/$value/gm) { $legacy=0; }
  }

  if ( $legacy ) {
    while( my ($key, $value) = each(%$params)) {
      $t =~ s/\b$key\b/$value/gm;
    }
  }
  return $t;
}

sub send_file($)
{
  my ($file) = @_;

  open(my $ifd, "<", $file) or die "cannot open $file: $!\n";
  my @content = <$ifd>;
  close $ifd;

  # First line(s) are assumed to be the subject
  my $subject = shift @content;
  if ($subject == "") {
    $subject = shift @content;
  }
  chomp $subject;

  my $body = join( //, @content);
  chomp $body;

  my @cmd;
  push @cmd, '--utf8';
  push @cmd, '--subject';
  push @cmd, $subject;
  push @cmd, '--body';
  push @cmd, $body;
  push @cmd, $vars{RECIPIENT} if defined( $vars{RECIPIENT} );

  my $ret = system('xdg-email', @cmd );

  return $ret;
}

sub edit_file($) {
  my ($file) = @_;

  if (@ARGV) {
    my $section = 'GREEN';
    my $record = join(' ', @ARGV);
    $section = $1 if $record =~ s{^(RED|AMBER|GREEN):?\s?}{};
    $record = $vars{BULLET} . ' ' . $record unless $record =~ m{^[\-\+\*\s\Q$vars{BULLET}\E]};

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

      if ($line =~ m{^\s*\[$section\]}) {
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
  } else {
    my $vi = $ENV{EDITOR} || `which vim`;
    chomp $vi;
    print "+ $vi $file\n";
    my @args = ($file);
    system($vi, @args);
  }
}

sub HELP_MESSAGE()
{
  my ($ofd, $p, $v, $s) = @_;
  print $ofd "\nUsage: $0 [-l | -w NN | -s | -o] [message ...]\n";
  print $ofd Getopt::Std::help_mess($s);
  print $ofd q{
$0 -l opens the report from last week.
$0 -w <number> opens the week number of the current year.
$0 -s opens the work report in preferred email client for sending. 
$0 -o syncs the weekly report to ownCloud before and after editing.

If an optional message is provided, the message is added to 
one of the sections of the report. The default section is GREEN. 
If the first word of the message is an all uppercase RED, AMBER, 
or GREEN then this is used as a section name.

If the message text does not start with one of '*', '+', '-', or ' '
then a '* ' prefix will be added in attempt to produce a bullet list.

When re is called without a message, $0 will start an editor as 
specified in the $EDITOR environment variable.

If re is called with the option -o, it will synchronize the entire
work report directory to ownCloud. This way, re can be used from 
more than one computer without hassle as ownCloud is the central
data hub. This setting requires special settings in the rerc file,
see the template for additional documentation of the values.

};

}

### Main starts here

getopts('lw:soc:');

# read the config file
init_config();

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

# === ownCloud sync:
if( $opt_o ) {
    $opt_o = ownCloud_sync();
}

my $reportFilename = "cw_" . $weekofyear . ".txt";
my $dir = "$vars{BASE_PATH}/$startyear";
my $file = "$dir/$reportFilename";

$vars{WEEK} = $weekofyear;
$vars{YEAR} = $startyear;
$vars{TIMESTAMP} = scalar localtime;

# If the year directory does not exist, create it.
make_path( $dir ) unless ( -e $dir && -d $dir );

# If the file does not exist, fill it with the template.
unless( -e $file ) {
  if( open FILE, ">$file" ) {
    print FILE template( \%vars );
    close FILE;
  } else {
    die "Failed to open report file to write: $!\n$file\n";
  }
}

if( $opt_s ) {
  send_file($file);
} else {
  edit_file($file);
  # === ownCloud sync, only do after edit again.
  if( $opt_o ) {
    $opt_o = ownCloud_sync();
  }
}

# end.
