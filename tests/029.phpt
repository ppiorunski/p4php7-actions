--TEST--
 Stream comments test
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

 date_default_timezone_set("America/Vancouver");

 # Create stream depot
 $p4->input = $p4->run("depot", "-o", "-t", "stream", "STREAM_TEST")[0];
 $p4->run("depot", "-i");

 # Create a test stream
 $p4->input = $p4->run("stream", "-o", "-t", "mainline", "//STREAM_TEST/TEST")[0];
 $p4->run("stream", "-i");

 # Read back the stream and add comments
 $stream = $p4->run("stream", "-o", "//STREAM_TEST/TEST");
 $stream[0]["PathsComment"][0]= "## Added inline comment";
 $stream[0]["Paths"][1]= "";
 $stream[0]["PathsComment"][1]= "## Added new-line comment";
 $p4->input = $stream[0];

 # Load the stream with comments to the server and then read back.
 $p4->run("stream", "-i");
 var_dump($p4->run("stream", "-o", "//STREAM_TEST/TEST")[0]["PathsComment"]);

 # Remove depot and stream
 $p4->run("stream", "-d", "//STREAM_TEST/TEST");
 $p4->run("depot", "-d", "STREAM_TEST");

 $p4->disconnect();

 ?>
--CLEAN--
 <?php
 require_once('teardown.inc');
 ?>
--EXPECTF--
array(3) {
  [0]=>
  string(23) "## Added inline comment"
  [1]=>
  NULL
  [2]=>
  string(25) "## Added new-line comment"
}