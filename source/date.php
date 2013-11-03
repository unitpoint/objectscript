<?php

$v = iconv('UTF-8', 'windows-1251', "Привет!");
echo iconv('windows-1251', 'UTF-8', $v)." -- $v\n";

$v = "Привет!";
echo json_encode($v)."\n";

echo date("D, d-M-Y H:i:s T")."\n";