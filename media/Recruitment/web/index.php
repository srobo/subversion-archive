<?php
$page	= "index";	//just in case

if (!empty($_GET))
	extract($_GET, EXTR_OVERWRITE);

if (!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);

if(isset($contact_join_submit))
	$page	= "error";

$page_n		= $page;
$page_filename	= "$page.inc.htm";

$page_top_t	= $page_n;	//the title at the top of the page

$page_title	= ucwords($page_n);
switch ($page_n)	//page name
{
	case "competition":
		$page_title	= "The Competition";
		break;
	case "join":
	case "contact":
		$page_title	.= " The Team";
		break;
	case "why":
		$page_title	.= " Join?";
		break;
	case "faq":
		$page_title	= "FAQ";
		break;
	case "index":
		$page_title	= "Student Robotics";
		break;
	default:
		break;
}

$page_title_head	= ($page_n != "index" ? "Student Robotics >> " : "").$page_title	;	//the page title (in the head section)

$referrer		= isset($_SERVER['HTTP_REFERER']) ? $_SERVER['HTTP_REFERER'] : "?page=".$page_n;

?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<!--this design was created by Vacant (Chris Blunden), http://www.web-site.tk .
Deep, version 1.5 created on 17/10/04 for OSWD.org ... DO NOT REMOVE this notice. Thanks. -->
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
<title><?php echo $page_title_head; ?></title>
<link rel="stylesheet" type="text/css" href="style.css" />
</head>
<body>
<?php if($page_n == "index") { ?>
<a href="http://www.studentrobotics.org" title="See our main site" id="index_SR_link"></a>
<?php } //end if index
if(isset($success)) { if($success == 1){?>
<div class="titleblock" style="right: 30px; top: 10px; position: absolute; font-size: 16px;">
Your email was sent successfully.
</div>
<?php }} //end if success ?>
<div id="outer">
	<div id="inner">
<?php if($page_n != "index") { ?>
		<div id="close"><a href="./" title="close and return to home page">X</a></div>
		<div id="top"><a href="#top" title="back to top" name="top" id="top">^</a></div>
		<div id="urltext"><a href="./" class="navigurl" title="click to return to the home page">home &raquo; </a><?php echo strtolower($page_title); ?></div>
		<div id="content">
<?php } //end if not index

if(is_readable($page_filename))
	include $page_filename;
else
	echo "<div class=\"titleblock\">Unknown Page</div>\n<p class=\"fix\" >\nThis page does not exist yet."
	." If you think it should please\n<a href=\"mailto:webmaster@studentrobotics.org?subject=$page%20page\" title=\"eMail them\">email</a>\nthe webmaster.</p>";

if($page_n != "index") { ?>
		</div><!-- end content -->
<?php } //end if not index ?>
	</div><!-- end inner -->
</div><!-- end outer -->
</body>
</html>
