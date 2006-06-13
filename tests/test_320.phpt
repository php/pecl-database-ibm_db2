--TEST--
IBM-DB2: db2_escape_string
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

$str[0] = "That's cool. Backslash (\). Double quote (\")";
$str[1] = "Nothing to quote!";
$str[2] = 200676;
$str[3] = "";
$str[4] = "The NULL character \0 must be quoted aswell!";

foreach( $str as $string ) {
        echo "\n";
        echo "Original: ".$string;
        echo "\n";
        echo "addslashes: ".addslashes($string);
        echo "\n";
        echo "db2_escape_string: ".db2_escape_string($string);
        echo "\n";
}

?>
--EXPECTF--
Original: That's cool. Backslash (\). Double quote (")
addslashes: That\'s cool. Backslash (\\). Double quote (\")
db2_escape_string: That''s cool. Backslash (\\). Double quote (\")

Original: Nothing to quote!
addslashes: Nothing to quote!
db2_escape_string: Nothing to quote!

Original: 200676
addslashes: 200676
db2_escape_string: 200676

Original:
addslashes:
db2_escape_string:

Original: The NULL character %c must be quoted aswell!
addslashes: The NULL character \0 must be quoted aswell!
db2_escape_string: The NULL character \0 must be quoted aswell!
