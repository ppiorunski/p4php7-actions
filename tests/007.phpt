--TEST--
P4::run_submit - Test run_submit shortcut function.
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

date_default_timezone_set("America/Vancouver");

$p4->client = CLIENT_ONE_NAME;
$client = $p4->fetch_client();
$root = $client["Root"];
$file = $root . DIRECTORY_SEPARATOR . "testsubmit.txt";
$p4->run("add", $file);
$fp = fopen($file, 'w');
fwrite($fp, "Testing run_submit " . date("F d r") . "\n");
fclose($fp);
$change = $p4->fetch_change();
$change['Description'] = "Testing run_submit...";
$results = $p4->run_submit("-i", $change);
var_dump($results);
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
array(3) {
  [0]=>
  array(3) {
    ["change"]=>
    string(%d) "%d"
    ["openFiles"]=>
    string(%d) "%d"
    ["locked"]=>
    string(%d) "%d"
  }
  [1]=>
  array(3) {
    ["depotFile"]=>
    string(22) "//depot/testsubmit.txt"
    ["rev"]=>
    string(%d) "%d"
    ["action"]=>
    string(3) "add"
  }
  [2]=>
  array(1) {
    ["submittedChange"]=>
    string(%d) "%d"
  }
}

