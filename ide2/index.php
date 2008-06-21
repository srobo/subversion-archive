<?php
$right_side	= TRUE;

include 'Head.inc.php';

//echo mysql_num_rows($result);
//while($row = mysql_fetch_array($result, MYSQL_ASSOC))
foreach($ide_sect as $row)
{
?>
			<h3 id="ide_sect_<?php echo $row['id']; ?>" onclick="showHide('ide_sect_<?php echo $row['id']; ?>_details')" title="Click to expand section">
				<span><a href="<?php echo $row['link']; ?>" title="Click to view this section of the site"><?php echo $row['title']; ?></a></span>
			</h3>
			<div class="task_attr_container">
				<div class="rs"><strong>Description</strong>:
<?php echo wiki2html($row['description']); ?>
				</div>
				<div id="ide_sect_<?php echo $row['id']; ?>_details" style="display: none;">
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
				</div>
			</div>
			<!-- New Task -->
<?php }	//end foreach 

include 'Foot.inc.php';
?>