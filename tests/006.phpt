--TEST--
P4::run_resolve - Test the resolve / resolver. 
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

// this test requires that two clients be present: test-client and 
// test-client2. Both are needed in order to simulate a resolve. 

function createFileInClientOne($file) {
    global $p4;
    // create file and submit 
    $fileName = CLIENT_ONE_ROOT . DIRECTORY_SEPARATOR . $file;
    $fp = fopen($fileName, "w");
    fwrite($fp, "Testing " . date("F d r") . "\n");
    fclose($fp);
    $p4->client = CLIENT_ONE_NAME;
    $results = $p4->run("add", $fileName);
    print "Sizeof add Results: " . sizeof($results) . "\n";
    var_dump(isset($results[0]['depotFile']));
    var_dump(isset($results[0]['clientFile']));
    var_dump(isset($results[0]['workRev']));
    var_dump(isset($results[0]['action']));
    var_dump(isset($results[0]['type']));
    $results = $p4->run_submit("-d", "Submitting test file...", $fileName);
    print "Sizeof run_submit Results: " . sizeof($results) . "\n";
    var_dump(isset($results[0]['change']));
    var_dump(isset($results[0]['locked']));
    var_dump(isset($results[1]['depotFile']));
    var_dump(isset($results[1]['rev']));
    var_dump(isset($results[1]['action']));
    var_dump(isset($results[2]['submittedChange']));
    return $fileName;
}

function syncBothClients() {
    global $p4;
    $p4->client = CLIENT_ONE_NAME;
    $p4->run("sync", "-f");
    $p4->client = CLIENT_TWO_NAME;
    $p4->run("sync", "-f");
}

function editAndSubmitFileInClientOne($fileName) {
    global $p4;
    $p4->client = CLIENT_ONE_NAME;
    $p4->run("edit", $fileName);
    $fp = fopen($fileName, "a");
    fwrite($fp, "Testing " . date("F d r") . "\n");
    fclose($fp);
    $p4->run_submit("-d", "Testing resolve", $fileName);
}

function editAndSubmitFileInClientTwo($file) {
    global $p4;
    $fileName = CLIENT_TWO_ROOT . DIRECTORY_SEPARATOR . $file;
    $p4->client = CLIENT_TWO_NAME;
    $p4->run("edit", $fileName);
    $fp = fopen($fileName, "a");
    fwrite($fp, "Testing from 2nd client " . date("F d r") . "\n");
    fclose($fp);
    try {
        $result = $p4->run_submit("-d", "Trying to submit from 2nd client", $fileName);
    } catch (P4_Exception $e) {}    
}


$fileName = createFileInClientOne("testresolve.txt");
syncBothClients();
editAndSubmitFileInClientOne($fileName);
editAndSubmitFileInClientTwo("testresolve.txt");

$result = $p4->run_resolve("-at");
print "Sizeof run_resolve Results: " . sizeof($result) . "\n";
var_dump(isset($result[0]['clientFile']));
var_dump(isset($result[0]['fromFile']));
var_dump(isset($result[0]['startFromRev']));
var_dump(isset($result[0]['endFromRev']));
var_dump(isset($result[1]['toFile']));
var_dump(isset($result[1]['how']));
var_dump(isset($result[1]['fromFile']));

// create a resolver class
class MyResolver extends P4_Resolver {
    public function resolve($merge_data) {
        if ($merge_data->merge_hint != 'e') {
            return $merge_data->merge_hint;
        } else {
            return "s"; // skip, there's a conflict
        }
    }
}

$fileName = createFileInClientOne("testresolve2.txt");
syncBothClients();
editAndSubmitFileInClientOne($fileName);
editAndSubmitFileInClientTwo("testresolve2.txt");

$resolver = new MyResolver();
$result = $p4->run_resolve($resolver);
var_dump($result[1]);
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
Sizeof add Results: 1
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Sizeof run_submit Results: 3
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Sizeof run_resolve Results: 2
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Sizeof add Results: 1
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Sizeof run_submit Results: 3
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
string(56) "Diff chunks: 0 yours + 0 theirs + 0 both + 1 conflicting"