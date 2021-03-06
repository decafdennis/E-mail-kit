<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains the code used when emailkit is used as Drupal's mail handler. Specifically, it contains the implementation of drupal_mail_wrapper() and the message hook that builds an emailkit message based on a Drupal message.
 */

/**
 * Implementation of drupal_mail_wrapper(). See drupal_mail_send().
 */
function drupal_mail_wrapper($drupal_message) {
  $message = emailkit_message('emailkit_drupal', $drupal_message);

  $destination = array(
    '#type' => 'emailkit_imf',
    '#to' => $drupal_message['to'],
  );

  return emailkit_send($message, $destination);
}

/**
 * Implementation of hook_message() for the emailkit_drupal message.
 */
function emailkit_drupal_message($message_id, $drupal_message) {
  $body = $drupal_message['body'];
  $content_type = isset($drupal_message['headers']['Content-Type']) ? $drupal_message['headers']['Content-Type'] : 'text/plain';

  // If the body is plain text, convert it to HTML
  if (strpos($content_type, 'text/plain') !== FALSE) {
    // Apply the line break filter
    $body = _filter_autop($body);
    
    // Apply the URL filter, without trimming the URLs so they can still be copy/pasted if the text version asks to do this
    variable_set('filter_url_length_emailkit_drupal', 9999);
    $body = _filter_url($body, 'emailkit_drupal');
  }
  
  // TODO: Process custom headers in $drupal_message
  $message = array(
    '#subject' => $drupal_message['subject'],
    'content' => array(
      'body' => array(
        '#value' => $body,
      ),
    ),
  );

  // Keep the Drupal message identifier around
  if (isset($drupal_message['id'])) {
    $message['#drupal_id'] = $drupal_message['id'];
  }

  return $message;
}
