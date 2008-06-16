<?php
$right_side	= TRUE;

include 'Head.inc.php';

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
<?php }	//end foreach 

include 'Foot.inc.php';
?>