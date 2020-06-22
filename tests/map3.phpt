--TEST--
P4_Map - Test P4_Map reverse() method
--ARGS--
-c tests/php.ini
--FILE--
<?php
$map = new P4_Map(array(
    "//depot/main/...      //client/...",
    "-//depot/main/something/... -//client/something/..."
));
var_dump($map->as_array());
$map2 = $map->reverse();
var_dump($map2->as_array());

// make sure that $map is unchanged.
var_dump($map->as_array());

// test with a single map record
$map = new P4_Map("//depot/main/... //client/main/...");
var_dump($map->as_array());
$map2 = $map->reverse();
var_dump($map2->as_array());

?>
--EXPECTF--
array(2) {
  [0]=>
  string(29) "//depot/main/... //client/..."
  [1]=>
  string(50) "-//depot/main/something/... //client/something/..."
}
array(2) {
  [0]=>
  string(29) "//client/... //depot/main/..."
  [1]=>
  string(50) "-//client/something/... //depot/main/something/..."
}
array(2) {
  [0]=>
  string(29) "//depot/main/... //client/..."
  [1]=>
  string(50) "-//depot/main/something/... //client/something/..."
}
array(1) {
  [0]=>
  string(34) "//depot/main/... //client/main/..."
}
array(1) {
  [0]=>
  string(34) "//client/main/... //depot/main/..."
}
