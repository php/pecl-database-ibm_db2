--TEST--
IBM-DB2: db2_fetch_row\(\) (index by position) - 3 - nested
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_prepare($conn, "SELECT * FROM animals", array('cursor' => DB2_SCROLLABLE) );
db2_execute($result);
while ($row = db2_fetch_row($result)) {
	$result2 = db2_prepare($conn, "SELECT * FROM animals", array ('cursor' => DB2_SCROLLABLE));
	db2_execute($result2);
	while ($row2 = db2_fetch_row($result2)) {
		echo db2_result($result2, 0) . " : ";
		echo db2_result($result2, 1) . " : ";
		echo db2_result($result2, 2) . " : ";
		echo db2_result($result2, 3) . "\n";
	}
}

?>
--EXPECT--
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00
0 : cat : Pook             : 3.20
1 : dog : Peaches          : 12.30
2 : horse : Smarty           : 350.00
3 : gold fish : Bubbles          : 0.10
4 : budgerigar : Gizmo            : 0.20
5 : goat : Rickety Ride     : 9.70
6 : llama : Sweater          : 150.00

