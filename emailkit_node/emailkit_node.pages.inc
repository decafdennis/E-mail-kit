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
  $result = db_query("SELECT tracking_id, uid, sent FROM {emailkit_node_message} WHERE nid = %d ORDER BY sent DESC", $node->nid);
  while ($record = db_fetch_object($result)) {
    $message = $record;
    $message->user = user_load($message->uid);
    
    $messages[] = $record;
  }

  return theme('emailkit_node_sent', $messages);
}

/**
 * Renders the page for the sent messages tab on a node.
 */
function theme_emailkit_node_sent($messages) {
  $output = "";

  $header = array(t('Date sent'), t('Sender'));
  $rows = array();
  foreach ($messages as $message) {
    // Add a row for the message
    $rows[] = array(
      format_date($message->sent),
      theme('username', $message->user),
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
