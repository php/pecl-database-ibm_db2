--TEST--
IBM-DB2: db2_fetch_row() (index by position) - 1
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php

require_once('connection.inc');

$conn = db2_connect($db,$user,$password);

$result = db2_exec($conn, "SELECT * FROM staff WHERE id < 101");

while ($row = db2_fetch_row($result)) 
{
   $result2 = db2_prepare($conn, "SELECT * FROM staff WHERE id < 101", array ('cursor' => DB2_SCROLLABLE));
   db2_execute($result2);
   while ($row2 = db2_fetch_row($result2)) 
   {
      echo db2_result($result2, 0) . " : ";
      echo db2_result($result2, 1) . " : ";
      echo db2_result($result2, 2) . " : ";
      echo db2_result($result2, 3) . " : ";
      echo db2_result($result2, 5) . "\n";
   }
}

?>
--EXPECT--
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80
10 : Sanders : 20 : Mgr   : 18357.50
20 : Pernal : 20 : Sales : 18171.25
30 : Marenghi : 38 : Mgr   : 17506.75
40 : O'Brien : 38 : Sales : 18006.00
50 : Hanes : 15 : Mgr   : 20659.80
60 : Quigley : 38 : Sales : 16808.30
70 : Rothman : 15 : Sales : 16502.83
80 : James : 20 : Clerk : 13504.60
90 : Koonitz : 42 : Sales : 18001.75
100 : Plotz : 42 : Mgr   : 18352.80

