--TEST--
P4::__call -- Test magic methods (fetch_, call_, etc)
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

// test creating and then deleting a client
$client_template = CLIENT_ONE_NAME;
$client = $p4->fetch_client('-t', $client_template);
$client['Root'] = '/var/tmp';
$client['Options'] = str_replace($client['Options'], 'normdir', 'rmdir');

$results = $p4->save_client($client);
print $results[0] . "\n";

$results = $p4->run_clients();
// verify that there are two clients now
print "Sizeof run_clients: " . sizeof($results) . "\n";
// verify that each client has the following fields: client, Owner, Options, Root
foreach ($results as $result) {
    var_dump(isset($result['client']));
    var_dump(isset($result['Owner']));
    var_dump(isset($result['Options']));
    var_dump(isset($result['Root']));
}
$results = $p4->run_info();
// verify the size of the results (1)
print "Sizeof run_info: " . sizeof($results) . "\n";
// verify the following fields: userName, clientName, clientRoot, clientHost
var_dump(isset($results[0]['userName']));
var_dump(isset($results[0]['clientName']));
var_dump(isset($results[0]['clientRoot']));
var_dump(isset($results[0]['clientHost']));

// delete the client
$results = $p4->delete_client('test-client-2');
print $results[0] . "\n";

$results = $p4->format_client($client);
var_dump($results);

// recreate the client
var_dump(isset($client['Client']));
var_dump(isset($client['Owner']));
var_dump(isset($client['Options']));
var_dump(isset($client['Root']));
$clientValue = $client['Client'];
$ownerValue  = $client['Owner'];
$optionsValue = $client['Options'];
$rootValue   = $client['Root'];

// save the client
$results = $p4->save_client($client);
print $results[0] . "\n";

// verify that nothing has changed
var_dump(isset($client['Client']));
var_dump(isset($client['Owner']));
var_dump(isset($client['Options']));
var_dump(isset($client['Root']));
var_dump($client['Client'] == $clientValue);
var_dump($client['Owner'] == $ownerValue);
var_dump($client['Options'] == $optionsValue);
var_dump($client['Root'] == $rootValue);

$spec = $p4->format_client($client);
$results = $p4->parse_client($spec);

var_dump(isset($results['Client']));
var_dump(isset($results['Owner']));
var_dump(isset($results['Options']));
var_dump(isset($results['Root']));
var_dump($results['Client'] == $clientValue);
var_dump($results['Owner'] == $ownerValue);
var_dump($results['Options'] == $optionsValue);
var_dump($results['Root'] == $rootValue);
?>
--CLEAN--
<?php
require_once('teardown.inc');
?>
--EXPECTF--
Client test-client-2 saved.
Sizeof run_clients: 2
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
Sizeof run_info: 1
bool(true)
bool(true)
bool(true)
bool(true)
Client test-client-2 deleted.
string(%d) "Client:	test-client-2

Update:	%d/%d/%d %d:%d:%d

Access:	%d/%d/%d %d:%d:%d

Owner:	tester

Host:	%s

Description:
	Created by tester.

Root:	/var/tmp

Options:	rmdir

SubmitOptions:	submitunchanged

LineEnd:	local

Type:	writeable

Backup:	enable

View:
	//depot/... //test-client-2/...
"
bool(true)
bool(true)
bool(true)
bool(true)
Client test-client-2 saved.
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
bool(true)
bool(true)
bool(true)
bool(true)
