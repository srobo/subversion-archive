<?php
if (!empty($_COOKIE))
	extract($_COOKIE, EXTR_OVERWRITE);

if (!empty($_GET))
	extract($_GET, EXTR_OVERWRITE);

if (!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);

	include_once("config.inc.php");

unset($error);
if($username == "")
	$error	.= "\nPlease login to send a message.\n<br />";

if($subject == "")
	$error	.= "\nPlease include a subject.\n<br />";

if($message == "")
	$error	.= "\nPlease include a message.\n<br />";

$message	.= "\n From User: $username\n"."X-Mailer: PHP/".phpversion();	//mail signature, including php version

$header		.= "From: $website_name Web Form <$website_form_email>";

if($to == "team")
{
	if(!isset($error))
	{
		if(!mail($mentors_emails, $subject, $message, $header))	//send the mail, checking for errors
			$error .= "\nYour email failed to send. please see \$debug_info for more information";
		else
			$debug_info .= "\nmail() reported no errors.\n<br />";
	}
} else {
	//send the mentor a message - ?email? ?mysql?
}

$debug_info	.= "\$to=$to\n<br />\$subject=$subject\n<br />\$message=$message\n<br />\$header=$header\n<br />";

if(stristr($ReturnURL, "?"))
	$ReturnURL	.= "&";
else
	$ReturnURL	.= "?";

if(isset($error) || $debug)
	include "handler.php";
else
	header("Location: ${ReturnURL}success=1");
?>