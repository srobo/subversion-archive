<?php
$debug_info	= "";

if($_SERVER['SERVER_NAME'] == $good_server)
{
	$conn	= mysql_connect($db_host, $db_user, $db_pass) or die ('Error connecting to mysql');
	mysql_select_db($db_name);
} else
	header("location: http://linuxproj.ecs.soton.ac.uk".$_SERVER['SCRIPT_NAME']);

include_once 'functions.inc.php';	//functions file
include_once 'config.inc.php';	//config file
include_once 'task_list.inc.php';	//file contains info in an array that mimics the db output


if(!empty($_COOKIE))
	extract($_COOKIE, EXTR_OVERWRITE);

if(!empty($_GET))				//recover all passed info
	extract($_GET, EXTR_OVERWRITE);

if(!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);

if(!isset($username))
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
	<?php if(isset($external_scripts))
		foreach(explode(", ", $external_scripts) as $script) { ?>
	<script type="text/javascript" src="<?php echo $script; ?>"></script>
	<?php } ?>
</head>
<body>
<div class="content">
	<div id="top"></div>
</div>

<div id="prec">
	<ul id="top_menu">
		<li><a href="./" title="home">IDE HOME</a></li>
<?php foreach($ide_sect as $row) { ?>
		<li><a href="<?php echo $row['link']; ?>" title="<?php echo $row['description']; ?>"><?php echo strtoupper($row['title']); ?></a></li>
<?php } ?>
	</ul>
</div>
<div class="content">
	<div id="main">
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

			<h3>Checklist Toolbox</h3>
			<ul>
			<?php foreach($checklist_toolbox as $tool_item)
			{
				if($page_n == "Checklist")
					$tool_item_link	= "javascript:get_checklist('".$tool_item['link']."');";
				else
					$tool_item_link	= "Checklist.php?search=".$tool_item['link'];
				$tool_item_num	= $tool_item['num'];//temporary - no db as yet: SELECT id FROM db WHERE ($search) then count rows
			?>
				<li><a href="<?php echo $tool_item_link; ?>" title="<?php echo $tool_item['description']; ?>"><?php echo $tool_item['title']." ($tool_item_num)"; ?></a></li>
			<?php } ?>
			</ul>

			<?php if($page_n == "Checklist") { ?>
			<h3>Checklist Help</h3>
			<ol>
				<li>Read through the checklist</li>
				<li>Complete the task</li>
				<li>When you think you're done, click 'Task Completed'</li>
				<li>Ask a mentor to confirm you've done the task</li>
				<li>Your mentor will then sign off the task</li>
				<li>Move on to the next task</li>
			</ol>
			<?php } ?>

			<h3>Recent Activity</h3>
			<ul>
				<li><a href="#" title="#">Flamming ducks wrote 'hello world' script</a></li>
				<li><a href="#" title="#">Taunton incinerated their boards</a></li>
				<li><a href="#" title="#">Team 7 finished the chasis</a></li>
				<li><a href="#" title="#">Team 5 passed the wiring test</a></li>
			</ul>
		</div><!-- end right_side -->

		<div id="left_side">