<?php
/* file contains info in an array that mimics the db output */
include 'task_list.inc.php';

function user_is_mentor($username)
{
	global $mentor_list;
	return in_array($username, $mentor_list);
}

if(isset($_POST['username']))
	$username	= $_POST['username'];
elseif(isset($_GET['username']))
	$username	= $_GET['username'];
else
	$username	= "Test Student";

if(user_is_mentor($username))
{
	$button_val	= "Sign Off Task";
	$button_val2	= "Task Incomplete";
	$school	= "SR";
	$team_number	= 0;
} else {
	$button_val	= "Task Completed";
	$school	= "Somewhere College";	//look it up in the db
	$team_number	= rand(1, 20);	//look it up in the db
	$mentor_name	= $mentor_list[($team_number % count($mentor_list))];	//look it up in the db
}


$page_arr	= explode("/", $_SERVER['PHP_SELF']);	//prep for page titles
$this_page	= $page_arr[count($page_arr)-1];
$page_n		= substr($this_page, 0, strlen($this_page)-4);	//remove .php
if($page_n == "index")
	$page_n		= "Home";
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>Student Progress Tracker | <?php echo $page_n; ?></title>
	<meta http-equiv="Content-Language" content="English" />
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<link rel="stylesheet" type="text/css" href="style.css" media="screen" />
	<link rel="alternate" type="application/rss+xml" title="RSS 2.0" href="rss/" />
	<script type="text/javascript" src="script.js"></script>
</head>
<body>
<div class="content">
	<div id="top"></div>
</div>

<div id="prec">
	<ul id="top_menu">
		<li><a href="#" title="home">IDE HOME</a></li>
		<li><a href="#" title="Articles">FORUMS</a></li>
		<li><a href="Checklist.php" title="Task Checklist">CHECK LIST</a></li>
		<li><a href="Graphs.php" title="Progress Graphs">PROGRESS GRAPHS</a></li>
		<li><a href="#" title="Affiliates">DOCUMENTATION</a></li>
		<li><a href="#" title="Articles">SIMULATOR</a></li>
		<li><a href="#" title="Abous us">MESSAGES</a></li>
		<li><a href="Contact.php" title="Contact your mentor or the team">CONTACT</a></li>
	</ul>
</div>
<div class="content">