<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains implementations for the hooks of the emailkit_address destination and dispatcher.
 */

/**
 * Implementation of hook_destination_form() for the emailkit_address destination type.
 */
function emailkit_address_destination_form($destination_type) {
  global $user;
  
  $form = array();
  
  // Allow the user to enter an e-mail address
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#description' => t('The address to which to send the e-mail. You may enter multiple addresses by separating them by a comma. Example: john@example.com'),
    '#required' => TRUE,
    '#default_value' => $user->mail,
  );
  
  return $form;
}

/**
 * Implementation of hook_destination_form_validate() for the emailkit_address destination type.
 */
function emailkit_address_destination_form_validate(&$form) {
  // If the address field has been filled in, check whether it contains valid e-mail addresses
  if (!empty($form['address']['#value'])) {
    $address_spec = $form['address']['#value'];
    $addresses = preg_split('/\s*,\s*/', $address_spec, NULL, PREG_SPLIT_NO_EMPTY);
    
    if (empty($addresses)) {
      form_error($form['address'], t('No e-mail address has been entered.'));
    }
    else {
      foreach ($addresses as $address) {
        if (!valid_email_address($address)) {
          form_error($form['address'], t('E-mail address is not a valid e-mail address.'));
        }
      }
    }
  }
}

/**
 * Implementation of hook_destination_form_submit() for the emailkit_address destination type.
 */
function emailkit_address_destination_form_submit(&$destination, $form_values) {
  $destination['#address'] = $form_values['address'];
}

/**
 * Implementation of hook_dispatcher_send() for the emailkit_address dispatcher.
 */
function emailkit_address_dispatcher_send($dispatcher, $message, $destination) {
  $address_spec = $destination['#address'];
  
  // Translate the emailkit_address destination to the more generic emailkit_default destination
  $destination = array(
    '#type' => 'emailkit_default',
    '#recipients' => array(),
  );
  
  $addresses = preg_split('/\s*,\s*/', $address_spec);
  
  foreach ($addresses as $address) {
    $destination['#recipients'][] = array(
      'address' => $address,
    );
  }
  
  // Resend the message
  return emailkit_send($message, $destination);
}
