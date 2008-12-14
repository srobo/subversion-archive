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
 *
function robogrid_preprocess_forum_topic_list(&$variables) {
	global $forum_topic_list_header;

	// Create the tablesorting header.
	$ts = tablesort_init($forum_topic_list_header);
	$header = '';
	foreach ($forum_topic_list_header as $cell) {
		$cell = tablesort_header($cell, $forum_topic_list_header, $ts);

//		$header .= '<th>'.$cell['data'].'</th>';
//		foreach($cell as $headings) {
//			$header .= '<th>'.$headings['data'].'</th>';
//		}
//		print_r($forum_topic_list_header);
//		print_r($ts);
//		echo 'Cell: '.$cell.' header:'.$forum_topic_list_header.' ts: '.$ts;
		$header .= _theme_table_cell($cell, TRUE);
	}

	$variables['header'] = $header;

/*	if (!empty($variables['topics'])) {
		$row = 0;
		foreach ($variables['topics'] as $id => $topic) {
			$variables['topics'][$id]->icon = theme('forum_icon', $topic->new, $topic->num_comments, $topic->comment_mode, $topic->sticky);
			$variables['topics'][$id]->zebra = $row % 2 == 0 ? 'odd' : 'even';
			$row++;

			// We keep the actual tid in forum table, if it's different from the
			// current tid then it means the topic appears in two forums, one of
			// them is a shadow copy.
			if ($topic->forum_tid != $variables['topic_id']) {
				$variables['topics'][$id]->moved = TRUE;
				$variables['topics'][$id]->title = check_plain($topic->title);
				$variables['topics'][$id]->message = l(t('This topic has been moved'), "forum/$topic->forum_tid");
			} else {
				$variables['topics'][$id]->moved = FALSE;
				$variables['topics'][$id]->title = l($topic->title, "node/$topic->nid");
				$variables['topics'][$id]->message = '';
			}
			echo $variables['topics'][$id]->created;// = theme('forum_submitted_starter', $topic); // Added by probinson 20/11/08

			$variables['topics'][$id]->last_reply = theme('forum_submitted', isset($topic->last_reply) ? $topic->last_reply : NULL);

			$variables['topics'][$id]->new_text = '';
			$variables['topics'][$id]->new_url = '';
			if ($topic->new_replies) {
				$variables['topics'][$id]->new_text = format_plural($topic->new_replies, '1 new', '@count new');
				$variables['topics'][$id]->new_url = url("node/$topic->nid", array('query' => comment_new_page_count($topic->num_comments, $topic->new_replies, $topic), 'fragment' => 'new'));
			}
		}
	} else {
		// Make this safe for the template
		$variables['topics'] = array();
	}
//*/
//	$variables['pager'] = theme('pager', NULL, $variables['forum_per_page'], 0);
//}
?>