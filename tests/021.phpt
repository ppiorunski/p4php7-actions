--TEST--
P4::fetch_remote - Test remote server spec
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
$remote = $p4->fetch_remote("myremote");
$remote['Description'] = 'Setting a remote specification';
$remote['DepotMap'] = '\t//depot/...\t//depot/...';
$results = $p4->save_remote($remote);
var_dump($results);
$p4->disconnect();
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
array(1) {
  [0]=>
  string(22) "Remote myremote saved."
}			 
