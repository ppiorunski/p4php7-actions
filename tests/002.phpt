--TEST--
P4 Jobs - Using a hash to create specs.
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
$job = $p4->fetch_job();
$job['Description'] = 'This is a test job';
$results = $p4->save_job($job);
var_dump($results);
$p4->disconnect();
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
array(1) {
  [0]=>
  string(20) "Job job%d saved."
}
