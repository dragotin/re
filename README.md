# re - Work Reports Made Easy

re is your tool to organize weekly work reports which employees often have
to write to their management. The reports are kept in normal text
files and are stored in a simple file tree at a defined place on 
the file system.

## How to Use
re is a command line utility. If you type 're' on the commandline, it 
opens $EDITOR with the work report of the current week. Using command 
line options, re opens other weeks for you.

The format of the reports you want to write is completely up to you, yet
re has a default that uses the [RED] - [AMBER] - [GREEN] format, that 
seems to be widely used in IT industry. Items posted under [RED] are critical 
and require help from your manager, items under [AMBER] are not running well,
but are kept under control by you and [GREEN] is things you did successfully.

## Configuration

To configure re, copy the file rerc.template from the source
directory to $HOME/.rerc and edit the data within accordingly. You 
should find sufficient documentation of the available options in the
template file.

Be careful with the syntax, the rerc must contain valid perl
hash code, ie. value => key, separated by colons.

In the rerc there is a template for new report files. The strings
SENDER, WEEK and YEAR are replaced automatically with the according 
values.

## Synchronization

To make re useful if you work with more than one computer, ie. an office 
desktop computer and a laptop, re can use an ownCloud server to synchronize 
the work reports. It just syncs  before and after an edit of the work 
report automatically. That way you always edit the most recent version, and 
save your edit immediately on the ownCloud.

For that, the ownCloud sync client package must be installed. re uses a 
tool called owncloudcmd that comes with the ownCloud sync client.

Check the configuration template for the ownCloud sync options.

Attention: To not to have to deal with credentials, re calls `owncloudcmd`
with option `-n`. That tells the tool to read the ownCloud credentials
from .netrc. You have to configure it there.

## Options

re can be influenced by the following command line options:

* `-l` opens the report from last week.
* `-w <number>` opens the week number of the current year.
* `-s` opens the work report in preferred email client for sending. 
* `-o` syncs the weekly report to ownCloud before and after editing.

## Contribute

If you want to contribute to re, just fork it here and post pull 
requests. For bug reports, please use the github issue tracker.

Have fun and make your manager happy!

