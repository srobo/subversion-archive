<?php
// $Id: forum-list.tpl.php,v 1.4 2007/08/30 18:58:12 goba Exp $

/**
 * @file forum-list.tpl.php
 * Default theme implementation to display a list of forums and containers.
 *
 * Available variables:
 * - $forums: An array of forums and containers to display. It is keyed to the
 *   numeric id's of all child forums and containers.
 * - $forum_id: Forum id for the current forum. Parent to all items within
 *   the $forums array.
 *
 * Each $forum in $forums contains:
 * - $forum->is_container: Is TRUE if the forum can contain other forums. Is
 *   FALSE if the forum can contain only topics.
 * - $forum->depth: How deep the forum is in the current hierarchy.
 * - $forum->zebra: 'even' or 'odd' string used for row class.
 * - $forum->name: The name of the forum.
 * - $forum->link: The URL to link to this forum.
 * - $forum->description: The description of this forum.
 * - $forum->new_topics: True if the forum contains unread posts.
 * - $forum->new_url: A URL to the forum's unread posts.
 * - $forum->new_text: Text for the above URL which tells how many new posts.
 * - $forum->old_topics: A count of posts that have already been read.
 * - $forum->num_posts: The total number of posts in the forum.
 * - $forum->last_reply: Text representing the last time a forum was posted or
 *   commented in.
 *
 * @see template_preprocess_forum_list()
 * @see theme_forum_list()
 */
?>
<table id="forum-<?php print $forum_id; ?>" class="forum-outer">

  <tbody>
  <?php foreach ($forums as $child_id => $forum): ?>
  	<?php if($forum->is_container) print '<tr><td colspan="3" class="container_spacer"></td></tr>'; ?>
    <tr id="forum-list-<?php print $child_id; ?>" class="<?php print $forum->zebra; ?>">
		
      <td <?php print $forum->is_container ? 'colspan="3" class="container"' : 'class="forum"'; ?>>
        <?php /* Enclose the contents of this cell with X divs, where X is the
               * depth this forum resides at. This will allow us to use CSS
               * left-margin for indenting.
               */ ?>
        <?php print str_repeat('<div class="indent">', $forum->depth); ?>
          <div class="name"><?php if($forum->is_container) print '<span class="raquo">&raquo;</span>&nbsp;&nbsp;'; ?><a href="<?php print $forum->link; ?>"><?php print $forum->name; ?></a></div>
          <?php if ($forum->description): ?>
            <div class="description"><?php print $forum->description; ?></div>
          <?php endif; ?>
        <?php print str_repeat('</div>', $forum->depth); ?>
      </td>
	  
      <?php if (!$forum->is_container): ?>
        
        <td class="last-reply"><?php print $forum->last_reply ?></td>
		<td class="topics">
          <?php print $forum->num_topics ?>
          <?php if ($forum->new_topics): ?>
            <br />
            <a href="<?php print $forum->new_url; ?>"><?php print $forum->new_text; ?></a>
          <?php endif; ?>
        </td>
      <?php endif; ?>
    </tr>
	<?php if ($forum->is_container): ?>
	  
	  	<tr class="table_headers"  >
      		<th><?php print t('Forum'); ?></th>
     		 <th><?php print t('Last post');?></th>
      		<th class="topics_header"><?php print t('Topics');?></th>
    	</tr>
		
	  <?php endif; ?>
  <?php endforeach; ?>
  </tbody>
</table>
