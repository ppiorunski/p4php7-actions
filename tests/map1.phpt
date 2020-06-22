--TEST--
P4_Map - Test P4_Map class constructor and as_array method. 
--ARGS--
-c tests/php.ini
--FILE--
<?php
$map = new P4_Map();
var_dump($map->as_array());
$map = new P4_Map("//depot/main/... //client/...");
var_dump($map->as_array());
$map = new P4_Map(array(
    "//depot/main/...      //client/...",
    "-//depot/main/something/... -//client/something/..."
));
var_dump($map->as_array());
$map = new P4_Map(array(
    "//depot/main/...", "//client/..."
));
var_dump($map->as_array());
$map = new P4_Map("//depot/main/...", "//client/...");
var_dump($map->as_array());
?>
--EXPECTF--
array(0) {
}
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
array(2) {
  [0]=>
  string(33) "//depot/main/... //depot/main/..."
  [1]=>
  string(25) "//client/... //client/..."
}
array(1) {
  [0]=>
  string(29) "//depot/main/... //client/..."
}
