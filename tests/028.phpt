--TEST--
 Parallel sync support
--ARGS--
 -c tests/php.ini
--SKIPIF--
 <?php
 include_once 'helper.inc';
 requireP4d();
 if( !str_contains( P4::identify(), "Parallel sync threading built-in" ) ) {
   die( "P4API doesn't have built-in parallel sync support!" );
 }
 ?>
--FILE--
 <?php
 include 'connect.inc';

 date_default_timezone_set("America/Vancouver");

$p4->client = CLIENT_ONE_NAME;
$client = $p4->fetch_client();
$root = $client["Root"];


$file = $root . DIRECTORY_SEPARATOR . "testsubmit1.txt";
$fp1 = fopen($file, 'w');
fwrite($fp1, str_repeat("Testing run_submit 1 arg" . date("F d r") . "\n", 2048));
fclose($fp1);

for ($x = 2; $x <= 20; $x++) {
  copy($file, str_replace("testsubmit1", "testsubmit" . $x, $file));
}
$p4->run_add($root . "/...");
$change = $p4->fetch_change();
$change['Description'] = "Testing run_submit 1 arg";
$results = $p4->run_submit($change);
var_dump($results);

echo "Un-sync\n";
var_dump( count( $p4->run_sync("//...#0") ) );

echo "Put 1 file back\n";
$file = $root . DIRECTORY_SEPARATOR . "testsubmit1.txt";
$fp1 = fopen($file, 'w');
fwrite($fp1, "break the sync!");
fclose($fp1);

echo "Enable parallel sync\n";
var_dump($p4->run_configure("set", "net.parallel.max=4"));
var_dump($p4->run_configure("set", "net.parallel.threads=4"));

echo "Re-sync\n";
try {
    var_dump( $p4->run_sync("//...") );
} catch (Exception $e) {
    if(preg_match_all("/(.*clobber writable file ).*(client-one.testsubmit1.txt)/s", $e->getMessage(), $matches)) {
        echo 'Caught exception: ', $matches[1][0] . $matches[2][0], "\n";
    } else {
        echo 'Caught exception: ', $e->getMessage(), "\n";
    }
}

 ?>
--CLEAN--
 <?php
 require_once('teardown.inc');
 ?>
--EXPECTF--
array(22) {
  [0]=>
  array(3) {
    ["change"]=>
    string(1) "1"
    ["openFiles"]=>
    string(2) "20"
    ["locked"]=>
    string(2) "20"
  }
  [1]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit1.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [2]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit10.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [3]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit11.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [4]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit12.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [5]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit13.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [6]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit14.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [7]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit15.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [8]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit16.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [9]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit17.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [10]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit18.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [11]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit19.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [12]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit2.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [13]=>
  array(3) {
    ["depotFile"]=>
    string(24) "//depot/testsubmit20.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [14]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit3.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [15]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit4.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [16]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit5.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [17]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit6.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [18]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit7.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [19]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit8.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [20]=>
  array(3) {
    ["depotFile"]=>
    string(23) "//depot/testsubmit9.txt"
    ["rev"]=>
    string(1) "1"
    ["action"]=>
    string(3) "add"
  }
  [21]=>
  array(1) {
    ["submittedChange"]=>
    string(1) "1"
  }
}
Un-sync
int(20)
Put 1 file back
Enable parallel sync
array(1) {
  [0]=>
  array(4) {
    ["Action"]=>
    string(3) "set"
    ["ServerName"]=>
    string(3) "any"
    ["Name"]=>
    string(16) "net.parallel.max"
    ["Value"]=>
    string(1) "4"
  }
}
array(1) {
  [0]=>
  array(4) {
    ["Action"]=>
    string(3) "set"
    ["ServerName"]=>
    string(3) "any"
    ["Name"]=>
    string(20) "net.parallel.threads"
    ["Value"]=>
    string(1) "4"
  }
}
Re-sync
Caught exception: [P4.run()] Errors during command execution( "p4 sync //..." )

	[Error]: Can't clobber writable file client-one%etestsubmit1.txt
