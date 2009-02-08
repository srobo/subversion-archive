<?php
// $Id: forum_access.node.inc,v 1.1 2009/01/26 23:38:40 salvis Exp $

/**
 * @file forum_access.node.inc
 *
 * Include file for forum_access.module, containing (sub-)page handling
 * (form_alter) code for the node form.
 *
 */

/**
 * Rewrite the taxonomy item on the node form.
 */
function _forum_access_node_form($form_id, &$form) {
  $vid = variable_get('forum_nav_vocabulary', '');

  if (!isset($form['taxonomy'][$vid]['#options'])) {
    return;
  }

  // forum administrators do NOT get their forms rewritten here.
  if (user_access('administer forums')) {
    return;
  }
  global $user;
  $roles = _forum_access_get_roles($user);
  $result = db_query("SELECT tid FROM {forum_access} WHERE rid IN (%s) AND grant_create = 1", $roles);
  while ($obj = db_fetch_object($result)) {
    $tids[$obj->tid] = $obj->tid;
  }

  // Also get all forums they happen to be able to moderate.

  $result = db_query("SELECT a.name AS tid FROM {acl} a INNER JOIN {acl_user} u ON a.acl_id = u.acl_id WHERE a.module = 'forum_access' AND u.uid = %d", $user->uid);
  while ($obj = db_fetch_object($result)) {
    $tids[$obj->tid] = $obj->tid;
  }

  // Ensure the forum they're trying to post to directly is allowed, otherwise
  // there will be much confusion.
  $forum_tid = arg(3);
  if (is_numeric($forum_tid) && $forum_tid && !$tids[$forum_tid]) {
    drupal_access_denied();
    module_invoke_all('exit');
    exit;
  }

  foreach ($form['taxonomy'][$vid]['#options'] as $tid => $name) {
    if (!is_numeric($tid)) {
      $options[$tid] = $name;
    }
    elseif (is_object($name)) {
      foreach ($name->option as $sub_tid => $sub_name) {
        if (!empty($tids[$sub_tid])) {
          $options[$tid]->option[$sub_tid] = $sub_name;
        }
      }
    }
    elseif ($tids[$tid]) {
      $options[$tid] = $name;
    }
  }

  if ($options) {
    $form['taxonomy'][$vid]['#options'] = $options;
  }
  else {
    unset($form['taxonomy'][$vid]);
  }
}

