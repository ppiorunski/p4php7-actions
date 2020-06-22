<?php

require_once 'helper.inc';

if (!file_exists(DEPOT_ROOT))
    mkdir(DEPOT_ROOT);

if (!file_exists(CLIENT_ONE_ROOT))
    mkdir(CLIENT_ONE_ROOT);

if (!file_exists(CLIENT_TWO_ROOT))
    mkdir(CLIENT_TWO_ROOT);

$p4d = whichP4d();

// die early (and loudly) if no p4d can be found.
if ($p4d === false) {
	die('Cannot locate p4d binary. Aborting!');
}

// create instance and connect.
$p4 = new P4();
$p4->port = "rsh:$p4d -r " . DEPOT_ROOT . " -i";
$p4->user = "tester";
$p4->tagged = true;
$p4->connect();

// create user.
$user = $p4->fetch_user();
$user['Name'] = 'tester';
$p4->save_user($user);
$p4->run_password("testing123", "testing123");
$p4->run_login("testing123");

// create clients
$p4->client = CLIENT_ONE_NAME;
$clientOne = $p4->fetch_client();
$clientOne['Root'] = CLIENT_ONE_ROOT;
$p4->save_client($clientOne);

$p4->client = CLIENT_TWO_NAME;
$clientTwo = $p4->fetch_client();
$clientTwo['Root'] = CLIENT_TWO_ROOT;
$p4->save_client($clientTwo);


