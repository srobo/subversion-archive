<?php

function robogrid_preprocess_forum_submitted(&$variables) {
	if(isset($variables['topic']->timestamp)) {
		if(isset($variables['topic']->forum_tid))	//created date
			 $variables['time'] = isset($variables['topic']->timestamp) ? date('d M H:i',$variables['topic']->timestamp) : '';
		else	//reply date
			$variables['time'] = format_interval(time() - $variables['topic']->timestamp);
	} else
		$variables['time'] = '';
}


/**
 * Preprocess variables to format the topic listing.
 *
 * $variables contains the following data:
 * - $tid
 * - $topics
 * - $sortby
 * - $forum_per_page
 *
 * @see forum-topic-list.tpl.php
 * @see theme_forum_topic_list()
 */
function robogrid_preprocess_forum_topic_list(&$variables) {
	global $forum_topic_list_header;

	// Create the tablesorting header.
	$ts = tablesort_init($forum_topic_list_header);
	$header = array();
//	echo "forum_topic_list_header: ".print_r($forum_topic_list_header, true)."\nts: ".print_r($ts, true);
	foreach ($forum_topic_list_header as $cell) {
		$cell_html = tablesort_header($cell, $forum_topic_list_header, $ts);

//		echo "\ncell: ".print_r($cell, true);
		$header[$cell['data']]	= _theme_table_cell($cell_html, TRUE);
	}
//	echo "\nheader: ".print_r($header, true);

	$variables['header'] = $header[''].$header['Topic'].$header['Last reply'].$header['Replies'];

}
?>