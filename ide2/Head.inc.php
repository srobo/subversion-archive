<?php
$dbhost = 'linuxproj.ecs.soton.ac.uk';
$dbuser = 'pjcl106';
$dbpass = 'sr_cheese';

$conn = mysql_connect($dbhost, $dbuser, $dbpass) or die ('Error connecting to mysql');
$query = "SELECT * FROM pjcl106.ide2";
$dbname = 'db_pjcl106';
$result = mysql_query($query);

mysql_close($conn);

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

function make_MySQL_query($search)
{
	global $MySQL_table;

	$where	= $order	= "";

	switch($search)	//do the WHERE's
	{
		case "unfinished":
			$where	= "completed=0";
			break;
		case "unchecked":
			$where	= " && signoff_date=0";
		case "finished":
			$where	= "completed>0$where";
			break;
		case "new":
			$where	= "new";
			break;
		case "reminders":
			$where	= "reminders";
			break;
		case "unread":
			$where	= "unread";
			break;
		default:
			break;
	}
	switch($search)	//do the ORDER BY's & tables
	{
		case "unfinished":
		case "finished":
		case "unchecked":
			$order	= "deadline";
			$table	= $MySQL_checklist_table;
			break;
		case "new":
			$order	= "new";
			$table	= $MySQL_news_table;
			break;
		case "reminders":
			$order	= "reminders";
			$table	= $MySQL_news_table;
			break;
		case "unread":
			$order	= "unread";
			$table	= $MySQL_news_table;
			break;
		default:
			break;
	}
	return "SELECT * FROM $table WHERE $where ORDER BY $order LIMIT 20";
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