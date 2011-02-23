<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains callbacks and related functions for user pages.
 */

/**
 * Returns the page for the send message tab on a node.
 */
function emailkit_node_send($node) {
  return drupal_get_form('emailkit_node_send_form', $node);
}

/**
 * Returns the form for the send message tab on a node.
 */
function emailkit_node_send_form(&$form_state, $node) {
  $form = array(
    '#node' => $node,
  );

  // Note: we can't call this field 'destination', because it collides with Drupal's drupal_get_destination() stuff
  $form['dest'] = array(
    '#type' => 'emailkit_destination_select',
    '#title' => t('Destination'),
    '#required' => TRUE,
    '#description' => t('Specify the destination to which to send the e-mail.'),
  );
  
  $form['send'] = array(
    '#type' => 'submit',
    '#value' => t('Send message'),
  );
  
  return $form;
}

/**
 * Handles submission of the form for the send message tab on a node.
 */
function emailkit_node_send_form_submit($form, &$form_state) {
  // Build the message
  $message = emailkit_message('emailkit_node', $form['#node']);

  // Send the message
  if (emailkit_destination_select_send($message, $form_state['values']['dest'])) {
    drupal_set_message(t('E-mail was sent successfully.'));
  }
  else {
    drupal_set_message(t('Failed to send e-mail.'), 'error');
  }
}

/**
 * Returns the page for the sent messages tab on a node.
 */
function emailkit_node_sent($node) {
  // Get all messages sent for this node
  $messages = array();
  
  // If the statistics module is enabled, also fetch a flag indicating whether statistics are available for a message
  if (module_exists('emailkit_statistics')) {
    $result = db_query("SELECT n.token, n.uid, n.sent, s.token IS NOT NULL AS statistics FROM {emailkit_node_message} n LEFT JOIN {emailkit_statistics_message} s ON n.token = s.token WHERE n.nid = %d ORDER BY n.sent DESC", $node->nid);
  }
  else {
    $result = db_query("SELECT token, uid, sent, 0 AS statistics FROM {emailkit_node_message} WHERE nid = %d ORDER BY sent DESC", $node->nid);
  }

  while ($record = db_fetch_object($result)) {
    $message = $record;
    $message->user = user_load($message->uid);
    $message->statistics_path = 'node/' . $node->nid . '/message/sent/' . $message->token . '/statistics';
    
    $messages[] = $record;
  }

  return theme('emailkit_node_sent', $messages);
}

/**
 * Returns the statistics page for a particular message.
 */
function emailkit_node_sent_message_statistics($node, $token) {
  // Set the breadcrumb
  $breadcrumb = array();
  $breadcrumb[] = l(t('Home'), '<front>');
  $breadcrumb[] = l($node->title, 'node/' . $node->nid);
  $breadcrumb[] = l(t('E-mail'), 'node/' . $node->nid . '/message');
  $breadcrumb[] = l(t('Sent messages'), 'node/' . $node->nid . '/message/sent');
  $breadcrumb[] = l(drupal_get_title(), 'node/' . $node->nid . '/message/sent/' . $token . '/statistics');
  drupal_set_breadcrumb($breadcrumb);
  
  // Ask the statistics module to render the page
  $output = emailkit_statistics_message_page($token);
  
  if (isset($output)) {
    return $output;
  }
  else {
    return drupal_not_found();
  }
}

/**
 * Renders the page for the sent messages tab on a node.
 */
function theme_emailkit_node_sent($messages) {
  $output = "";

  $header = array(t('Date sent'), t('Sender'), t('Statistics'));
  $rows = array();
  foreach ($messages as $message) {
    // Add a row for the message
    $rows[] = array(
      format_date($message->sent),
      theme('username', $message->user),
      $message->statistics ? l(t('statistics'), $message->statistics_path) : t('no statistics available'),
    );
  }

  if (empty($rows)) {
    $rows[] = array(
      array(
        'data' => t('No messages have been sent with this content.'),
        'colspan' => _emailkit_table_colspan($header),
      ),
    );
  }

  $output .= theme('table', $header, $rows);

  return $output;
}
