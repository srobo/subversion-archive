<?php
/* Convert wiki text to html for output */
function wiki2html($wikitext)
{
	if(!isset($wikitext) || $wikitext == "")
		return FALSE;

	$inter_text	= $wikitext;
	while(strpos($inter_text,"[[") && strpos($inter_text,"]]"))
	{
		$link	= str_replace(array("[[", "]]"), "", substr($inter_text, strpos($inter_text, "[["), (strpos($inter_text, "]]")-strpos($inter_text, "[["))));
		if(strpos($link, "|"))
			list($href, $title)	= explode("|", $link);
		else
			$href	= $title	= $link;
		$inter_text	= str_replace("[[$link]]", "<a href=\"$href\">$title</a>", $inter_text);
	}

	if(strpos($inter_text,"\n*") || strpos($inter_text,"*") === 0)
		$inter_text	= "\n<ul>".str_replace("*", "\n	<li>", str_replace("\n*", "</li>\n	<li>", $inter_text))."</li>\n</ul>\n";

	return $inter_text;
}

/* find if the user is a mentor */
function user_is_mentor($username)
{
	global $mentor_list;
	return in_array($username, $mentor_list);
}

/* make a MySQL query from a search type */
function make_MySQL_query($search)
{
	global $debug_info, $team_number;
	$where	= "WHERE ";
	switch($search)	//do the WHERE's
	{
		case "unfinished":
			$where	.= "completed=0";
			break;
		case "unchecked":
			$where	.= " && signoff_date=0";
		case "finished":
			$where	.= "completed>0$where";
			break;
		case "new":
			$where	.= "new";
			break;
		case "reminders":
			$where	.= "reminders";
			break;
		case "unread":
			$where	.= "unread";
			break;
		case "all":
		default:
			$where	= "";
			break;
	}
	$order	= "ORDER BY ";
	switch($search)	//do the ORDER BY's & tables
	{
		case "all":
		case "unfinished":
		case "finished":
		case "unchecked":
			$order	.= "deadline";
			$table	= "task_list, tracker";
			$where	.= ($where == "" ? "WHERE" : " &&")." task_list_id = task_id && team_id=$team_number";
			break;
		case "new":
			$order	.= "new";
			$table	= $MySQL_news_table;
			break;
		case "reminders":
			$order	.= "reminders";
			$table	= $MySQL_news_table;
			break;
		case "unread":
			$order	.= "unread";
			$table	= $MySQL_news_table;
			break;
		default:
			return;
	}
	$query	= "SELECT * FROM $table $where $order LIMIT 20";
	$debug_info	.= "\n<br />\$query = '$query'\n<br />";
	return $query;
}

/* This function prints the success item on the admin page */
function print_success($success)
{
	global $page_n;

	$out	= "\n			<span class=\"f_right\" id=\"success\">Your ";

	if($success != 1)
		$suc	.= " <span style=\"color: blue;\">not</span>";

	if($page_n == "Admin")
		$out	.= "changes were$suc saved";
	else
		$out	.= "email was$suc sent";

	$out	.= " successfully.</span>";
	return $out;
}

/* This function generates a Time selector */
function genTimeSelector($end)
{
	global $debug_info;
	echo "\n				<select name=\"${end}hour\">";

	for($i = 0; $i <= 23; $i++)
		echo "\n					<option value=\"$i\"" . ($i == 19 ? " selected=\"selected\"" : "") . ">$i</option>";

	echo "\n				</select>\n				:\n				<select name=\"${end}minute\" >";

	for($i = 0; $i <= 56; $i++)
	{
		if($i % 5 == 0)
		{
				$debug_info .= "\$i=$i\n<br />\n";

			echo "\n					<option value=\"$i\">".($i<10?"0":"")."$i</option>";
		}
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

	$min = $year;
	$max = $year + 1;

	for($i = $min; $i <= $max; $i++)
		print "\n					<option value=\"$i\"" . ($i == $year ? " selected=\"selected\"" : "") . ">$i</option>";

	echo "\n				</select>\n";
}


?>