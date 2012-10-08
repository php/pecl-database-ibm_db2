--TEST--
IBM-DB2: Call a stored procedure with IN and OUT parameters multiple times
--SKIPIF--
<?php require_once('skipif.inc'); ?>
--FILE--
<?php
    require_once('connection.inc');
    $prepconn = db2_connect($database, $user, $password);

    @db2_exec( $prepconn , "DROP PROCEDURE kr_testproc" );
    db2_exec( $prepconn , "CREATE PROCEDURE kr_testproc ( IN p1 BIGINT, IN p2 DECIMAL(12,2), IN p3 DECIMAL(12,2), OUT p4 BIGINT, OUT p5 DATE)
                           DYNAMIC RESULT SETS 1
                           LANGUAGE SQL
                           BEGIN
                             SET p4 = p1 + p2;
                             SET p5 = '24.01.1982';
                           END" );

    db2_commit( $prepconn );

    $purchase_insertorder_query = "CALL kr_testproc (?, ?, ?, ?, ?)";
    $prep = db2_prepare($prepconn, $purchase_insertorder_query);
    for( $i=0 ; $i<3; $i++ ){
        $p1 = $i;
        $p2 = 12.001;
        $p3 = 12.00;
        $p4 = -1;
        $p5 = "2002-10-20";

        db2_bind_param($prep, 1, 'p1', DB2_PARAM_IN, DB2_LONG);
        db2_bind_param($prep, 2, 'p2', DB2_PARAM_IN, DB2_DOUBLE, 12, 2);
        db2_bind_param($prep, 3, 'p3', DB2_PARAM_IN, DB2_DOUBLE, 12, 2);
        db2_bind_param($prep, 4, 'p4', DB2_PARAM_OUT, DB2_LONG);
        db2_bind_param($prep, 5, 'p5', DB2_PARAM_OUT, DB2_CHAR);

        print "date before $p5\n";
        $result = db2_execute($prep);
        print "date after $p5\n";
        print db2_stmt_errormsg( $prep);
        db2_commit( $prepconn );
    }
    db2_close( $prepconn );
?>
--EXPECT--
date before 2002-10-20
date after 1982-01-24
date before 2002-10-20
date after 1982-01-24
date before 2002-10-20
date after 1982-01-24
