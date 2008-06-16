<?php
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
foreach($ide_sect as $row)
{
?>
			<h3 id="ide_sect_<?php echo $row['id']; ?>" onclick="showHide('ide_sect_<?php echo $row['id']; ?>_details')">
				<span><?php echo $row['id']; ?> - <?php echo $row['title']; ?></span>
			</h3>
			<div class="task_attr_container">
				<div class="rs"><strong>Description</strong>:
<?php echo wiki2html($row['description']); ?>
				</div>
				<div class="rs"><strong>Relevent Documentation/Help</strong>:
<?php echo wiki2html($row['related_docs_help']); ?>
				</div>
				<div id="ide_sect_<?php echo $row['id']; ?>_details" style="display: none;">
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
				</div>
			</div>
			<!-- New Task -->
<?php }	//end foreach ?>
		</div><!-- end left_side -->

	</div><!-- end main -->
<?php
include 'Foot.inc.php';
?>