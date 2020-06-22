--TEST--
P4::extraTag - Test extraTag fields in spec.
--ARGS--
-c tests/php.ini
--SKIPIF--
<?php
include_once 'helper.inc';
requireMinP4d("2010", "2");
?>
--FILE--
<?php
include 'connect.inc';

// go to tagged mode
$p4->tagged = true;

$user = $p4->fetch_user($p4->user);
var_dump(
    in_array('passwordChange', $user['extraTag'])
);


?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
bool(true)
