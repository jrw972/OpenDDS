eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
    & eval 'exec perl -S $0 $argv:q'
    if 0;

# -*- perl -*-

use Env (DDS_ROOT);
use lib "$DDS_ROOT/bin";
use Env (ACE_ROOT);
use lib "$ACE_ROOT/bin";
use PerlDDS::Run_Test;
use strict;

my $status = 0;
my $debuglevel = 0;

my $pub_opts = "-ORBDebugLevel $debuglevel -DCPSDebugLevel $debuglevel -DCPSBits 0";
my $sub_opts = "-DCPSTransportDebugLevel $debuglevel -ORBDebugLevel $debuglevel -DCPSDebugLevel $debuglevel -DCPSBits 0";
my $testcase = "";

if ($ARGV[0] eq 'group') {
  $testcase = "-q 2"; #default
}
elsif ($ARGV[0] eq 'topic') {
  $testcase = "-q 1";
}
elsif ($ARGV[0] eq 'instance') {
  $testcase = "-q 0";
}
elsif ($ARGV[0] ne '') {
  print STDERR "ERROR: invalid parameter $ARGV[0]\n";
  exit 1;
}

my $dcpsrepo_ior = "repo.ior";

unlink $dcpsrepo_ior;
unlink <*.log>;

my %configs = (
    'ir_tcp' => {
        'discovery' => 'info_repo',
        'file' => {
            'common' => {
                'DCPSGlobalTransportConfig' => 'tcp',
                'DCPSDebugLevel' => '0',
                'DCPSInfoRepo' => 'file://repo.ior',
                'DCPSChunks' => '20',
                'DCPSChunkAssociationMultiplier' => '10',
                'DCPSLivelinessFactor' => '80'
            },
            'transport/tcp' => {
                'transport_type' => 'tcp'
            },
            'config/tcp' => {
                'transports' => 'tcp'
            },
        }
    }
);

my $test = new PerlDDS::TestFramework(configs => \%configs, config => 'ir_tcp');

$test->setup_discovery("-ORBDebugLevel 1 -ORBLogFile DCPSInfoRepo.log ");

$test->process("subscriber", "subscriber", "$sub_opts -ORBVerboseLogging 1 $testcase");
$test->process("publisher", "publisher", $pub_opts);

$test->start_process("subscriber");
$test->start_process("publisher");

exit $test->finish(120);
