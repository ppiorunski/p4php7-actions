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

$file1 = $root . DIRECTORY_SEPARATOR . "testsubmit1.txt";
$p4->run("add", $file1);
$fp1 = fopen($file1, 'w');
fwrite($fp1, "Testing run_submit 1 arg" . date("F d r") . "\n");
fclose($fp1);
$change = $p4->fetch_change();
$change['Description'] = "Testing run_submit 1 arg";
$results = $p4->run_submit($change);
var_dump($results);

$file2 = $root . DIRECTORY_SEPARATOR . "testsubmit2.txt";
$p4->run("add", $file2);
$fp2 = fopen($file2, 'w');
fwrite($fp2, "Testing run_submit 2 args" . date("F d r") . "\n");
fclose($fp2);
$change = $p4->fetch_change();
$change['Description'] = "Testing run_submit 2 arg";
$results = $p4->run_submit("-i", $change);
var_dump($results);

$file3= $root . DIRECTORY_SEPARATOR . "testsubmit3.txt";
$p4->run("add", $file3);
$fp3 = fopen($file3, 'w');
fwrite($fp3, "Testing run_submit 3 args" . date("F d r") . "\n");
fclose($fp3);
$change = $p4->fetch_change();
$change['Description'] = "Testing run_submit 3 arg";
$results = $p4->run_submit("-d", "Testing run_submit 3 args", '/...');
var_dump($results);

$file4= $root . DIRECTORY_SEPARATOR . "testsubmit4.txt";
$p4->run("add", $file4);
$fp4 = fopen($file4, 'w');
fwrite($fp4, "Testing run_submit 2 args - with -d and comment" . date("F d r") . "\n");
fclose($fp4);
$change = $p4->fetch_change();
$change['Description'] = "Testing run_submit 2 args - with -d and comment";
$results = $p4->run_submit("-d", "Testing run_submit 2 args - with -d and comment");
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
    string(1) "1"
    ["openFiles"]=>
    string(1) "1"
    ["locked"]=>
    string(1) "1"
  }
  [1]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit1.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [2]=>
  array(1) {
    ["submittedChange"]=>
    string(1) "1"
  }
}
array(3) {
  [0]=>
  array(3) {
    ["change"]=>
    string(1) "2"
    ["openFiles"]=>
    string(1) "1"
    ["locked"]=>
    string(1) "1"
  }
  [1]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit2.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [2]=>
  array(1) {
    ["submittedChange"]=>
    string(1) "2"
  }
}
array(3) {
  [0]=>
  array(2) {
    ["change"]=>
    string(1) "3"
    ["locked"]=>
    string(1) "1"
  }
  [1]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit3.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [2]=>
  array(1) {
    ["submittedChange"]=>
    string(1) "3"
  }
}
array(3) {
  [0]=>
  array(2) {
    ["change"]=>
    string(1) "4"
    ["locked"]=>
    string(1) "1"
  }
  [1]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit4.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [2]=>
  array(1) {
    ["submittedChange"]=>
    string(1) "4"
  }
}
