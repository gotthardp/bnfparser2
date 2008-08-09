<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html>
<head>
<title>The BNF Verification Service</title>
<link rel="stylesheet" type="text/css" href="bnfweb.css">
</head>
<body>
<h1>The BNF Verification Service</h1>
This is a universal syntax verification utility for syntax specifications
written in <a href="http://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form" target="_blank">Backus-Naur Form (BNF)</a>.
Select syntax specification(s) and symbol that should be used for verification,
enter text to verify and click the "Get Results" button. The application checks
if the text conforms to the syntax specification(s) given.<br/>

<br/>
<form method="post" action="bnfweb.cgi" enctype="multipart/form-data">

<table class="query" border="0">
<tr>
<td rowspan="3" width="50%" valign="top">

<fieldset><legend><a href="#syntax">Syntax</a> specification to use</legend>
<table border="0" cellspacing="0" cellpadding="2">

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value='<domain>'">
<td><input type="checkbox" name="syntax" value="rfc1035-2.3.abnf"/></td>
<td nowrap>
<b>Domain Names - Implementation And Specification</b><br/>
Preferred name syntax<br/>
<a href="share/rfc1035-2.3.abnf" target="_blank">rfc1035-2.3.abnf</a><br/>
</td>
</tr>

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value='rulelist'">
<td><input type="checkbox" name="syntax" value="rfc2234-4.abnf"/></td>
<td nowrap>
<b>Augmented BNF for Syntax Specifications: ABNF</b><br/>
ABNF Definition of ABNF<br/>
<a href="share/rfc2234-4.abnf" target="_blank">rfc2234-4.abnf</a><br/>
</td>
</tr>

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value=''">
<td><input type="checkbox" name="syntax" value="rfc2234-6.1.abnf"/></td>
<td nowrap>
<b>Augmented BNF for Syntax Specifications: ABNF</b><br/>
Core Rules<br/>
<a href="share/rfc2234-6.1.abnf" target="_blank">rfc2234-6.1.abnf</a><br/>
</td>
</tr>

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value='telephone-url'">
<td><input type="checkbox" name="syntax" value="rfc2806-2.abnf"/></td>
<td nowrap>
<b>URLs for Telephone Calls</b><br/>
"tel" URL scheme<br/>
<a href="share/rfc2806-2.abnf" target="_blank">rfc2806-2.abnf</a><br/>
</td>
</tr>

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value='SIP-message'">
<td><input type="checkbox" name="syntax" value="rfc3261-25.abnf"/></td>
<td nowrap>
<b>SIP: Session Initiation Protocol</b><br/>
Augmented BNF for the SIP Protocol<br/>
<a href="share/rfc3261-25.abnf" target="_blank">rfc3261-25.abnf</a><br/>
</td>
</tr>

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value='rulelist'">
<td><input type="checkbox" name="syntax" value="rfc4234-4.abnf"/></td>
<td nowrap>
<b>Augmented BNF for Syntax Specifications: ABNF</b><br/>
ABNF Definition of ABNF<br/>
<a href="share/rfc4234-4.abnf" target="_blank">rfc4234-4.abnf</a><br/>
</td>
</tr>

<tr class="grammar" onDblClick="document.getElementsByName('symbol')[0].value=''">
<td><input type="checkbox" name="syntax" value="rfc4234-B.1.abnf"/></td>
<td nowrap>
<b>Augmented BNF for Syntax Specifications: ABNF</b><br/>
Core Rules<br/>
<a href="share/rfc4234-B.1.abnf" target="_blank">rfc4234-B.1.abnf</a><br/>
</td>
</tr>

<tr><td colspan="2"><a href="#syntax-file">and</a><br/>
<?php
$count =  min(max(1,$_GET["sf"]),100);
for($i = 0; $i < $count; $i++)
  echo '<input type="file" name="syntax-file', $i, '" style="width:100%"/><br/>';

if($count < 100)
  echo '<a href="bnfweb.php?sf=', $count+1, '&tf=', $_GET["tf"], '">(+)</a>';
else
  echo '(+)';
if($count > 1)
  echo '<a href="bnfweb.php?sf=', $count-1, '&tf=', $_GET["tf"], '">(&ndash;)</a>';
else
  echo '(&ndash;)';
?>
</td></tr>
</table>

</fieldset>Some specification is not listed here?
<a href="mailto:bnfparser2-users@lists.sourceforge.net">Tell us.</a>
</td>

<td width="50%" valign="top">
<fieldset><legend><a href="#symbol">Symbol</a> to use</legend>
<table border="0" width="100%">
<tr><td>
<input type="text" name="symbol" style="width:100%"/>
</td></tr>
</fieldset>
</table>
</td></tr>

<tr><td width="50%" valign="top">
<fieldset><legend><a href="#text">Text</a> to verify</legend>

<table border="0" width="100%">
<tr><td>
<textarea name="input-text" style="height:100px;width:100%"></textarea>
</td></tr>

<tr><td>
or<br/>
<?php
$count = min(max(1,$_GET["tf"]),100);
for($i = 0; $i < $count; $i++)
  echo '<input type="file" name="input-file', $i, '" style="width:100%"/><br/>';

if($count < 100)
  echo '<a href="bnfweb.php?sf=', $_GET["sf"], '&tf=', $count+1, '">(+)</a>';
else
  echo '(+)';
if($count > 1)
  echo '<a href="bnfweb.php?sf=', $_GET["sf"], '&tf=', $count-1, '">(&ndash;)</a>';
else
  echo '(&ndash;)';
?>
</td></tr>

</table>

</fieldset>

</td></tr>
<tr><td valign="top">
<input type="submit" name="submit" value="Get Results"/>
</td></tr>

</table>
</form>

<p class="credits">Privacy statement: Data you enter in this form will be
removed immediately after the completing the verification report.</p>

<hr>

<dl>
<dt><a name="syntax">Syntax specification to use</a></dt>
<dd>Select what specifications the text has to conform, and/or upload a
<a href="#syntax-file">custom syntax specification.</a>
(A file describing the BNF grammar.) The pre-defined specifications have been
copy-pasted from RFC documents and other standards.</dd>
<dt><a name="symbol">Symbol to use</a></dt>
<dd>Define what symbol will be checked. The symbol must be defined in one of
the selected syntax specifications.
If you double-click some syntax specification, this field will be completed
with a main symbol from the respective specification.</dd>
<dt><a name="text">Text to verify</a></dt>
<dd>Define what text should be checked for compliance to selected syntax
specifications.</dd>
</dl>

<p>
The <b><a name="syntax-file">custom syntax specification</a></b> may may contain
line comments with the following tags
(see <a href="http://bnfparser2.sourceforge.net" target="_blank">BNF Parser&sup2;</a>
documentation for more defails):
<dl>
<dt>; !syntax("abnf")</dt>
<dd>Indicates BNF variant used in the specification. (A file describing a syntax
of the BNF grammar.) Syntax specifications uploaded via this form <b>must</b>
include a single !syntax tag.<br/>
The value may be "abnf" or "abnf-rfc1035". This refers to the ABNF variant
defined in RFC 2234/4234
(file <a href="share/syntax/abnf.conf" target="_blank">abnf.conf</a>), or
the ABNF variant used in RFC 1035
(file <a href="share/syntax/abnf-rfc1035.conf" target="_blank">abnf-rfc1035.conf</a>).
If you need more, please,
<a href="mailto:bnfparser2-users@lists.sourceforge.net">tell us</a>.
</dd>
<dt>; !import("ALPHA", "CHAR", "DIGIT", "rfc2234-6.1.abnf")</dt>
<dd>Lists symbol(s) defined in another specification. Syntax specifications
uploaded via this form may only reference the .abnf specifications listed above.</dd>
</dl>
</p>

<br/>
<p class="credits">
Powered by <a href="http://bnfparser2.sourceforge.net" target="_blank">BNF Parser&sup2;</a>.
Credits <a href="http://iti.fi.muni.cz" target="_blank">Institute for Theoretical Computer Science (ITI)</a> research center, Faculty of Informatics, Masaryk University Brno
and
<a href="http://www.anfdata.cz" target="_blank">ANF DATA spol. s r.o.</a>, Siemens IT Solutions and Services, PSE Czech Republic.
</p>
<p class="credits">
Web interface &copy; 2007 ANF DATA spol. s r.o.
</p>
</body>
</html>
