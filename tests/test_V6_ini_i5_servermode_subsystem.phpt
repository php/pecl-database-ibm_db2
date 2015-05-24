--TEST--
IBM-DB2: 1.9.7 - IBM i change QSQSRVR subsystem ccsid 1208 (operator issue)
--SKIPIF--
<?php 
require_once('skipif.inc'); 
?>
--FILE--
<?php
require_once('connection.inc');
require_once('xmlservice.inc');
/*
> crtlib adc                                     
> CRTSBSD SBSD(ADC/ADC) POOLS((1 *BASE)) TEXT('tony subsystem descr')
> CRTJOBD JOBD(ADC/ADC) TEXT('tony job descr')   
> CRTCLS CLS(ADC/ADC) TEXT('tony class')              
> ADDPJE SBSD(ADC/ADC) PGM(QSYS/QSQSRVR) MAXJOBS(*NOMAX)
> strsbs adc/adc
*/

putenv("IBM_DB_I5_TEST=1");
putenv("IBM_DB_i5_override_ccsid=1208");
putenv("IBM_DB_i5_servermode_subsystem=ADC"); /* before include this test */
$conn1 = db2_connect($database, $user, $password);
$xml = xmlservice_diag($conn1);
$key1 = xmlservice_diag_jobinfo($xml);
echo "key1 $key1\n";
db2_close($conn1);

if (strpos($key1,"ADC") === false) {
  echo "failed key1 ($key1) missing ADC\n";
} else {
  echo "success\n";
}

?>
--EXPECTF--
%s
success


