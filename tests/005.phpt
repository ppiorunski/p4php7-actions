--TEST--
P4::run_* - Test shortcut functions
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

// test that run_login and run('login') are the same
$results = $p4->run_login("testing123");
var_dump($results);
$p4->input = "testing123";
$results2 = $p4->run("login");
var_dump($results2);

// test that run_password and run('passwd') are the same
$results = $p4->run_password("testing123", "testing321");
var_dump($results);
$p4->input = array("testing321", "testing123", "testing123");
$results2 = $p4->run("passwd");
var_dump($results2);

// test that logging in still works
$results = $p4->run_login("testing123");
var_dump($results);

// add some files so that we can test various file related commands
$file = CLIENT_TWO_ROOT . DIRECTORY_SEPARATOR . "file.c";
touch($file);
$p4->run("add", $file);

$file2 = CLIENT_TWO_ROOT . DIRECTORY_SEPARATOR . "file.h";
touch($file2);
$p4->run_add($file2);

$submit_results = $p4->run_submit("-d", "Adding files", "//...");
var_dump(sizeof($submit_results));
var_dump(array_key_exists('change', $submit_results[0]));
var_dump(array_key_exists('locked', $submit_results[0]));
var_dump(array_key_exists('depotFile', $submit_results[1]));
var_dump(array_key_exists('rev', $submit_results[1]));
var_dump(array_key_exists('action', $submit_results[1]));
var_dump(array_key_exists('depotFile', $submit_results[2]));
var_dump(array_key_exists('rev', $submit_results[2]));
var_dump(array_key_exists('action', $submit_results[2]));
var_dump(array_key_exists('submittedChange', $submit_results[3]));

var_dump($p4->run_fstat("//..."));

// Change files in client two so they are not read-only
chmod($file,0777);
chmod($file2,0777);

$p4->disconnect();
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
array(1) {
  [0]=>
  array(2) {
    ["User"]=>
    string(6) "tester"
    ["TicketExpiration"]=>
    string(5) "43200"
  }
}
array(1) {
  [0]=>
  array(2) {
    ["User"]=>
    string(6) "tester"
    ["TicketExpiration"]=>
    string(5) "43200"
  }
}
array(1) {
  [0]=>
  string(17) "Password updated."
}
array(1) {
  [0]=>
  string(17) "Password updated."
}
array(1) {
  [0]=>
  array(2) {
    ["User"]=>
    string(6) "tester"
    ["TicketExpiration"]=>
    string(5) "43200"
  }
}
int(4)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
array(2) {
  [0]=>
  array(10) {
    ["depotFile"]=>
    string(14) "//depot/file.c"
    ["clientFile"]=>
    string(%d) "%sfile.c"
    ["isMapped"]=>
    string(0) ""
    ["headAction"]=>
    string(3) "add"
    ["headType"]=>
    string(4) "text"
    ["headTime"]=>
    string(%d) "%d"
    ["headRev"]=>
    string(1) "1"
    ["headChange"]=>
    string(1) "1"
    ["headModTime"]=>
    string(%d) "%d"
    ["haveRev"]=>
    string(1) "1"
  }
  [1]=>
  array(10) {
    ["depotFile"]=>
    string(14) "//depot/file.h"
    ["clientFile"]=>
    string(%d) "%sfile.h"
    ["isMapped"]=>
    string(0) ""
    ["headAction"]=>
    string(3) "add"
    ["headType"]=>
    string(4) "text"
    ["headTime"]=>
    string(%d) "%d"
    ["headRev"]=>
    string(1) "1"
    ["headChange"]=>
    string(1) "1"
    ["headModTime"]=>
    string(%d) "%d"
    ["haveRev"]=>
    string(1) "1"
  }
}

