--TEST--
P4::fetch_client() - Test modifying clients.
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

$p4->client = CLIENT_ONE_NAME;
$client = $p4->fetch_client();
print_r($client);

$client['Root'] = '/var/tmp/something';
$p4->save_client($client);

$client = $p4->fetch_client();
print_r($client);
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
Array
(
    [Client] => test-client-1
    [Update] => %d/%d/%d %d:%d:%d
    [Access] => %d/%d/%d %d:%d:%d
    [Owner] => tester
    [Host] => %s
    [Description] => Created by tester.

    [Root] => %sclient-one
    [Options] => noallwrite noclobber nocompress unlocked nomodtime normdir
    [SubmitOptions] => submitunchanged
    [LineEnd] => local
    [View] => Array
        (
            [0] => //depot/... //test-client-1/...
        )

    [Type] => writeable
    [Backup] => enable
)
Array
(
    [Client] => test-client-1
    [Update] => %d/%d/%d %d:%d:%d
    [Access] => %d/%d/%d %d:%d:%d
    [Owner] => tester
    [Host] => %s
    [Description] => Created by tester.

    [Root] => /var/tmp/something
    [Options] => noallwrite noclobber nocompress unlocked nomodtime normdir
    [SubmitOptions] => submitunchanged
    [LineEnd] => local
    [View] => Array
        (
            [0] => //depot/... //test-client-1/...
        )

    [Type] => writeable
    [Backup] => enable
)
