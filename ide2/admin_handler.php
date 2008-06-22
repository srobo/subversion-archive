<?php
if (!empty($_GET))
	extract($_GET, EXTR_OVERWRITE);

if (!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);

$error	= $debug_info	= "";
$logged_in	= FALSE;	//just in case

include "functions.inc.php";	//contains my general functions - the file system gallery ones are now separate
include "config.inc.php";			//these files are now included in all the cathsoc pages since I'm using lots of functions

if($description == "" || $description ==  " " || !isset($description))
{
	$error	.= "You have not specified a description";
	$debug_info	.= "\nno description suplied\n<br />";
} elseif($design_consider == "" || $design_consider ==  " " || !isset($design_consider))
{
	$error	.= "You have not specified a design_consider";
	$debug_info	.= "\nno design_consider suplied\n<br />";
} elseif($related_docs_help == "" || $related_docs_help ==  " " || !isset($related_docs_help))
{
	$error	.= "You have not specified a related_docs_help";
	$debug_info	.= "\nno related_docs_help suplied\n<br />";
} elseif($category == "" || $category ==  " " || !isset($category))
{
	$error	.= "You have not specified a category";
	$debug_info	.= "\nno category suplied\n<br />";
} elseif($title == "" || $title ==  " " || !isset($title))
{
	$error	.= "You have not specified a title";
	$debug_info	.= "\nno title suplied\n<br />";
} else {

$conn	= mysql_connect($db_host, $db_user, $db_pass) or die ("Error connecting to mysql");
mysql_select_db($db_name) or die ("Error: database $db_name does not exsist");

if($_month < 10)
	$_month	= "0$_month";

if($_day < 10)
	$_day	= "0$_day";

$deadline	= "$_year-$_month-$_day";

$debug_info .= "\$deadline=$deadline\n<br />\$description=$description\n<br />\$title=$title\n<br />\$related_docs_help=$related_docs_help\n<br />\$design_consider=$design_consider\n<br />\n";

if(isset($edit_id) && $edit_id != "New")	//if we're editing
{
	$result	= mysql_query("SELECT * FROM task_list WHERE task_list_id=$edit_id");	//check it really exists
	if(!$result)
	{
		$debug_info .= "\n<br />mysql_error() = '".mysql_error()."'";
		$mysql_query	= "";
		echo mysql_error();
	} else {
		$row = mysql_fetch_assoc($result);
		$mysql_query	= "UPDATE task_list SET title='$title', description='$description', design_consider='$design_consider',"
						." related_docs_help='$related_docs_help', category='$category', deadline='$deadline' WHERE task_list_id=$edit_id";
	}
}

if(!isset($edit_id) || $edit_id == "New" || (isset($row) && $row['task_list_id'] != $edit_id))	//we're making a new one
{
	$mysql_query	= "INSERT INTO task_list (title, description, design_consider, related_docs_help, deadline, category)"
					." VALUES ('$title', '$description', '$design_consider', '$related_docs_help', '$deadline', '$category')";
}

$result	= mysql_query($mysql_query);	//check it really exists
if(!$result)
{
	$debug_info .= "\n<br />mysql_error() = '".mysql_error()."'";
	echo mysql_error();
}

$debug_info .= "\$mysql_query='$mysql_query'\n<br />\n";

}	//end if bad content

if(isset($debug) && $debug)
{
	echo "\n<br />POST";
	foreach($_POST as $key => $val)
		echo "<br />$key	= $val\n";
	echo "\n<br />GET";
	foreach($_GET as $key => $val)
		echo "<br />$key	= $val\n";
	echo "\n<br />\$debug_info:\n<br />$debug_info";
}

mysql_close($conn);
if((isset($error) && $error != "") || (isset($debug) && $debug))
	include "handler.php";
else
	header("Location: Admin.php?success=1");
?>