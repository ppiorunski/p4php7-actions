--TEST--
 Trace log
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

 $trace_file = __DIR__ . "/trace.log";

 $p4->set_trace($trace_file, "rpc=3");
 echo $p4->run_info()[0]["userName"]; 
 $p4->disconnect();
 echo "\n\nconnection closed\n\n";
 
 $file = fopen($trace_file,"r");
 while(!feof($file)) {
   $line = fgets($file);
   if( preg_match( "/Rpc dispatch/", $line ) ) {
     echo $line;
   }
 }
 fclose($file);
 ?>
--CLEAN--
 <?php
 require_once('teardown.inc');
 unlink( __DIR__ . "/trace.log" );
 ?>
--EXPECTF--
tester

connection closed

Rpc dispatch client-FstatInfo
Rpc dispatch release
