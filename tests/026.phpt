--TEST--
 P4API SSO Handler
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

 echo "Enable SSO\n";
 print_r($p4->fetch_triggers());
 print_r($p4->save_triggers(array( "Triggers" => array("loginsso auth-check-sso auth pass"))));
 print_r($p4->fetch_triggers());
 $p4->run_admin("restart");
 $p4->disconnect();

 echo "Test the default behavior\n";
 $p4->connect();
 try {
   echo var_export($p4->loginsso, true) . "\n";
   print_r($p4->run_login("Passw0rd"));
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 } catch (P4_Exception $e) {
   print_r($e->getMessage());
   print_r($p4->errors);
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 }

 echo "Test the explicitly disabled behavior\n";
 try {
   echo var_export($p4->loginsso, true) . "\n";
   $p4->loginsso = false;
   echo var_export($p4->loginsso, true) . "\n";
   print_r($p4->run_login("Passw0rd"));
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 } catch (P4_Exception $e) {
   print_r($e->getMessage());
   print_r($p4->errors);
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 }

 echo "Test the enabled behavior\n";
 try {
   echo var_export($p4->loginsso, true) . "\n";
   $p4->loginsso = true;
   echo var_export($p4->loginsso, true) . "\n";
   print_r($p4->run_login("Passw0rd"));
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 } catch (P4_Exception $e) {
   print_r($e->getMessage());
   print_r($p4->errors);
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 }

 echo "Test the enabled behavior (alt login call)\n";
 try {
   print_r($p4->run("login"));
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 } catch (P4_Exception $e) {
   print_r($e->getMessage());
   print_r($p4->errors);
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
 }

 echo "Test the fail result\n";
 try {
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
   $p4->ssofailresult = "My bad result!";
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
   print_r($p4->run("login"));
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
 } catch (P4_Exception $e) {
   print_r($e->getMessage());
   print_r($p4->errors);
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
 }

 echo "Test the pass result\n";
 try {
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
   $p4->ssopassresult = "My good result!";
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
   print_r($p4->run("login"));
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
 } catch (P4_Exception $e) {
   print_r($e->getMessage());
   print_r($p4->errors);
   print_r(is_array($p4->ssovars) ? array_keys($p4->ssovars) : $p4->ssovars);
   echo var_export($p4->ssofailresult, true) . "\n";
   echo var_export($p4->ssopassresult, true) . "\n";
 }

 ?>
--CLEAN--
 <?php
 require_once('teardown.inc');
 ?>
--EXPECTF--
Enable SSO
Array
(
)
Array
(
    [0] => Triggers saved.
)
Array
(
    [Triggers] => Array
        (
            [0] => loginsso auth-check-sso auth pass
        )

)
Test the default behavior
NULL
[P4.run()] Errors during command execution( "p4 login" )

	[Error]: Single sign-on on client failed: 'P4LOGINSSO' not set.

Array
(
    [0] => Single sign-on on client failed: 'P4LOGINSSO' not set.
)
Array
(
)
Test the explicitly disabled behavior
NULL
false
[P4.run()] Errors during command execution( "p4 login" )

	[Error]: Single sign-on on client failed: 'P4LOGINSSO' not set.

Array
(
    [0] => Single sign-on on client failed: 'P4LOGINSSO' not set.
)
Array
(
)
Test the enabled behavior
false
true
Array
(
)
Array
(
    [0] => user
    [1] => serverAddress
    [2] => P4PORT
    [3] => ssoArgs
    [4] => data
)
Test the enabled behavior (alt login call)
Array
(
)
Array
(
    [0] => user
    [1] => serverAddress
    [2] => P4PORT
    [3] => ssoArgs
    [4] => data
)
Test the fail result
NULL
NULL
'My bad result!'
NULL
[P4.run()] Errors during command execution( "p4 login" )

	[Error]: Single sign-on on client failed: My bad result!

Array
(
    [0] => Single sign-on on client failed: My bad result!
)
Array
(
)
NULL
NULL
Test the pass result
NULL
NULL
NULL
'My good result!'
Array
(
    [0] => Array
        (
            [User] => tester
            [TicketExpiration] => 43200
        )

)
Array
(
)
NULL
NULL
