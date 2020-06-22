--TEST--
P4::fetch_change() - Test fetch_change method.
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

// add a file
$p4->client = CLIENT_ONE_NAME;
$file = CLIENT_ONE_ROOT . DIRECTORY_SEPARATOR . "file.txt";
touch($file);
$p4->run("add", $file);
$p4->run_submit("-d", "Adding a file...", "//...");

// add some more files but do not submit them
$file2 = CLIENT_ONE_ROOT . DIRECTORY_SEPARATOR . "file2.c";
touch($file2);
$p4->run("add", $file2);
$file3 = CLIENT_ONE_ROOT . DIRECTORY_SEPARATOR . "file2.h";
touch($file3);
$p4->run("add", $file3);

// submit the change
try {
    $change = $p4->fetch_change();
    print_r($change);
    $change['Description'] = 'This is a description...';
    $change['Files'] = array(
        '//depot/file2.c',
        '//depot/file2.h'
    );
    $result = $p4->save_change($change);
    print_r($result);
} catch (P4_Exception $e) {
    print $e->getMessage();
}
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
Array
(
    [Change] => new
    [Client] => test-client-1
    [User] => tester
    [Status] => new
    [Description] => <enter description here>

    [Files] => Array
        (
            [0] => //depot/file2.c
            [1] => //depot/file2.h
        )

)
Array
(
    [0] => Change 2 created with 2 open file(s).
)
