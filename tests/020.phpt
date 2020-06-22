--TEST--
P4 Sequence Expansion - Verify gaps in sequences are filled in.
--ARGS--
-c tests/php.ini
--SKIPIF--
<?php
include_once 'helper.inc';
requireP4d();
?>
--FILE--
<?php
include 'connect.inc';

$jobspec = current($p4->run('jobspec', '-o'));
$jobspec['Fields'][] = "106 Test9 text 0 required";
$p4->input = $jobspec;
$p4->run('jobspec', '-i');

$job = $p4->fetch_job();
$job['Description'] = 'This is a test job';
$p4->save_job($job);

$spec = current($p4->run('jobs'));
print_r(array_keys($spec['Test']));
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
Array
(
    [0] => 0
    [1] => 1
    [2] => 2
    [3] => 3
    [4] => 4
    [5] => 5
    [6] => 6
    [7] => 7
    [8] => 8
    [9] => 9
)