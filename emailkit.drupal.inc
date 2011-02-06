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
    
    // Apply the URL filter
    $body = _filter_url($body, '__emailkit_foo');
  }
  
  // TODO: Process custom headers in $drupal_message
  return array(
    '#subject' => $drupal_message['subject'],
    'content' => array(
      'body' => array(
        '#value' => $body,
      ),
    ),
  );
}
