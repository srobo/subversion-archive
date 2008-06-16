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
		$out	= "\n<ul>".str_replace("*", "\n	<li>", str_replace("\n*", "</li>\n	<li>", $inter_text))."</li>\n</ul>\n";

	return $out;
}

function user_is_mentor($username)
{
	return in_array($username, array("Peter", "Chris", "Steve", "Rob"));
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
}
else
{
	$button_val	= "Task Completed";
	$school	= "Somewhere College";	//look it up in the db
	$team_number	= 9;	//look it up in the db
}


/* file contains info in an array that mimics the db output */
include 'task_list.inc.php';
include 'Head.inc.php';

?>
	<div id="main">
		<div id="right_side">
			<h3>User Details</h3>
			<table id="user_info">
			<tr><th>Username:</th><td><?php echo $username; ?></td></tr>
			<tr><th>Team Number:</th><td><?php echo $team_number; ?></td></tr>
			<tr><th>School:</th><td><?php echo $school; ?></td></tr>
			<tr><th>User Status:</th><td><?php echo (user_is_mentor($username) ? "Mentor" : "Student"); ?></td></tr>
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
<?php
//echo mysql_num_rows($result);
//while($row = mysql_fetch_array($result, MYSQL_ASSOC))
foreach($task_list as $row)
{
?>
			<h3 id="task_<?php echo $row['id']; ?>" onclick="showHide('task_<?php echo $row['id']; ?>_details')">
				<span><?php echo $row['id']; ?> - <?php echo $row['title']; ?></span>
			</h3>
			<div class="task_attr_container">
			<table class="task_attr_tbl">
				<tr><th>Task:</th><td colspan="3"><?php echo $row['description']; ?></td></tr>
				<tr><th>Deadline:</th><td class="L"><?php $deadline	= strtotime($row['deadline']);
					$date	= date("D j M Y", $deadline);
					echo $date;
					if($row['completed'] == 0)
					{
						$overdue	= floor((time()-$deadline)/(24*60*60));
						if($overdue < -1 && $overdue > -5)
							$overdue_comment	= ", Due in ".abs($overdue)." Day".($overdue == -1 ? "": "s");
						elseif($overdue == -1 || $overdue == 0)
							$overdue_comment	= ", <span class=\"today_tomprrow\">Due ".($overdue == 0 ? "Today" : "Tomorrow")."</span>";
						elseif($overdue > 0)
							$overdue_comment	= ", <span class=\"overdue\">Overdue by $overdue day".($overdue == 1 ? "": "s")."</span>";
						else
							$overdue_comment	= "";
						echo $overdue_comment;
					} ?></td>
				<th>Category:</th><td class="R"><?php echo $row['category']; ?></td></tr>
				<tr><th><?php if($row['completed'] == 0)
					{ ?>Incomplete!</th><td class="L">
				<?php }
					elseif($row['signoff_date'] != 0)
					{ ?>Signed Off:</th><td class="L">
						<?php echo date("D j M Y", $row['signoff_date'])." by ".$row['signoff_mentor'];
					} else { ?>Completed:</th><td class="L">
						<?php echo date("D j M Y", $row['completed']);
					} ?></td>
					<th>Completed by:</th><td class="R"><?php echo $teams_completed[$row['id']]." Team".($teams_completed[$row['id']] == 1 ? "": "s"); ?></td></tr>
				</table>

				<div id="task_<?php echo $row['id']; ?>_details" style="display: none;">

					<div class="rs"><strong>Design considerations</strong>:
<?php echo wiki2html($row['design_consider']); ?>
					</div>
					<div class="rs"><strong>Relevent Documentation/Help</strong>:
<?php echo wiki2html($row['related_docs_help']); ?>
					</div>
					<strong>Team Comments</strong>
					<span style="float: right;"><a href="edit.htm" title="Edit the comments">Edit</a></span>
					<blockquote>
					<p><?php echo $row['teams_comments']; ?></p>
					</blockquote>
					<strong>Mentor Comments</strong>
					<span style="float: right;"><a href="edit.htm" title="Edit the comments">Edit</a></span>
					<blockquote>
					<p><?php echo $row['mentor_comments']; ?></p>
					</blockquote>
<?php if($row['completed'] == 0 || user_is_mentor($username)) { ?>
					<form id="signoff_task_<?php echo $row['id']; ?>" action="" method="get">
						<p>
							<input type="hidden" name="task" value="<?php echo $row['id']; ?>" />
<?php if(($row['signoff_date'] == 0 && user_is_mentor($username)) || $row['completed'] == 0) { ?>
							<input type="submit" value="<?php echo $button_val; ?>" name="signoff" class="signoff" />
<?php }	//if task incomplete or (mentor and not signed off) - completion
	if($row['completed'] != 0 && user_is_mentor($username)) { ?>
							<input type="submit" value="<?php echo $button_val2; ?>" name="reject" class="signoff" />
<?php } //if task complete and mentor - rejection ?>
						</p>
					</form>
<?php } //if task incomplete or mentor - completion ?>
				</div>
			</div>
			<!-- New Task -->
<?php }	//end foreach ?>
		</div><!-- end left_side -->

	</div><!-- end main -->
<?php
include 'Foot.inc.php';
?>