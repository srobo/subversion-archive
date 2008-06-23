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
function make_MySQL_query($search, $select)
{
	global $debug_info, $team_number, $pg;
	$where	= "WHERE ";
	$order	= "ORDER BY ";
	switch($search)	//do the WHERE's
	{
		case "all":
			$where	.= "1";
			$order	.= "deadline";
			$table	= "task_list";
			break;
		case "unfinished":
			$where	.= "task_id != task_list_id && team_id=$team_number";
			$order	.= "deadline";
			$table	= "task_list, tracker";
			break;
		case "unchecked":
			$where	.= "completed>0 && signoff_date=0 && task_id=task_list_id && team_id=$team_number";
			$order	.= "deadline";
			$table	= "task_list, tracker";
			break;
		case "finished":
			$where	.= "completed>0 && task_id=task_list_id && team_id=$team_number";
			$order	.= "deadline";
			$table	= "task_list, tracker";
			break;
		case "new":
			$where	.= "new";
			$order	.= "new";
			$table	= $MySQL_news_table;
			break;
		case "reminders":
			$where	.= "reminders";
			$order	.= "reminders";
			$table	= $MySQL_news_table;
			break;
		case "unread":
			$where	.= "unread";
			$order	.= "unread";
			$table	= $MySQL_news_table;
			break;
		default:
			return;
	}
	
	$low	= 20*$pg;
	$high	= 20+$low;
	
	$query	= "SELECT $select FROM $table $where $order LIMIT $low, $high";
	$debug_info	.= "\n<br />\$query = '$query'\n<br />";
	return $query;
}

/* This function prints the success item on the admin page */
function print_success($success)
{
	global $page_n;

	$out	= "\n			<span class=\"f_right\" id=\"success\">Your ";

	if($success != 1)
		$not	= " <span style=\"color: blue;\">not</span>";
	else
		$not	= "";

	if($page_n == "Admin")
		$out	.= "changes were$not saved";
	else
		$out	.= "email was$not sent";

	$out	.= " successfully.</span>";
	return $out;
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