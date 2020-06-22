--TEST--
P4::run_filelog - Test that filelog returns correct results
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

$fp = fopen(CLIENT_TWO_ROOT . '/file.txt', 'w');
fwrite($fp, "Blah");
fclose($fp);

$p4->run("add", CLIENT_TWO_ROOT . '/file.txt');
$p4->run_submit("-d", "Adding a file", CLIENT_TWO_ROOT . '/...');

mkdir(CLIENT_TWO_ROOT . '/tmp');
mkdir(CLIENT_TWO_ROOT . '/tmp2');

$fp = fopen(CLIENT_TWO_ROOT . '/tmp/file2.txt', 'w');
fwrite($fp, 'Blah');
fclose($fp);

$p4->run("add", CLIENT_TWO_ROOT . '/tmp/file2.txt');
$p4->run_submit("-d", "Adding a file", CLIENT_TWO_ROOT . '/...');

$p4->run("integ", CLIENT_TWO_ROOT . '/tmp/file2.txt', CLIENT_TWO_ROOT . '/tmp2/file2.txt');
$p4->run_submit("-d", "Integrating a file", CLIENT_TWO_ROOT . '/...');


$p4->run_edit(CLIENT_TWO_ROOT . '/file.txt');
$fp = fopen(CLIENT_TWO_ROOT . '/file.txt', 'a');
fwrite($fp, "Snah");
fclose($fp);
$p4->run_submit("-d", "Editing a file", CLIENT_TWO_ROOT . '/...');

try {
	$p4->exception_level = 2;
	$filelog = $p4->run_filelog('//...');
	// verify size of filelog results
    print "Sizeof filelog: " . sizeof($filelog) . "\n";
    // verify that each element is a P4_DepotFile object
    var_dump(get_class($filelog[0]));
    var_dump(get_class($filelog[1]));
    var_dump(get_class($filelog[2]));
    // verify depotFile fields
    var_dump($filelog[0]->depotFile);
    var_dump($filelog[1]->depotFile);
    var_dump($filelog[2]->depotFile);
    // verify that each P4_Revision has an array of P4_Revision objects
    print "Sizeof filelog[0]->revisions: " . sizeof($filelog[0]->revisions) . "\n";
    print "Sizeof filelog[1]->revisions: " . sizeof($filelog[1]->revisions) . "\n";
    print "Sizeof filelog[2]->revisions: " . sizeof($filelog[2]->revisions) . "\n";
    // verify P4_Revision objects
    var_dump(isset($filelog[0]->revisions[0]->action));
    var_dump(isset($filelog[1]->revisions[0]->action));
    var_dump(isset($filelog[2]->revisions[0]->action));
    var_dump(isset($filelog[0]->revisions[0]->change));
    var_dump(isset($filelog[1]->revisions[0]->change));
    var_dump(isset($filelog[2]->revisions[0]->change));
    var_dump(isset($filelog[0]->revisions[0]->client));
    var_dump(isset($filelog[1]->revisions[0]->client));
    var_dump(isset($filelog[2]->revisions[0]->client));
    var_dump(isset($filelog[0]->revisions[0]->depotFile));
    var_dump(isset($filelog[1]->revisions[0]->depotFile));
    var_dump(isset($filelog[2]->revisions[0]->depotFile));
    // verify that filelog[1] and filelog[2] have P4_Integration objects
    print "Sizeof filelog[0]->revisions[0]->integrations: " . sizeof($filelog[0]->revisions[0]->integrations) . "\n";
    print "Sizeof filelog[1]->revisions[0]->integrations: " . sizeof($filelog[1]->revisions[0]->integrations) . "\n";
    print "Sizeof filelog[2]->revisions[0]->integrations: " . sizeof($filelog[2]->revisions[0]->integrations) . "\n";
    var_dump(get_class($filelog[1]->revisions[0]->integrations[0]));
    var_dump(get_class($filelog[2]->revisions[0]->integrations[0]));
    // verify P4_Integration fields
    var_dump(isset($filelog[1]->revisions[0]->integrations[0]->how));
    var_dump(isset($filelog[1]->revisions[0]->integrations[0]->file));
    var_dump(isset($filelog[1]->revisions[0]->integrations[0]->erev));
    var_dump(isset($filelog[1]->revisions[0]->integrations[0]->srev));
    var_dump(isset($filelog[2]->revisions[0]->integrations[0]->how));
    var_dump(isset($filelog[2]->revisions[0]->integrations[0]->file));
    var_dump(isset($filelog[2]->revisions[0]->integrations[0]->erev));
    var_dump(isset($filelog[2]->revisions[0]->integrations[0]->srev));
} catch (Exception $e) {
	print $e->getMessage();
}

$p4->disconnect();
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
Sizeof filelog: 3
string(12) "P4_DepotFile"
string(12) "P4_DepotFile"
string(12) "P4_DepotFile"
string(16) "//depot/file.txt"
string(21) "//depot/tmp/file2.txt"
string(22) "//depot/tmp2/file2.txt"
Sizeof filelog[0]->revisions: 2
Sizeof filelog[1]->revisions: 1
Sizeof filelog[2]->revisions: 1
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Sizeof filelog[0]->revisions[0]->integrations: 0
Sizeof filelog[1]->revisions[0]->integrations: 1
Sizeof filelog[2]->revisions[0]->integrations: 1
string(14) "P4_Integration"
string(14) "P4_Integration"
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
