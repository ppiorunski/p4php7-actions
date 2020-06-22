<?php

define('DEPOT_ROOT',      getcwd() . DIRECTORY_SEPARATOR . 'testroot');
define('CLIENT_ONE_ROOT', getcwd() . DIRECTORY_SEPARATOR . 'client-one');
define('CLIENT_TWO_ROOT', getcwd() . DIRECTORY_SEPARATOR . 'client-two');

define('CLIENT_ONE_NAME', 'test-client-1');
define('CLIENT_TWO_NAME', 'test-client-2');

function whichP4d()
{
    // locate p4d binary
    $path = getenv('PATH');

    if ($path) {
        $dirs = explode(PATH_SEPARATOR, $path);
		$dirs[] = getcwd();
    } else {
	$dirs = array(
            getcwd(),
            '/bin',
	    '/usr/bin',
	    '/usr/local/bin',
	    'c:/Program Files/Perforce',
	    'c:/Program Files (x86)/Perforce'
	);
    }

    foreach ($dirs as $dir) {
        $p4d = $dir . '/p4d' . (isWindows() ? '.exe' : null);
	if (is_executable($p4d)) 
            return $p4d;
    }

    return false;
}

function requireP4d()
{
    if (whichP4d() === false) {
        die('Skip missing p4d; not in current directory');
    }
}

function requireMinP4d($year, $release)
{
    requireP4d();

    if (preg_match('#Rev\..*/(\d\d\d\d)\.(\d)[/.]#m', shell_exec(whichP4d() . ' -V'), $matches) !== 1
        || (int)$matches[1] < (int)$year
        || ((int)$matches[1] == (int)$year && (int)$matches[2] < (int)$release)
    ) {
        die("Skip p4d version too low; requires >= $year.$release");
    }
}

function isWindows()
{
    return (bool) preg_match('/^WIN/i', PHP_OS);
}
?>
