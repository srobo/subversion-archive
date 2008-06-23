<?php
include_once 'functions.inc.php';

$_search_type	= isset($_GET['search']) ? $_GET['search'] : "all";

$result	= mysql_query(make_MySQL_query($_search_type, "*"));

//echo mysql_num_rows($result);

if(!$result)
	echo mysql_error();
else
while($row	= mysql_fetch_assoc($result))
{
//foreach($task_list as $row)

$res1	= mysql_query("SELECT team_id FROM tracker WHERE completed>0 && task_id=".$row['task_list_id']);	//check it really exists
if(!$res1)
{
	$debug_info .= "\n<br />mysql_error() = '".mysql_error()."'";
	echo mysql_error();
} else
	$teams_completed	= mysql_num_rows($res1);

$res2	= mysql_query("SELECT * FROM tracker WHERE team_id=$team_number && task_id=".$row['task_list_id']);	//check it really exists
if(!$res2)
{
	$debug_info .= "\n<br />mysql_error() = '".mysql_error()."'";
	echo mysql_error();
} else
	$row2	= mysql_fetch_assoc($res2);

?>
			<h3 id="task_<?php echo $row['task_list_id']; ?>" onclick="showHide('task_<?php echo $row['task_list_id']; ?>_details')" title="Click to expand task">
				<span><?php echo $row['task_list_id']; ?> - <?php echo $row['title']; ?></span>
			</h3>
			<div class="task_attr_container">
			<table class="task_attr_tbl">
				<tr><th>Task:</th><td colspan="3"><?php echo $row['description']; ?></td></tr>
				<tr><th>Deadline:</th><td class="L"><?php $deadline	= strtotime($row['deadline']);
					$date	= date("D j M Y", $deadline);
					echo $date;
					if($row2['completed'] == 0)
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
				<tr><th><?php if($row2['completed'] == 0)
					{ ?>Incomplete!</th><td class="L">
				<?php }
					elseif($row2['signoff_date'] != 0)
					{ ?>Signed Off:</th><td class="L">
						<?php echo date("D j M Y", $row2['signoff_date'])." by ".$row2['signoff_mentor'];
					} else { ?>Completed:</th><td class="L">
						<?php echo date("D j M Y", $row2['completed']);
					} ?></td>
					<th>Completed by:</th><td class="R"><?php echo $teams_completed." Team".($teams_completed == 1 ? "": "s"); ?></td></tr>
				</table>

				<div id="task_<?php echo $row['task_list_id']; ?>_details" style="display: none;">

					<div class="rs"><strong>Design considerations</strong>:
<?php echo wiki2html($row['design_consider']); ?>
					</div>
					<div class="rs"><strong>Relevent Documentation/Help</strong>:
<?php echo wiki2html($row['related_docs_help']); ?>
					</div>
					<strong>Team Comments</strong>
					<span style="float: right;"><a href="edit.htm" title="Edit the comments">Edit</a></span>
					<blockquote>
					<p><?php echo $row2['team_comments']; ?></p>
					</blockquote>
					<strong>Mentor Comments</strong>
					<span style="float: right;"><a href="edit.htm" title="Edit the comments">Edit</a></span>
					<blockquote>
					<p><?php echo $row2['mentor_comments']; ?></p>
					</blockquote>
<?php if($row2['completed'] == 0 || user_is_mentor($username)) { ?>
					<form id="signoff_task_<?php echo $row['task_list_id']; ?>" action="" method="get">
						<p>
							<input type="hidden" name="task" value="<?php echo $row['task_list_id']; ?>" />
<?php if(($row2['signoff_date'] == 0 && user_is_mentor($username)) || $row2['completed'] == 0) { ?>
							<input type="submit" value="<?php echo $button_val; ?>" name="signoff" class="signoff" />
<?php }	//if task incomplete or (mentor and not signed off) - completion
	if($row2['completed'] != 0 && user_is_mentor($username)) { ?>
							<input type="submit" value="<?php echo $button_val2; ?>" name="reject" class="signoff" />
<?php } //if task complete and mentor - rejection ?>
						</p>
					</form>
<?php } //if task incomplete or mentor - completion ?>
				</div>
			</div>
			<!-- New Task -->
<?php }	//end foreach ?>