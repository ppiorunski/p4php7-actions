--TEST--
P4::__get and P4::__set - Tests getting and setting of fields in the P4 class.
--ARGS--
-c tests/php.ini
--FILE--
<?php

// Create P4 instance 
$p4 = new P4();

// INPUT TESTS

// Test uninitialized (should be NULL)
var_dump($p4->input);

// Test a scalar (string)
$p4->input = "Hello World!";
var_dump($p4->input);

// Test a scalar (int - will be converted to string)
$p4->input = 5;
var_dump($p4->input);

// Test a scalar (bool - will be converted to string)
$p4->input = true;
var_dump($p4->input);

// Test NULL (will be converted to empty string)
$p4->input = NULL;
var_dump($p4->input);

// Test a list / vector with string values. 
$p4->input = array( "One", "Two", "Three" );
var_dump($p4->input);

// Test a list / vector with vector values. 
$p4->input = array( array( "One", "Two" ), array( "Three", "Four" ) );
var_dump($p4->input);

// Test a hash
$p4->input = array( "KeyOne" => "ValueOne", "KeyTwo" => "ValueTwo" );
var_dump($p4->input);

// Test an object
$p4->input = new stdClass();
var_dump($p4->input);

// GENERAL (NON-INPUT) TESTS

// Test accessing uninitialized variable
var_dump($p4->something);

// Test variable created at runtime
$p4->something = "Hello";
var_dump($p4->something);
if (isset($p4->something)) {
    print("Something is set\n");
}

// Test declared class fields
$p4->api_level = 1;
var_dump($p4->api_level);

// try setting an invalid / unsupported charset
try {
    $p4->charset = "abcdef";
} catch (P4_Exception $e) {
    print "Unsupported charset\n";
}

// try setting valid charset
$p4->charset = "utf8";
var_dump($p4->charset);

// current working dir should accept any string
$p4->cwd = "anystring for cwd";
var_dump($p4->cwd);

// should accept ints but ignore strings
$fields = array("exception_level", "maxlocktime", "maxresults", "maxscanrows");
foreach ($fields as  $field) {
	$p4->$field = 2;
	var_dump($p4->$field);
	$p4->$field = "foo";
	var_dump($p4->$field);
}

// Should evaluate to "true"
$p4->tagged = true;
var_dump($p4->tagged);
$p4->tagged = "fdsa";
var_dump($p4->tagged);
// false
$p4->tagged = false;
var_dump($p4->tagged);
$p4->tagged = 0;
var_dump($p4->tagged);

// starts true
var_dump($p4->streams);
// set to false in a couple manners
$p4->streams = 0;
var_dump($p4->streams);
$p4->streams = false;
var_dump($p4->streams);
// set back to true
$p4->streams = "fdsa";
var_dump($p4->streams);
$p4->streams = true;
var_dump($p4->streams);

// test read-only fields
var_dump($p4->errors);
var_dump($p4->warnings);
var_dump($p4->server_level);

// test various other fields
$p4->client      = "myclient";
$p4->host        = "myhost";
$p4->password    = "password";
$p4->port        = "myserver:1666";
$p4->prog        = "myprogram";
$p4->ticket_file = "myticket";
$p4->user        = "user";

var_dump($p4->client);
var_dump($p4->host);
var_dump($p4->password);
var_dump($p4->port);
var_dump($p4->prog);
var_dump($p4->ticket_file);
var_dump($p4->user);

?>
--EXPECTF--
NULL
string(12) "Hello World!"
string(1) "5"
string(1) "1"
string(0) ""
array(3) {
  [0]=>
  string(3) "One"
  [1]=>
  string(3) "Two"
  [2]=>
  string(5) "Three"
}
array(2) {
  [0]=>
  array(2) {
    [0]=>
    string(3) "One"
    [1]=>
    string(3) "Two"
  }
  [1]=>
  array(2) {
    [0]=>
    string(5) "Three"
    [1]=>
    string(4) "Four"
  }
}
array(2) {
  ["KeyOne"]=>
  string(8) "ValueOne"
  ["KeyTwo"]=>
  string(8) "ValueTwo"
}
object(stdClass)#2 (0) {
}

%s: Undefined property:%w P4::$%s in %s001.php on line %d
NULL
string(5) "Hello"
int(1)
Unsupported charset
string(4) "utf8"
string(17) "anystring for cwd"
int(2)
int(2)
int(2)
int(2)
int(2)
int(2)
int(2)
int(2)
bool(true)
bool(true)
bool(false)
bool(false)
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
array(0) {
}
array(0) {
}
int(0)
string(8) "myclient"
string(6) "myhost"
string(8) "password"
string(13) "myserver:1666"
string(9) "myprogram"
string(8) "myticket"
string(4) "user"
