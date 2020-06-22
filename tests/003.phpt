--TEST--
P4::connect - Test connecting to a Perforce server and running info
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

// disconnect and connect again
var_dump($p4->disconnect()); // shouldn't return anything
var_dump($p4->connected());  // should be false
var_dump($p4->connect());    // should return true
var_dump($p4->connected());  // should return false

$p4->client = "test-client";
$info = $p4->run('info');

$keys = array(
    'userName', 'password', 'clientName', 'clientHost', 'clientAddress',
    'serverAddress', 'serverRoot', 'serverVersion', 'serverLicense'
);

foreach ($keys as $key) {
    var_dump(array_key_exists($key, $info[0]));
}

// verify that run('info') and run_info() return the same thing. 
$info2 = $p4->run_info();
var_dump(is_array($info));
var_dump(is_array($info2));
var_dump(is_array($info[0]));
var_dump(is_array($info2[0]));
var_dump(sizeof($info));
var_dump(sizeof($info[0]));

foreach ($keys as $key) {
    var_dump($info[0][$key] == $info2[0][$key]);
}
echo $info[0]['serverDate'] . " " . $info2[0]['serverDate'];
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
NULL
bool(false)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
int(%d)
int(%d)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
%d/%d/%d %d:%d:%d %s %s %d/%d/%d %d:%d:%d %s %s
