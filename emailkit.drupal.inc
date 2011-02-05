<?php
// Developed by Dennis Stevense for Digital Deployment

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
  // TODO: Process custom headers in $drupal_message
  return array(
    '#subject' => $drupal_message['subject'],
    'content' => array(
      'body' => array(
        // TODO: Less primitive way of converting Drupal's plain text messages to HTML
        '#value' => '<pre>' . $drupal_message['body'] . '</pre>',
      ),
    ),
  );
}
