<?php
$result = @db2_exec($conn,"drop table t_string");
$result = db2_exec($conn,"create table t_string(a integer, b double, c varchar(100))");
?>
