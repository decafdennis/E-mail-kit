<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_dispatcher_send() for the emailkit_default dispatcher.
 */
function emailkit_default_dispatcher_send($dispatcher, $message, $destination) {
  $mail = array(
    'id' => 'emailkit',
    'from' => variable_get('site_mail', ini_get('sendmail_from')),
    'subject' => $message['#subject'],
    'body' => drupal_wrap_mail(drupal_render($message['#body'])),
    'headers' => array(
      'X-Mailer' => 'E-mail kit (Drupal)',
    ),
  );
  
  $to_array = array();
  foreach ($destination['#recipients'] as $recipient) {
    $to_array[] = $recipient['address'];
  }
  $mail['to'] = implode(", ", $to_array);
  
  return drupal_mail_send($mail);
}
