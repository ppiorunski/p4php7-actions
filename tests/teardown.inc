<?php

	require_once 'helper.inc';

    function removeDirectory($directory, $recursive = true)
    {
        if (is_dir($directory)) {
            $files = new RecursiveDirectoryIterator($directory);
            foreach ($files as $file) {
                if ($files->isDot()) {
                    continue;
                }
                if ($file->isFile()) {
                    chmod($file->getPathname(), 0700);
                    if (!unlink($file->getPathname())) {
                        throw new Exception(
                            "Can't delete '" . $file->getPathname() . "'"
                        );
                    }
                } elseif ($file->isDir() && $recursive) {
                    chmod($file->getPathname(), 0700);
					removeDirectory($file->getPathname());
                }
            }
            rmdir($directory);
        }
    }

    if (isWindows())
    {
        sleep(1);
    }
    removeDirectory(DEPOT_ROOT);
    removeDirectory(CLIENT_ONE_ROOT);
    removeDirectory(CLIENT_TWO_ROOT);

?>
