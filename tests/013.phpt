--TEST--
P4::identify() - Test identify method.
--ARGS--
-c tests/php.ini
--SKIPIF--
<?php
include_once 'helper.inc';
requireP4d();
?>
--FILE--
<?php

// identify shouldn't require that we're connected to a server
// so don't include connect.inc
print P4::identify() . "\n";

// the identify method should return the same value whether it's
// called statically or as an instance method (PHP is quirky).
$p4 = new P4();
var_dump($p4->identify() === P4::identify());

// should be the same for tagged and untagged
$p4->tagged = false;
var_dump($p4->identify() === P4::identify());
?>
--EXPECTF--
Perforce - The Fast Software Configuration Management System.
Copyright 1995-20%d Perforce Software.  All rights reserved.%A
%A
Version of OpenSSL Libraries: %s %s %d %s %d
%A
%A
Rev. P4PHP/%s/%d.%a/%d (%d.%a API) (%d/%d/%d).

bool(true)
bool(true)
