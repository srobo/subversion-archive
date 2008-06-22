<?php
$deadline	= date("Y-j-d");
$related_docs_help	= $title	= $design_consider	= $description	= "";

include_once 'functions.inc.php';	//functions file
include_once 'config.inc.php';	//config file

$conn	= mysql_connect($db_host, $db_user, $db_pass) or die ("Error connecting to mysql");
mysql_select_db($db_name) or die ("Error: database $db_name does not exsist");

if(!empty($_GET))				//recover all passed info
	extract($_GET, EXTR_OVERWRITE);

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
list($day, $month, $year)	= split("[./-]",date("j.n.Y", strtotime($deadline)));
?>
FORM.title.value	= "<?php echo $title; ?>";
FORM.description.value	= "<?php echo $description; ?>";
FORM.design_consider.value	= "<?php echo $design_consider; ?>";
FORM.related_docs_help.value	= "<?php echo $related_docs_help; ?>";
FORM_category	= "<?php echo $related_docs_help; ?>";
FORM_day	= "<?php echo $day; ?>";
FORM_month	= "<?php echo $month; ?>";
FORM_year	= "<?php echo $year; ?>";
<?php 	mysql_close($conn); ?>