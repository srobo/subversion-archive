<?php
if(!empty($_GET))				//recover all passed info
	extract($_GET, EXTR_OVERWRITE);

if(!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);
?>
<html>
<head>
<title>Contact Student Robotics</title>
<style type="text/css">
#contact_tbl	{ width: 590px; border: none; margin-left: 60px; }
#contact_tbl th	{ width: 125px; line-height: 1.6em; vertical-align: top; text-align: right; padding-right: 8px; }
#contact_tbl .text_in	{ width: 401px; font-family: Verdana, Arial, Helvetica, sans-serif; margin-left: 1px; }
#reset, #submit	{ width: 238px; margin: 2px 0 0 3px; line-height: 1.6em; padding: 0; font-family: Tahoma, sans-serif; }
</style>
</head>
<body>
		<form method="post" action="">
			<table id="contact_tbl">
				<tr>
					<td colspan="2" class="center">
					<span class="f_right">All fields are required</span>
					<h4>Send an email to the Committee</h4>
					</td>
				</tr><tr>
					<th>
						<label for="from_name">Name</label>
					</th>
					<td>
						<input id="from_name" name="from_name" class="text_in" type="text" />
					</td>
				</tr><tr>
					<th>
						<label for="from_email">eMail</label>
					</th>
					<td>
						<input id="from_email" name="from_email" class="text_in" type="text" />	
					</td>
				</tr><tr>
					<th>
						<label for="subject">Subject</label>
					</th>
					<td>
						<input id="subject" name="subject" class="text_in" type="text" value="<?php echo $subject; ?>" />
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
						<input class="tick" type="checkbox" name="mailing_list" id="mailing_list" value="mail" />
						<label for="mailing_list">Please include me in the Student Robotics mailing list.</label>
					</td>
				</tr><tr>
					<td colspan="2" class="center">
						<input type="hidden" name="ReturnURL" value="<?php echo htmlspecialchars($referrer); ?>" />
						<input type="hidden" name="debug" value="<?php echo $debug; ?>" />
						<input type="reset" value="Reset" id="reset" />
						<input id="submit" name="contact_join_submit" type="submit" value="Send" />
					</td>
				</tr>
			</table>
		</form>
</body>
</html>