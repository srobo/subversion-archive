<?php
include 'Head.inc.php';
?>
	<div id="main">
	<form method="post" action="" id="contact" onsubmit="return Validate_On_Contact_Submit('contact')">
		<table id="contact_tbl">
			<tr>
				<td colspan="2" class="center">
				<span class="f_right">All fields are required</span>

				<h4>Ask for help from <? if($team_number != 0) { ?>your mentor (<?php echo $mentor_name; ?>) or <?php } ?>the SR Mentor Netwrok</h4>
				<? if($team_number != 0) { ?></td>
			</tr><tr>
				<th>
					<label for="to">To:</label>
				</th>
				<td>
					<input type="radio" name="to" value="mentor" id="_mentor" checked="checked" />
					<label for="_mentor">Your Mentor (<?php echo $mentor_name; ?>)</label>
					<input type="radio" name="to" value="team" id="_team" />
					<label for="_team">The SR Team</label>
				<?php } else { ?><input type="hidden" name="to" value="team"><?php } ?></td>
			</tr><tr>
				<th>
					<label for="subject">Subject</label>
				</th>
				<td>
					<input id="subject" name="subject" class="text_in" type="text" value="" />
				</td>
			</tr><tr>
				<th>
					<label for="message">Message</label>
				</th>

				<td>
					<textarea id="message" name="message" class="text_in" rows="6" cols="51"></textarea>
				</td>
			</tr><tr>
				<td colspan="2" class="center">
					<input type="hidden" name="from_user" value="<?php echo $username; ?>" />
					<input type="hidden" name="debug" value="<?php echo $debug; ?>" />
					<input id="contact_reset" type="reset" value="Reset" />
					<input id="contact_submit" name="contact_submit" type="submit" value="Send" />
				</td>
			</tr>
		</table>
	</form>
	</div><!-- end main -->
<?php
include 'Foot.inc.php';
?>