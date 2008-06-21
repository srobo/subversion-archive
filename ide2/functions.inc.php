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
	if($search != "all")
	{
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
			default:
				$where	= "";
				return;
		}
	}
	$order	= "ORDER BY ";
	switch($search)	//do the ORDER BY's & tables
	{
		case "all":
		case "unfinished":
		case "finished":
		case "unchecked":
			$order	.= "deadline";
			$table	= "task_list";
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
			$order	= "";
			break;
	}
	return "SELECT * FROM $table $where $order LIMIT 20";
}
?>