--TEST--
P4 Binary Safety - Test binary input and args.
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

// establish a couple constants
$filename = CLIENT_TWO_ROOT . '/test';
$data     = str_repeat("deadbeefcafe\0", 3);

// setup a test file
file_put_contents($filename, "");
$p4->run("add", $filename);

echo "setting test value via argument and dumping\n";
$p4->run('attribute', '-n', 'testArg', '-v', $data, $filename);
$response = $p4->run('fstat', '-Oa', '//...');
echo "binary attribute set via argument is: " 
     . strlen($response[0]["openattr-testArg"]) 
     . " bytes\n";

echo "\nsetting test value via input and dumping\n";
$p4->input = $data;
$p4->run('attribute', '-n', 'testInput', '-i', $filename);

$response = $p4->run('fstat', '-Oa', '//...');
echo "binary attribute set via input is: " 
     . strlen($response[0]["openattr-testInput"]) 
     . " bytes\n";
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
setting test value via argument and dumping
binary attribute set via argument is: 39 bytes

setting test value via input and dumping
binary attribute set via input is: 39 bytes