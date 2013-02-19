<?php
$startMemory = memory_get_usage();
$array = range(1, 1024*1024);
echo memory_get_usage() - $startMemory, ' bytes';