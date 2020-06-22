--TEST--
P4::fetch_client() - Output client spec. 
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
$p4->client = "test-client";
$spec = $p4->fetch_client();
foreach ($spec as $key => $value) {
    print "KEY: $key\n";
}
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
KEY: Client
KEY: Owner
KEY: Host
KEY: Description
KEY: Root
KEY: Options
KEY: SubmitOptions
KEY: LineEnd
KEY: View
KEY: Type
KEY: Backup

