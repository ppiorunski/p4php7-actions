--TEST--
 P4 Changes - closing a job on a change submitted 
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
// Get job spec
$job                = $p4->fetch_job('job000001');
$job['Description'] = 'This is a test job';
$job['Status']      = 'open';
$results            = $p4->save_job($job);
var_dump($results); // Should output "Job job000001 saved."

// Check the job has been set to open.
$jobCheck = $p4->fetch_job('job000001');
var_dump($jobCheck['Status']); // Should ouput "open"


// Get client and create file
$client = $p4->fetch_client();
$root   = $client["Root"];
$file   = $root . DIRECTORY_SEPARATOR . "testsubmit.txt";
$p4->run("add", $file);
$fp = fopen($file, 'w');
fwrite($fp, "Testing run_submit " . date("F d r") . "\n");
fclose($fp);

// Giving full path Depot path
$files[] = "//depot/testsubmit.txt";

$jobentry = 'job000001 closed';
$arr = array ( & $jobentry );

// Get change spec and set value.
$change = $p4->fetch_change();
$change['Description'] = 'This is a test change.'. date("F d r");
$change['Files']       = $files;
$change['Jobs']        = & $arr;
$p4->input = $change;
$resultssubmit = $p4->run_change('-si');

var_dump($resultssubmit);

// Check the job has changed to closed.
$jobCheck = $p4->fetch_job('job000001');
var_dump($jobCheck['Status']); // Should ouput "closed"

$p4->disconnect();
?>
--CLEAN--
 <?php
 require_once('teardown.inc');
 ?>
--EXPECTF--
array(1) {
  [0]=>
  string(20) "Job job000001 saved."
}
string(4) "open"
array(1) {
  [0]=>
  string(53) "Change 1 created with 1 open file(s) fixing 1 job(s)."
}
string(4) "open"
