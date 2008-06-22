<?php
$external_scripts	= "Admin.js";
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
		$row = mysql_fetch_assoc($result);
		if(!empty($row))
			extract($row, EXTR_OVERWRITE);
	}
}
	if(isset($success)) print_success($success); ?>
	<form id="admin_form" action="admin_handler.php" method="post" onsubmit="return Validate_On_Admin_Submit('admin_form')">
		<div class="admin_form_head">
	<?php	if($debug)	echo "<input type=\"hidden\" name=\"debug\" value=\"$debug\" />";
			if(isset($edit_id))	echo "<input type=\"hidden\" name=\"edit_id\" value=\"$edit_id\" />"; ?>
			<h3>Edit or Add a task.</h3>
			<span class="f_right"><input type="submit" value="Save" /><input type="reset" value="Reset" /></span>
		</div>
		<div class="admin_form">
			<table>
				<tr>
					<th>Deadline:</th>
					<td><?php
					list($day, $month, $year) = split("[./-]",date("j.n.Y", strtotime($deadline)));

					$debug_info .= "\$day=$day,	\$month=$month,	\$year=$year\n<br />\n";

					genDateSelector("", $day, $month, $year);
					?>
						<script type="text/javascript">
							<!--
							// fix number of days for the $month/$year that you start with\n"
							ChangeOptionDays(document.admin_form, ''); // Note: The 2nd arg must match the first in the call to genDateSelector above.\n"
							// -->
						</script>
					</td>
				</tr><tr>
					<th>
						<label for="title">Title:</label>
					</th>
					<td >
						<input name="title" id="title" value="<?php echo $title; ?>" />
					</td>
				</tr><tr>
					<th>
						<label for="category">Category:</label>
					</th>
					<td>
						<select name="category" id="category">
							<option value="Electronics">Electronics</option>
							<option value="Mechanical">Mechanical</option>
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