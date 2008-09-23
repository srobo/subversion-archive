<?php
// $Id: libdebug.php,v 1.2.4.2 2008/07/19 23:14:10 miglius Exp $

/**
 * @file
 * ldapauth module debug options.
 */

/**
 * Prints debug string.
 */
function msg($string) {
  drupal_set_message("<pre style=\"border: 0; margin: 0; padding: 0;\">$string</pre>");
}

/**
 * Prints debug object.
 */
function msg_r($object) {
  msg(print_r($object, TRUE));
}

