--TEST--
P4_Map - Test P4_Map instance methods.
--ARGS--
-c tests/php.ini
--FILE--
<?php
$map = new P4_Map();
$map->insert("//depot/main/...      //client/...");
var_dump($map->as_array());
$map->insert("-//depot/main/something/...", "-//client/something/...");
var_dump($map->as_array());
var_dump($map->count());
var_dump($map->is_empty());
var_dump($map->translate("//depot/main/foo/bar.txt"));
var_dump($map->translate("//depot/main/something/foo"));
var_dump($map->includes("//depot/main/something/foo"));
var_dump($map->includes("//depot/main/foo/bar.txt"));
?>
--EXPECTF--
array(1) {
  [0]=>
  string(29) "//depot/main/... //client/..."
}
array(2) {
  [0]=>
  string(29) "//depot/main/... //client/..."
  [1]=>
  string(50) "-//depot/main/something/... //client/something/..."
}
int(2)
bool(true)
string(20) "//client/foo/bar.txt"
NULL
bool(false)
bool(true)
