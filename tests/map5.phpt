--TEST--
P4_Map - Test P4_Map case sensitivity
--ARGS--
-c tests/php.ini
--FILE--
<?php
$view1 = "//SentrySuite/... //swarm-3c90988a-be47-aabe-f581-cb74bfbd8ec1/SentrySuite/...";
$view2 = "//sentrysuite/... //sentrysuite/...";

$view1Map1 = new \P4_Map;
$view1Map1->set_case_sensitive( true );
$view1Map1->insert($view1);
var_dump($view1Map1->as_array());
$view2Map1 = new \P4_Map;
$view2Map1->set_case_sensitive( true );
$view2Map1->insert($view2);
var_dump($view2Map1->as_array());
$join1 = \P4_Map::join($view1Map1, $view2Map1);
var_dump($join1->as_array());
$join1 = \P4_Map::join($view2Map1, $view1Map1);
var_dump($join1->as_array());

$view1Map2 = new \P4_Map;
$view1Map2->set_case_sensitive( false );
$view1Map2->insert($view1);
var_dump($view1Map2->as_array());
$view2Map2 = new \P4_Map;
$view2Map2->set_case_sensitive( false );
$view2Map2->insert($view2);
var_dump($view2Map2->as_array());
$join2 = \P4_Map::join($view1Map2, $view2Map2);
var_dump($join2->as_array());
$join2 = \P4_Map::join($view2Map2, $view1Map2);
var_dump($join2->as_array());

--EXPECTF--
array(1) {
  [0]=>
  string(78) "//SentrySuite/... //swarm-3c90988a-be47-aabe-f581-cb74bfbd8ec1/SentrySuite/..."
}
array(1) {
  [0]=>
  string(35) "//sentrysuite/... //sentrysuite/..."
}
array(0) {
}
array(0) {
}
array(1) {
  [0]=>
  string(78) "//SentrySuite/... //swarm-3c90988a-be47-aabe-f581-cb74bfbd8ec1/SentrySuite/..."
}
array(1) {
  [0]=>
  string(35) "//sentrysuite/... //sentrysuite/..."
}
array(0) {
}
array(1) {
  [0]=>
  string(78) "//sentrysuite/... //swarm-3c90988a-be47-aabe-f581-cb74bfbd8ec1/SentrySuite/..."
}

