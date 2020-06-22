--TEST--
P4 Handler - Test using a streaming data handler
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
class MyReportHandler extends P4_OutputHandlerAbstract {   
    public function outputStat($d)    
    { 
        echo "stat report ";
        var_dump($d);
        return self::HANDLER_REPORT;
    }
}

class MyHandledHandler extends P4_OutputHandlerAbstract {   
    public function outputStat($d)    
    { 
        echo "stat handled ";
        echo $d['client'] . "\n";
        return self::HANDLER_HANDLED;
    }
}

var_dump($p4->handler);
$p4->handler = new MyReportHandler;
var_dump($p4->handler);

$clients = $p4->run('clients');
echo $clients[0]['client'] . "\n";
echo $clients[1]['client'] . "\n";

$p4->handler = new MyHandledHandler;
var_dump($p4->handler);

$clients = $p4->run('clients');
var_dump($clients);
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
NULL
object(MyReportHandler)#2 (0) {
}
stat report array(12) {
  ["client"]=>
  string(13) "test-client-1"
  ["Update"]=>
  string(%d) "%d"
  ["Access"]=>
  string(%d) "%d"
  ["Owner"]=>
  string(6) "tester"
  ["Options"]=>
  string(58) "noallwrite noclobber nocompress unlocked nomodtime normdir"
  ["SubmitOptions"]=>
  string(15) "submitunchanged"
  ["LineEnd"]=>
  string(5) "local"
  ["Root"]=>
  string(%d) "%s%eclient-one"
  ["Host"]=>
  string(%d) "%s"
  ["Type"]=>
  string(9) "writeable"
  ["Backup"]=>
  string(6) "enable"
  ["Description"]=>
  string(19) "Created by tester.
"
}
stat report array(12) {
  ["client"]=>
  string(13) "test-client-2"
  ["Update"]=>
  string(%d) "%d"
  ["Access"]=>
  string(%d) "%d"
  ["Owner"]=>
  string(6) "tester"
  ["Options"]=>
  string(58) "noallwrite noclobber nocompress unlocked nomodtime normdir"
  ["SubmitOptions"]=>
  string(15) "submitunchanged"
  ["LineEnd"]=>
  string(5) "local"
  ["Root"]=>
  string(%d) "%s%eclient-two"
  ["Host"]=>
  string(%d) "%s"
  ["Type"]=>
  string(9) "writeable"
  ["Backup"]=>
  string(6) "enable"
  ["Description"]=>
  string(19) "Created by tester.
"
}
test-client-1
test-client-2
object(MyHandledHandler)#3 (0) {
}
stat handled test-client-1
stat handled test-client-2
array(0) {
}
