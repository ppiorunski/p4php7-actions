--TEST--
P4::run - Test that specmgr prevents trashing of fstat keys
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
$p4->client = CLIENT_ONE_NAME;
$file = CLIENT_ONE_ROOT . DIRECTORY_SEPARATOR . 'file.txt';
touch($file);
$fp = fopen($file, "w");
fwrite($fp, "A simple submit...");
fclose($fp);
$p4->run('add', '//' . CLIENT_ONE_NAME . '/file.txt');
$p4->run_submit("-d", "Test submit", $file);

$p4->client = CLIENT_TWO_NAME;
$p4->run('sync', '-f');
$p4->run('edit', '//' . CLIENT_TWO_NAME . '/file.txt');

$p4->client = CLIENT_ONE_NAME;
$tags = $p4->run('fstat', '-Oail', '//' . CLIENT_ONE_NAME . '/file.txt');
var_dump($tags);

$p4->client = CLIENT_TWO_NAME;
$p4->run('revert', '//' . CLIENT_TWO_NAME . '/file.txt');

$p4->disconnect();
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
array(1) {
  [0]=>
  array(16) {
    ["depotFile"]=>
    string(16) "//depot/file.txt"
    ["clientFile"]=>
    string(%d) "%s"
    ["isMapped"]=>
    string(0) ""
    ["headAction"]=>
    string(3) "add"
    ["headType"]=>
    string(4) "text"
    ["headTime"]=>
    string(%d) "%d"
    ["headRev"]=>
    string(%d) "%d"
    ["headChange"]=>
    string(%d) "%d"
    ["headModTime"]=>
    string(%d) "%d"
    ["haveRev"]=>
    string(%d) "%d"
    ["otherOpen"]=>
    array(1) {
      [0]=>
      string(%d) "tester@%s"
    }
    ["otherAction"]=>
    array(1) {
      [0]=>
      string(4) "edit"
    }
    ["otherChange"]=>
    array(1) {
      [0]=>
      string(7) "default"
    }
    ["otherOpens"]=>
    string(1) "1"
    ["fileSize"]=>
    string(%d) "%d"
    ["digest"]=>
    string(%d) "%s"
  }
}

