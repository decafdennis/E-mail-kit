<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_destination_form() for the emailkit_address destination type.
 */
function emailkit_address_destination_form($destination_type) {
  $form = array();
  
  // Allow the user to enter an e-mail address
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#description' => t('The address to which to send the e-mail. Example: john@example.com'),
    '#required' => TRUE,
  );
  
  return $form;
}

/**
 * Implementation of hook_destination_form_validate() for the emailkit_address destination type.
 */
function emailkit_address_destination_form_validate(&$form) {
  // If the address field has been filled in, check whether it is a valid e-mail address
  if (!empty($form['address']['#value']) && !valid_email_address($form['address']['#value'])) {
    form_error($form['address'], t('E-mail address is not a valid e-mail address.'));
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
  // Translate the emailkit_address destination to the more generic emailkit_default destination
  $destination = array(
    '#type' => 'emailkit_default',
    '#recipients' => array(
      array(
        'address' => $destination['#address'],
      ),
    ),
  );

  // Resend the message
  return emailkit_send($message, $destination);
}
