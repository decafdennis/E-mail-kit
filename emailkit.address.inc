<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_destination_form() for the emailkit_address destination.
 */
function emailkit_address_destination_form($destination) {
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
 * Implementation of hook_destination_form_validate() for the emailkit_address destination.
 */
function emailkit_address_destination_form_validate(&$form) {
  // If the address field has been filled in, check whether it is a valid e-mail address
  if (!empty($form['address']['#value']) && !valid_email_address($form['address']['#value'])) {
    form_error($form['address'], t('E-mail address is not a valid e-mail address.'));
  }
}

/**
 * Implementation of hook_dispatcher_send() for the emailkit_address dispatcher.
 */
function emailkit_address_dispatcher_send($message, $destination, $destination_options) {
  // Translate the emailkit_address options to more generic emailkit_recipients options
  $options = array(
    'recipients' => array(
      array(
        'address' => $destination_option['address'],
      ),
    ),
  );
  
  // Resend the message
  emailkit_send($message, 'emailkit_recipients', $options);
}
