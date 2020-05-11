<?php
function xmlservice($conn, $xml, $ctlIn='*here', $ipcIn="*na", $lib='qxmlserv') {
  $ctl = $ctlIn;
  $ipc = $ipcIn;
  $xmlIn = $xml;
  $xmlOut = '';
  $stmt = db2_prepare($conn, "call $lib.iPLUG512K(?,?,?,?)"); // Call XMLSERVICE 
  $ret=db2_bind_param($stmt, 1, "ipc", DB2_PARAM_IN);     // ? - /tmp/raw_$user (*sbmjob)
  $ret=db2_bind_param($stmt, 2, "ctl", DB2_PARAM_IN);     // ? - *here or *sbmjob
  $ret=db2_bind_param($stmt, 3, "xmlIn", DB2_PARAM_IN);   // ? - XML input script
  $ret=db2_bind_param($stmt, 4, "xmlOut", DB2_PARAM_OUT); // ? - XML output return
  $ret=db2_execute($stmt);
  return $xmlOut;
}

/* C6F3C3F1C4F0 00000005*/
function xmlservice_QSYGETPH($conn, $uid="*CURRENT", $pwd="*NOPWD", $ctlIn='*here', $ipcIn="*na", $lib='qxmlserv') {
$xml = "<?xml version='1.0'?>
<xmlservice>
<pgm name='QSYGETPH' lib='QSYS' error='fast'>
<parm io='in'><data type='10A'>".$uid."</data></parm>
<parm io='in'><data type='10A'>".$pwd."</data></parm>
<parm io='out'><data type='12b'/></parm>
<parm io='in'><data type='10i0'>0</data></parm>
";
if (substr($pwd, 0, 1) != '*') {
  $xml .= "<parm io='in'><data type='10i0'>".strlen($pwd)."</data></parm>\n";
  $xml .= "<parm io='in'><data type='10i0'>-1</data></parm>\n";
}
$xml .= "</pgm>
</xmlservice>
";
return xmlservice($conn, $xml, $ctlIn, $ipcIn, $lib);
}

function xmlservice_QWTSETP($conn, $profile_handle, $ctlIn='*here', $ipcIn="*na", $lib='qxmlserv') {
$xml = "<?xml version='1.0'?>
<xmlservice>
<pgm name='QWTSETP' lib='QSYS' error='fast'>
<parm io='in'><data type='12b'>".$profile_handle."</data></parm>
<parm io='in'><data type='10i0'>0</data></parm>
</pgm>
</xmlservice>
";
return xmlservice($conn, $xml, $ctlIn, $ipcIn, $lib);
}

function xmlservice_QSYRLSPH($conn, $profile_handle, $ctlIn='*here', $ipcIn="*na", $lib='qxmlserv') {
$xml = "<?xml version='1.0'?>
<xmlservice>
<pgm name='QSYRLSPH' lib='QSYS' error='fast'>
<parm io='in'><data type='12b'>".$profile_handle."</data></parm>
</pgm>
</xmlservice>
";
return xmlservice($conn, $xml, $ctlIn, $ipcIn, $lib);
}
function xmlservice_switch_profile($conn, $uid="*CURRENT", $pwd="*NOPWD", $ctlIn='*here', $ipcIn="*na", $lib='qxmlserv') {
  $xml = xmlservice_QSYGETPH($conn, $uid, $pwd, $ctlIn, $ipcIn, $lib);
  // var_dump($xml);
  $xmlobj = simplexml_load_string($xml);
  $profile_obj = $xmlobj->xpath('/xmlservice/pgm/parm/data');
  // var_dump($profile_obj);
  $profile_handle = (string)$profile_obj[0];
  // var_dump($profile_handle);
  $xml = xmlservice_QWTSETP($conn, $profile_handle, $ctlIn, $ipcIn, $lib);
  $xml = xmlservice_QSYRLSPH($conn, $profile_handle, $ctlIn, $ipcIn, $lib);
}

function xmlservice_diag_jobinfo($xml) {
  // var_dump($xml);
  $xmlobj = simplexml_load_string($xml);
  $jobname = $xmlobj->xpath('/xmlservice/diag/jobinfo/jobname');
  $jobuser = $xmlobj->xpath('/xmlservice/diag/jobinfo/jobuser');
  $jobnbr = $xmlobj->xpath('/xmlservice/diag/jobinfo/jobnbr');
  $curuser = $xmlobj->xpath('/xmlservice/diag/jobinfo/curuser');
  $sbsname = $xmlobj->xpath('/xmlservice/diag/jobinfo/sbsname');
  return "{$sbsname[0]}-{$jobname[0]}-{$jobuser[0]}-{$jobnbr[0]}-{$curuser[0]}";
}

function xmlservice_diag($conn, $ctlIn='*here', $ipcIn="*na", $lib='qxmlserv') {
  return xmlservice($conn, "<?xml version='1.0'?><xmlservice><diag/></xmlservice>", $ctlIn, $ipcIn, $lib);
}

function xmlservice_kill($conn, $ctlIn='*here *immed', $ipcIn="*na", $lib='qxmlserv') {
  if ($ipcIn == "*na") {
    /* ENDJOBABN JOB(*) ??? ENDJOB JOB(*) OPTION(*IMMED)??? */
    return xmlservice($conn, "<?xml version='1.0'?><xmlservice><cmd>ENDJOB JOB(*) OPTION(*IMMED)</cmd></xmlservice>", $ctlIn, $ipcIn, $lib);
  }
  return xmlservice($conn, "<?xml version='1.0'?>", $ctlIn, $ipcIn, $lib);
}

?>
