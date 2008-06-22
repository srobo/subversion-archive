<?php
	include 'Head.inc.php';
?>
<?php
if(isset($error) && $error != "")
{
	if(isset($referrer))
		$referrer	= array_shift(explode('?', basename($referrer)));
	else
		$referrer	= "./";
?>
	You generated an error! Please	
	<a href="mailto:web@studentrobotics.org?subject=Web%20Form%20Error&body=<? echo htmlspecialchars($debug_info.$error); ?>" title="email the webmaster">eMail</a>
	the Web Master if you believe that the script was in error.
	<br />The details of the error are:<br /><? echo $error; ?><br />
	Once you have feel free to <a href="<?php echo $referrer."?success=0"; ?>" title="Did you email the webmaster?">return</a> to the page you were on page.
<?php
}

if(isset($debug) && $debug)
	echo $debug_info;

	include 'Foot.inc.php';
?>