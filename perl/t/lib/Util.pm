# Utility class for webauth tests.
#
# Written by Jon Robertson <jonrober@stanford.edu>
# Parts from Russ Allbery <rra@stanford.edu>
# Copyright 2010
#     The Board of Trustees of the Leland Stanford Junior University
#
# See LICENSE for licensing terms.

package Util;
require 5.006;

use strict;
use vars qw(@ISA @EXPORT $VERSION);

use WebAuth qw (:const);

# This version should be increased on any code change to this module.  Always
# use two digits for the minor version with a leading zero if necessary so
# that it will sort properly.
$VERSION = '0.03';

use Exporter ();
@ISA    = qw(Exporter);
@EXPORT = qw(contents get_userinfo remctld_spawn remctld_stop create_keyring
    getcreds);

##############################################################################
# General utility functions
##############################################################################

# Returns the username and password from a file that contains them both,
# each on one line.
sub get_userinfo  {
    my ($file) = @_;
    open (FILE, '<', $file) or die "cannot open $file: $!\n";
    my $username = <FILE>;
    my $password = <FILE>;
    close FILE;
    chomp ($username, $password);
    return ($username, $password);
}

# Returns the one-line contents of a file as a string, removing the newline.
sub contents {
    my ($file) = @_;
    open (FILE, '<', $file) or die "cannot open $file: $!\n";
    my $data = <FILE>;
    close FILE;
    chomp $data;
    return $data;
}

##############################################################################
# User test configuration
##############################################################################

# Set up the database configuration parameters.  Use a local SQLite database
# for testing by default, but support t/data/test.database as a configuration
# file to use another database backend.
sub db_setup {
    if (-f 't/data/test.database') {
        open (DB, '<', 't/data/test.database')
            or die "cannot open t/data/test.database: $!";
        my $driver = <DB>;
        my $info = <DB>;
        my $user = <DB>;
        my $password = <DB>;
        chomp ($driver, $info);
        chomp $user if $user;
        chomp $password if $password;
        $Wallet::Config::DB_DRIVER = $driver;
        $Wallet::Config::DB_INFO = $info;
        $Wallet::Config::DB_USER = $user if $user;
        $Wallet::Config::DB_PASSWORD = $password if $password;
    } else {
        $Wallet::Config::DB_DRIVER = 'SQLite';
        $Wallet::Config::DB_INFO = 'wallet-db';
        unlink 'wallet-db';
    }
}

##############################################################################
# Kerberos utility functions
##############################################################################

# Given a keytab file and a principal, try authenticating with kinit.
sub getcreds {
    my ($file, $principal) = @_;
    my @commands = (
        "kinit -k -t $file $principal >/dev/null 2>&1 </dev/null",
        "kinit -t $file $principal >/dev/null 2>&1 </dev/null",
        "kinit -T /bin/true -k -K $file $principal >/dev/null 2>&1 </dev/null",
    );
    for my $command (@commands) {
        if (system ($command) == 0) {
            return 1;
        }
    }
    return 0;
}

# Given keytab data and the principal, write it to a file and try
# authenticating using kinit.
sub keytab_valid {
    my ($keytab, $principal) = @_;
    open (KEYTAB, '>', 'keytab') or die "cannot create keytab: $!\n";
    print KEYTAB $keytab;
    close KEYTAB;
    $principal .= '@' . $Wallet::Config::KEYTAB_REALM
        unless $principal =~ /\@/;
    my $result = getcreds ('keytab', $principal);
    if ($result) {
        unlink 'keytab';
    }
    return $result;
}

# Create a keyring file for use by the server.
sub create_keyring {
    my ($fname) = @_;
    return if -f $fname;

    my $key = WebAuth::key_create (WebAuth::WA_AES_KEY,
                                   WebAuth::random_key (WebAuth::WA_AES_128));
    my $ring = WebAuth::Keyring->new (32);
    my $curr = time();
    $ring->add ($curr, $curr, $key);
    $ring->write_file ($fname);
}

##############################################################################
# remctld handling
##############################################################################

# Start remctld with the appropriate options to run our fake keytab backend.
# Takes the path to remctld, the principal it uses as its server principal,
# the keytab it uses for authentication, and the configuration file it should
# load.
sub remctld_spawn {
    my ($path, $principal, $keytab, $config) = @_;
    unlink 'test-pid';
    my @command = ($path, '-m', '-p', 14373, '-s', $principal, '-P',
                   'test-pid', '-f', $config, '-S', '-F', '-k', $keytab);
    print "Starting remctld: @command\n";
    my $pid = fork;
    if (not defined $pid) {
        die "cannot fork: $!\n";
    } elsif ($pid == 0) {
        open (STDERR, '>&STDOUT') or die "cannot redirect stderr: $!\n";
        exec (@command) or die "cannot exec $path: $!\n";
    } else {
        my $tries = 0;
        while ($tries < 10 && ! -f 'test-pid') {
            select (undef, undef, undef, 0.25);
        }
    }
}

# Stop the running remctld process.
sub remctld_stop {
    open (PID, '<', 'test-pid') or return;
    my $pid = <PID>;
    close PID;
    chomp $pid;
    kill 15, $pid;
}