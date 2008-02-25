<?php
if (!empty($_GET))
	extract($_GET, EXTR_OVERWRITE);

if (!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);

unset($error);
if($message == "")
	$error	.= "\nPlease include a message.\n<br />";

if($from_name == "")
	$error	.= "\nPlease your name.\n<br />";

if($from_email == "")
	$error	.= "\nPlease your email address.\n<br />";

$message	.= "\n\n--\n"."X-Mailer: PHP/".phpversion();	//mail signature, including php version

$header		.= "From: Student Robotics Recruitment Web Form <Recruitment.web-form@studentrobotics.org>"."\r\n"."Reply-To: $from_name <$from_email>";
$to	= "Student Robotics Committee <committee@studentrobotics.org>";

if(!isset($error))
{
	if(!mail($to, $subject, $message, $header))	//send the mail, checking for errors
		$error .= "\nYour email failed to send. please see \$debug_info for more information";
	else
		$debug_info .= "\nmail() reported no errors.\n<br />";
}
$debug_info	.= "\$to=$to\n<br />\$subject=$subject\n<br />\$message=$message\n<br />\$header=$header\n<br />";

if(isset($error) || $debug)
	include "index.php";
else
	header("Location: ?success=1");
?>