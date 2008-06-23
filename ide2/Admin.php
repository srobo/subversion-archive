<?php
$external_scripts	= "Admin.js";
$onload	= "page_load()";
include 'Head.inc.php';

$logged_in	= TRUE;

if(!$logged_in)
	print_logon_form();
else
{
$deadline	= "now";
$related_docs_help	= $title	= $design_consider	= $description	= "";

if(isset($edit_id))
{
	$result	= mysql_query("SELECT * FROM task_list WHERE task_list_id=$edit_id");
	if(!$result)
		echo mysql_error();
	else
	{
		$row	= mysql_fetch_assoc($result);
		if(!empty($row))
			extract($row, EXTR_OVERWRITE);
	}
}
	if(isset($success)) echo print_success($success); ?>
	<form id="admin_form" action="admin_handler.php" method="post" onsubmit="return Validate_On_Admin_Submit('admin_form')">
		<div class="admin_form_head">
	<?php	if($debug)	echo "<input type=\"hidden\" name=\"debug\" value=\"$debug\" />";
			if(isset($edit_id))	echo "<input type=\"hidden\" name=\"edit_id\" value=\"$edit_id\" />"; ?>
			<h3>Edit or Add a task.</h3>
			<span class="f_right"><input type="submit" value="Save" />
			<br /><input type="reset" value="Reset" /></span>
		</div>
		<div class="admin_form">
			<table>
				<tr>
					<th>Deadline:</th>
					<td colspan="3"><?php
					list($day, $month, $year)	= split("[./-]",date("j.n.Y", strtotime($deadline)));

					$debug_info .= "\$day=$day,	\$month=$month,	\$year=$year\n<br />\n";

					genDateSelector("_", $day, $month, $year);
					?>
						<script type="text/javascript">
							<!--
							// fix number of days for the $month/$year that you start with
							// Note: The 2nd arg must match the first in the call to genDateSelector above.
							function page_load()
							{
								ChangeOptionDays(document.admin_form, '_');
							}
							// -->
						</script>
					</td>
				</tr><tr>
					<th>
						<label for="title">Title:</label>
					</th>
					<td colspan="3">
						<input name="title" id="title" value="<?php echo $title; ?>" />
					</td>
				</tr><tr>
					<th>
						<label for="category">Category:</label>
					</th>
					<td>
						<select name="category" id="category">
<?php foreach(array("Electronics", "Mechanical") as $cat)
{
			if($cat == $category)
				$selected	= " selected=\"selected\"";
			else
				$selected	= "";
	echo "\n	<option value=\"$cat\"$selected>$cat</option>";
}
?>						</select>
					</td>
					<th>
						<label for="edit_id">Task ID:</label>
					</th>
					<td>
						<select name="edit_id" id="edit_id" onchange="get_new_task('admin_form', this.value)">
	<option value="New">New</option><?php
	$result	= mysql_query("SELECT task_list_id, title FROM task_list WHERE 1 ORDER BY task_list_id");
	if(!$result)
		echo mysql_error();
	else
	{
		while($row_2	= mysql_fetch_assoc($result))
		{
			if($row_2['task_list_id'] == $edit_id)
				$selected	= " selected=\"selected\"";
			else
				$selected	= "";
			echo "\n	<option value=\"".$row_2['task_list_id']."\"$selected>".$row_2['task_list_id']." - ".$row_2['title']."</option>";
		}
	}
?>
						</select>
					</td>
				</tr>
			</table>
			<h4 class="f_left"><label for="description">Description: (Plain text only)</label></h4>
			<textarea id="description" name="description" class="text_in" rows="6" cols="62"><?php echo $description; ?></textarea>
			<h4 class="f_left"><label for="design_consider">Design Considerations: (Wiki markup supported)</label></h4>
			<textarea id="design_consider" name="design_consider" class="text_in" rows="10" cols="62"><?php echo $design_consider; ?></textarea>
			<h4 class="f_left"><label for="related_docs_help">Related Documnets and other Help: (Wiki markup supported)</label></h4>
			<textarea id="related_docs_help" name="related_docs_help" class="text_in" rows="10" cols="62"><?php echo $related_docs_help; ?></textarea>
		</div>
	</form>
	<?php }	// end if logged in ?>
<?php
include 'Foot.inc.php';
?>