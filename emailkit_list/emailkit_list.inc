<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_destination_form() for the emailkit_list destination.
 */
function emailkit_list_destination_form($destination) {
  $form = array();

  // Get an array of e-mail lists
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
  $lists = EmailkitList::loadAll();  
  
  // Build an array of options
  $options = array();
  foreach ($lists as $lid => $list) {
    $options[$lid] = $list->getName();
  }

  $form['lists'] = array(
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
 * Implementation of hook_destination_form_submit() for the emailkit_list destination.
 */
function emailkit_list_destination_form_submit($destination, $form_values) {
  $options = array();
  
  $options['lists'] = array_keys(array_filter($form_values['lists']));
  
  return $options;
}

/**
 * Implementation of hook_dispatcher_send() for the emailkit_list dispatcher.
 */
function emailkit_list_dispatcher_send($dispatcher, $message, $destination, $destination_options) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
  
  // Translate the emailkit_list options to more generic emailkit_default options
  $options = array();
  
  $recipients = array();
  foreach ($destination_options['lists'] as $lid) {
    $list = EmailkitList::loadWithId($lid);
    
    // TODO: Add actual subscribers
    $recipients[] = array(
      'address' => 'emailkit_list_' . $lid . '@example.com',
    );
  }
  
  $options['recipients'] = $recipients;

  // Resend the message
  emailkit_send($message, 'emailkit_default', $options);
}
