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
  global $user;
  
  $form = array(
    '#node' => $node,
  );

  // Note: we can't call this field 'destination', because it collides with Drupal's drupal_get_destination() stuff
  $form['dest'] = array(
    '#type' => 'emailkit_destination_select',
    '#title' => t('To'),
    '#required' => TRUE,
    '#description' => t('The destination to which to send the e-mail.'),
  );
  
  // Allows the user to override the default sender name, which is the user's name
  $form['from_name'] = array(
    '#type' => 'textfield',
    '#title' => t('From name'),
    '#description' => t('The name to use for the sender of the e-mail message.'),
    '#default_value' => $user->name,
  );
  
  // Allows the user to override the default sender e-mail address, which is the user's e-mail address
  $form['from_address'] = array(
    '#type' => 'textfield',
    '#title' => t('From e-mail address'),
    '#description' => t('The e-mail address to use for the sender of the e-mail message.'),
    '#required' => TRUE,
    '#default_value' => $user->mail,
  );
  
  // Allows the user to override the default message subject, which is the node title
  $form['subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Subject'),
    '#description' => t('The subject of the e-mail message.'),
    '#required' => TRUE,
    '#default_value' => $node->title,
  );
  
  $form['send'] = array(
    '#type' => 'submit',
    '#value' => t('Send message'),
  );
  
  return $form;
}

/**
 * Handles validation of the form for the send message tab on a node.
 */
function emailkit_node_send_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['from_address']) && !valid_email_address($form_state['values']['from_address'])) {
    form_error($form['from_address'], t('From e-mail address field does not contain a valid e-mail address.'));
  }
}

/**
 * Handles submission of the form for the send message tab on a node.
 */
function emailkit_node_send_form_submit($form, &$form_state) {
  // Build the message
  $message = emailkit_message('emailkit_node', $form['#node']);
  $message['#from']['name'] = $form_state['values']['from_name'];
  $message['#from']['address'] = $form_state['values']['from_address'];
  $message['#subject'] = $form_state['values']['subject'];
  
  // Send the message
  if (emailkit_destination_select_send($message, $form_state['values']['dest'])) {
    drupal_set_message(t('The message was sent successfully.'));
  }
  else {
    drupal_set_message(t('The message could not be sent.'), 'error');
  }
}

/**
 * Returns the page for the preview message tab on a node.
 */
function emailkit_node_preview($node) {
  $output = '<dl>';

  $options = array();
  $options['attributes'] = array();

  // Show in a lightbox, if the lightbox2 module is available
  if (module_exists('lightbox2')) {
    $options['attributes']['rel'] = 'lightframe[emailkit_node_preview]';
  }

  $output .= '<dt>'. l(t('Preview HTML version'), 'node/' . $node->nid . '/message/preview/html', $options) .'</dt>';
  $output .= '<dd>'. t('Most e-mail readers are able to display the HTML version of the message.') .'</dd>';

  $output .= '<dt>'. l(t('Preview text version'), 'node/' . $node->nid . '/message/preview/text', $options) .'</dt>';
  $output .= '<dd>'. t('E-mail readers that are not able to display the HTML version, or recipients who do not wish to receive HTML messages, will see this textual version of the message.') .'</dd>';

  $output .= '</dl>';
  return $output;
}

/**
 * Previews the message for a node in the given format.
 */
function emailkit_node_preview_format($node, $format) {
  if (!in_array($format, array('text', 'html'))) {
    return drupal_not_found();
  }

  // Build the message
  $message = emailkit_message('emailkit_node', $node, TRUE);

  switch ($format) {
    case 'html':
      // Render the message
      $html = emailkit_message_render($message, EMAILKIT_MESSAGE_FORMAT_HTML);

      // Display the message, suppressing any devel output
      drupal_set_header('Content-Type: text/html; charset=utf-8');
      $GLOBALS['devel_shutdown'] = FALSE;
      print($html);
      break;

    case 'text':
      // Render the message
      $text = emailkit_message_render($message, EMAILKIT_MESSAGE_FORMAT_TEXT);

      // Display the message, suppressing any devel output
      drupal_set_header('Content-Type: text/plain; charset=utf-8; format=flowed; delsp=yes');
      $GLOBALS['devel_shutdown'] = FALSE;
      print($text);
      break;
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
    $result = db_query("SELECT n.token, n.uid, n.sent, s.token IS NOT NULL AS statistics FROM {emailkit_node_message} n LEFT JOIN {emailkit_statistics_event} s ON n.token = s.token WHERE n.nid = %d GROUP BY n.token ORDER BY n.sent DESC", $node->nid);
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
