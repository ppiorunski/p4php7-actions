--TEST--
P4::expand_sequences - Verify disabling expand sequences works.
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
$p4->expand_sequences = false;
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
KEY: View0
KEY: Type
KEY: Backup

