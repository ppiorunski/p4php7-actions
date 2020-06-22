--TEST--
P4 Handler - Test get/set of p4 handler
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

/*
 * The p4->handler property takes a reference to a handler 
 * class. It therefore has to manage the reference count
 * when setting, returning and clearing the class reference.
 * We often do a number of var_dumps in a row as those can 
 * expose problems with this ref count tracking.
 */

class MyHandler implements P4_OutputHandlerInterface {
    public $foo = "bar";
    
    public function outputStat($d)    { return self::HANDLER_REPORT; }
    public function outputInfo($d)    { return self::HANDLER_REPORT; }
    public function outputText($d)    { return self::HANDLER_REPORT; }
    public function outputBinary($d)  { return self::HANDLER_REPORT; }
    public function outputWarning($d) { return self::HANDLER_REPORT; }
    public function outputError($d)   { return self::HANDLER_REPORT; }
}

class MyInvalidHandler {
    public $bing = "baz";
}

echo "starting handler\n";
var_dump($p4->handler);

echo "\ntrying to set bad handler\n";
$p4->handler = new MyInvalidHandler;
var_dump($p4->handler);

echo "\nassigning new class directly to handler\n";
$p4->handler = new MyHandler;
echo '1 '; var_dump($p4->handler);
echo '2 '; var_dump($p4->handler);
echo '3 '; var_dump($p4->handler);

echo "\ntrying to set bad handler\n";
$p4->handler = new MyInvalidHandler;
var_dump($p4->handler);

echo "\ntaking a copy of then clearing p4 handler\n";
$other = $p4->handler;
$p4->handler = null;
echo '1 '; var_dump($other);
echo '2 '; var_dump($other);
echo '3 '; var_dump($other);
echo '4 '; var_dump($other);

echo "\nassigning new class to local variable then to handler\n";
$handler     = new MyHandler;
$hash        = spl_object_hash($handler);
$p4->handler = $handler;

echo "\np4 handler object\n";
echo '1 '; var_dump($p4->handler);
echo '2 '; var_dump($p4->handler);
echo '3 '; var_dump($p4->handler);

$hash2 = spl_object_hash($p4->handler);

echo "\nconfirming hashes match and handler is still ok\n";
var_dump($hash === $hash2);
var_dump($p4->handler);

echo "\nunsetting local variable\n";
unset($handler);

echo "\np4 handler post unset local var\n";
var_dump($p4->handler);

echo "\nclear p4 handler as well\n";
$p4->handler = null;
var_dump($p4->handler);


?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
starting handler
NULL

trying to set bad handler
NULL

assigning new class directly to handler
1 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}
2 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}
3 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}

trying to set bad handler
object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}

taking a copy of then clearing p4 handler
1 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}
2 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}
3 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}
4 object(MyHandler)#2 (1) {
  ["foo"]=>
  string(3) "bar"
}

assigning new class to local variable then to handler

p4 handler object
1 object(MyHandler)#3 (1) {
  ["foo"]=>
  string(3) "bar"
}
2 object(MyHandler)#3 (1) {
  ["foo"]=>
  string(3) "bar"
}
3 object(MyHandler)#3 (1) {
  ["foo"]=>
  string(3) "bar"
}

confirming hashes match and handler is still ok
bool(true)
object(MyHandler)#3 (1) {
  ["foo"]=>
  string(3) "bar"
}

unsetting local variable

p4 handler post unset local var
object(MyHandler)#3 (1) {
  ["foo"]=>
  string(3) "bar"
}

clear p4 handler as well
NULL