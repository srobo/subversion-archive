<?php
/*
$dbhost = 'mysql2.ecs.soton.ac.uk';
$dbuser = 'cc1206';

$conn = mysql_connect($dbhost, $dbuser, $dbpass) or die ('Error connecting to mysql');
$query = "SELECT * FROM cc1206.tids";
$dbname = 'cc1206';
$result = mysql_query($query);

mysql_close($conn);
*/

function wiki2html($wikitext)
{
	if(!isset($wikitext) || $wikitext == "")
		return FALSE;

	$inter_text	= $wikitext;
	while(strpos($inter_text,"[[") && strpos($inter_text,"]]"))
	{
		$link	= str_replace(array("[[", "]]"), "", substr($inter_text, strpos($inter_text, "[["), (strpos($inter_text, "]]")-strpos($inter_text, "[["))));
		if(strpos($link, "|"))
			list($href, $title)	= explode("|", $link);
		else
			$href	= $title	= $link;
		$inter_text	= str_replace("[[$link]]", "<a href=\"$href\">$title</a>", $inter_text);
	}

	if(strpos($inter_text,"\n*") || strpos($inter_text,"*") === 0)
		$inter_text	= "\n<ul>".str_replace("*", "\n	<li>", str_replace("\n*", "</li>\n	<li>", $inter_text))."</li>\n</ul>\n";

	return $inter_text;
}

function user_is_mentor($username)
{
	global $mentor_list;
	return in_array($username, $mentor_list);
}

/* file contains info in an array that mimics the db output */
include 'task_list.inc.php';

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
		<li><a href="./" title="home">IDE HOME</a></li>
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
	<div id="main">
<?php if($right_side) { ?>
		<div id="right_side">
			<h3>User Details</h3>
			<table id="user_info">
			<tr><th>Username:</th><td><?php echo $username; ?></td></tr>
			<tr><th>User Status:</th><td><?php echo (user_is_mentor($username) ? "Mentor" : "Student"); ?></td></tr>
			<tr><th>Team Number:</th><td><?php echo $team_number; ?></td></tr>
			<tr><th>School:</th><td><?php echo $school; ?></td></tr>
			<?php if($team_number) { ?>
			<tr><th>Team Mentor:</th><td><?php echo $mentor_name; ?></td></tr>
			<?php } ?>
			</table>

			<h3>Toolbox</h3>
			<ul>
				<li><a href="#" title="Articles">Unfinished Tasks (78)</a></li>
				<li><a href="#" title="Gallery">Finished Tasks (10) </a></li>
				<li><a href="#" title="Affiliates">Unchecked Tasks (2)</a></li>
				<li><a href="#" title="Articles">Reminders (2)</a></li>
				<li><a href="#" title="Abous us">New Tasks (2)</a></li>
				<li><a href="#" title="Contact">Unread Messages (1)</a></li>
			</ul>

			<h3>Help</h3>
			<ol>
				<li>Read through the checklist</li>
				<li>Complete the task</li>
				<li>When you think you're done, click 'Task Completed'</li>
				<li>Ask a mentor to confirm you've done the task</li>
				<li>Your mentor will then sign off the task</li>
				<li>Move on to the next task</li>
			</ol>

			<h3>Recent Activity</h3>
			<ul>
				<li><a href="#" title="#">Flamming ducks wrote 'hello world' script</a></li>
				<li><a href="#" title="#">Taunton incinerated their boards</a></li>
				<li><a href="#" title="#">Team 7 finished the chasis</a></li>
				<li><a href="#" title="#">Team 5 passed the wiring test</a></li>
			</ul>
		</div><!-- end right_side -->

		<div id="left_side">
<?php } ?>