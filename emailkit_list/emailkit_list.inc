<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_destination_form() for the emailkit_list destination type.
 */
function emailkit_list_destination_form($destination_type) {
  $form = array();

  // Build an array of options
  $options = array();
  $lists = emailkit_list_load_all();
  foreach ($lists as $lid => $list) {
    $options[$lid] = $list->name;
  }

  $form['lids'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Lists'),
    '#options' => $options,
    '#default_value' => array(),
    '#description' => t('Select one or more lists to which to send the e-mail. If a subscriber is subscribed to more than one of the selected lists, then he/she will receive just a single e-mail.'),
    '#required' => TRUE,
  );
  
  return $form;
}

/**
 * Implementation of hook_destination_form_submit() for the emailkit_list destination type.
 */
function emailkit_list_destination_form_submit(&$destination, $form_values) {
  $destination['#lids'] = array_keys(array_filter($form_values['lids']));
}

/**
 * Implementation of hook_dispatcher_send() for the emailkit_list dispatcher.
 */
function emailkit_list_dispatcher_send($dispatcher, $message, $destination) {
  $lids = $destination['#lids'];
  
  // Translate the emailkit_list destination to the more generic emailkit_default destination
  $destination = array(
    '#type' => 'emailkit_default',
    '#recipients' => array(),
  );

  foreach ($lids as $lid) {
    // Add all subscribers to the list of recipients
    $subscribers = emailkit_list_load_subscribers($lid);
    foreach ($subscribers as $subscriber) {
      $destination['#recipients'][] = array(
        'address' => $subscriber->address,
      );
    }
  }
  
  // Resend the message
  return emailkit_send($message, $destination);
}
