--TEST--
P4::set_evar and P4::get_evar - Tests getting and setting of extended arguments in the P4 class.
--ARGS--
-c tests/php.ini
--FILE--
<?php
$p4 = new P4();

$p4->set_evar("foo", "bar");
var_dump($p4->get_evar("foo"));
?>
--EXPECTF--
string(3) "bar"
