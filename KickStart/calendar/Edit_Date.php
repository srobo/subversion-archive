<?php
$users_arr["srobo"] = "fea0f1f6fede90bd0a925b4194deac11";

/* This function generates a Time selector */
function genTimeSelector($end, $hour, $minute)
{
	global $debug_info;
	$minute	= 5*round($minute/5);
	echo "\n				<select name=\"${end}hour\">";

	for($i = 0; $i <= 23; $i++)
		echo "\n					<option value=\"$i\"" . ($i == $hour ? " selected=\"selected\"" : "") . ">$i</option>";

	echo "\n				</select>\n				:\n				<select name=\"${end}minute\" >";

	for($i = 0; $i <= 56; $i+=5)
	{
		$debug_info .= "\$i=$i\n<br />\n";
		echo "\n					<option value=\"$i\"" . ($i == $minute ? " selected=\"selected\"" : "") . ">".($i<10?"0":"")."$i</option>";
	}

	echo "\n				</select>\n";
}

/* This function generates a date selector. from MRBS, with tweaks by me */
function genDateSelector($prefix, $day, $month, $year)
{
	if($day == 0)	$day	= date("d");	//if the current date isn't supplied
	if($month == 0)	$month	= date("m");
	if($year == 0)	$year	= date("Y");

	echo "\n				<select name=\"${prefix}day\">";

	for($i = 1; $i <= 31; $i++)
		echo "\n					<option value=\"$i\"" . ($i == $day ? " selected=\"selected\"" : "") . ">$i</option>";

	echo "\n				</select>\n				<select name=\"${prefix}month\" onchange=\"ChangeOptionDays(this.form,'$prefix')\">";

	for($i = 1; $i <= 12; $i++)
	{
		$m = strftime("%b", mktime(0, 0, 0, $i, 1, $year));

		print "\n					<option value=\"$i\"" . ($i == $month ? " selected=\"selected\"" : "") . ">$m</option>";
	}

	echo "\n				</select>\n				<select name=\"${prefix}year\" onchange=\"ChangeOptionDays(this.form,'$prefix')\">";

	$min = $year - 5;
	$max = $year + 5;

	for($i = $min; $i <= $max; $i++)
		print "\n					<option value=\"$i\"" . ($i == $year ? " selected=\"selected\"" : "") . ">$i</option>";

	echo "\n				</select>\n";
}

/* This function is only in php5(file_put_contents()), so I need to write it, but to avoid clashes I have changed the name slightly */
function file_put_content($file, $data)
{
	if(function_exists(file_put_contents))
		return file_put_contents($file, $data);

	$handle = fopen($file, 'w');

	if(!fwrite($handle, $data))
		return FALSE;

	return fclose($handle);
}

if(!empty($_COOKIE))
	extract($_COOKIE, EXTR_OVERWRITE);

if(!empty($_GET))				//recover all passed info
	extract($_GET, EXTR_OVERWRITE);

if(!empty($_POST))
	extract($_POST, EXTR_OVERWRITE);

session_start();	//start the php session, just in case

if(!isset($_username) || $_username == "")
{
	if(isset($_SESSION['user']) && $_SESSION['user'] != "")
		$_username	= $_SESSION['user'];
	elseif(isset($_SESSION['SR_user']) && $_SESSION['SR_user'] != "")
		$_username	= $_SESSION['SR_user'];
}

$_logged_in	= ($users_arr[$_username] == md5($_pass)) || (isset($_SESSION['SR_user']) && $users_arr[$_username] != "") ? TRUE : FALSE;

if($_logged_in)
{
	//use cookies only to handle session stuff
$debug_info		.="\n ini_set(\"session.use_only_cookies\", \"1\") = ".ini_set("session.use_only_cookies", "1")."\n<br />";
$debug_info		.="\n ini_set('url_rewriter.tags', '') = ".ini_set('url_rewriter.tags', '')."\n<br />";

$_SESSION['SR_user']	= $_username;
$_filename	= "Date.inc.php";
$DATE_OUT	= "<?\n\$Kick_DATE	= \"".implode("-", array($Kick_year, $Kick_month, $Kick_day,))." $Kick_hour:$Kick_minute\";\n"
			."\$Comp_DATE	= \"".implode("-", array($Comp_year, $Comp_month, $Comp_day,))."\";\n"
			."\$Easter_DATE	= \"".implode("-", array($Easter_year, $Easter_month, $Easter_day,))."\";\n?>";

if(isset($_saving))
	if(!file_put_content($_filename, $DATE_OUT))
		echo "file write failed";

include $_filename;

if(!isset($Comp_DATE))
	$Comp_DATE	= "2008-04-12";

if(!isset($Easter_DATE))
	$Easter_DATE	= "2008-03-23";

if(!isset($Kick_DATE))
	$Kick_DATE	= "2007-10-28 14:00";

?>
<html>
<head>
<title>Date Editing Page</title>
<script type="text/javascript">
/*	script inspired by "True Date Selector"
	 Created by: Lee Hinder, lee.hinder@ntlworld.com

	 Tested with Windows IE 6.0
	 Tested with Linux Opera 7.21, Mozilla 1.3, Konqueror 3.1.0

*/

function daysInFebruary (year){
	//	February has 28 days unless the year is divisible by four, and if it is the turn of the century
	//	then the century year must also be divisible by 400 when it has 29 days
	return (((year % 4 == 0) && ( (!(year % 100 == 0)) || (year % 400 == 0))) ? 29 : 28 );
}

//function for returning how many days there are in a month including leap years
function DaysInMonth(WhichMonth, WhichYear)
{
	var DaysInMonth = 31;
	if (WhichMonth == "4" || WhichMonth == "6" || WhichMonth == "9" || WhichMonth == "11")
	DaysInMonth = 30;
	if (WhichMonth == "2")
	DaysInMonth = daysInFebruary( WhichYear );
	return DaysInMonth;
}

//function to change the available days in a months
function ChangeOptionDays(formObj, prefix)
{
	var DaysObject = eval("formObj." + prefix + "day" );
	var MonthObject = eval("formObj." + prefix + "month" );
	var YearObject = eval("formObj." + prefix + "year" );

	if (DaysObject.selectedIndex && DaysObject.options)
	{ //	The DOM2 standard way
		//	alert("The DOM2 standard way" );
		var DaySelIdx = DaysObject.selectedIndex;
		var Month = parseInt(MonthObject.options[MonthObject.selectedIndex].value);
		var Year = parseInt(YearObject.options[YearObject.selectedIndex].value);
	}
	else if (DaysObject.selectedIndex && DaysObject[DaysObject.selectedIndex])
	{ //	The legacy ETBS way
		//	alert("The legacy ETBS way" );
		var DaySelIdx = DaysObject.selectedIndex;
		var Month = parseInt(MonthObject[MonthObject.selectedIndex].value);
		var Year = parseInt(YearObject[YearObject.selectedIndex].value);
	}
	else if (DaysObject.value)
	{ //	Opera 6 stores the selectedIndex in property 'value'.
		//	alert("The Opera 6 way" );
		var DaySelIdx = parseInt(DaysObject.value);
		var Month = parseInt(MonthObject.options[MonthObject.value].value);
		var Year = parseInt(YearObject.options[YearObject.value].value);
	}

	//	alert("Day="+(DaySelIdx+1)+" Month="+Month+" Year="+Year);

	var DaysForThisSelection = DaysInMonth(Month, Year);
	var CurrentDaysInSelection = DaysObject.length;
	if (CurrentDaysInSelection > DaysForThisSelection)
	{
		for (i=0; i<(CurrentDaysInSelection-DaysForThisSelection); i++)
		{
			DaysObject.options[DaysObject.options.length - 1] = null
		}
	}
	if (DaysForThisSelection > CurrentDaysInSelection)
	{
		for (i=0; i<DaysForThisSelection; i++)
		{
			DaysObject.options[i] = new Option(eval(i + 1));
		}
	}
	if (DaysObject.selectedIndex < 0)	DaysObject.selectedIndex = 0;
	if (DaySelIdx >= DaysForThisSelection)
		DaysObject.selectedIndex = DaysForThisSelection-1;
	else
		DaysObject.selectedIndex = DaySelIdx;
}
</script>
<style type="text/css">
a.visited	{ color: blue; }

#main_table	{ width: 100%; }
#file_list	{ width: 250px; border: 1px solid grey; vertical-align: top; padding: 25 0; }

#_php_box	{ width: 100%; height: 400px; }
#_form_box	{ width: 100%; height: 150px; }

#custom_bits_label	{ font-weight: bold; }

#sandbox_table	{ width: 70%; height: 250px; border: 2px dashed pink; border-collapse: collapse; margin: 0; padding: 0; }
#sandbox_table td	{ border-collapse: collapse; margin: 0; padding: 0; min-height: 50px; }
#sandbox	{ border-top: 1px dashed pink; }
</style>
</head>
<body onload="ChangeOptionDays(document.forms['date_form'], 'Kick_');ChangeOptionDays(document.forms['date_form'], 'Comp_');">
<form id="date_form" action="" method="POST">
<b>KickStart Date &amp; Time:</b>
<br />
	<?php
	list($hour, $minute, $day, $month, $year) = split("[./-]",date("G.i.j.n.Y", strtotime($Kick_DATE)));

	$debug_info .= "Kick:	\$hour=$hour,	\$minute=$minute,	\$day=$day,	\$month=$month,	\$year=$year\n<br />\n";

	genTimeSelector("Kick_", $hour, $minute);
	genDateSelector("Kick_", $day, $month, $year);

	echo "\n<br/><br />\n<b>Competition Date:</b>\n<br />\n";

	list($day, $month, $year) = split("[./-]",date("j.n.Y", strtotime($Comp_DATE)));

	$debug_info .= "Comp:	\$day=$day,	\$month=$month,	\$year=$year\n<br />\n";

	genDateSelector("Comp_", $day, $month, $year);

	echo "\n<br/><br />\n<b>Easter Date:</b>\n<br />\n";

	list($day, $month, $year) = split("[./-]",date("j.n.Y", strtotime($Easter_DATE)));

	$debug_info .= "Easter:	\$day=$day,	\$month=$month,	\$year=$year\n<br />\n";

	genDateSelector("Easter_", $day, $month, $year);
	?>
<br /><br />
<input type="submit" name="_saving" value="Save the date" />
</form>
<? } else {//end if _logged_in ?>
<html>
<head>
<title>Date Editing Page</title>
</head>
<body>
<p>Please login to access the PHP Testing Page.</p>
<form action="" method="post">
	<label for="_username">Username:</label>
	<input type="text" name="_username" id="_username" />
	<br />
	<label for="_pass">Password:</label>
	<input type="password" name="_pass" id="_pass" />
	<br />
	<input type="submit" value="Submit" />
</form>
<?php }
if($debug)
	echo $debug_info;
?>
</body>
</html>