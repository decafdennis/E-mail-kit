<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_destination_form() for the emailkit_address destination.
 */
function emailkit_address_destination_form($destination) {
  $form = array();
  
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
  if (!empty($form['address']['#value']) && !valid_email_address($form['address']['#value'])) {
    form_error($form['address'], t('E-mail address is not a valid e-mail address.'));
  }
}

/**
 * Implementation of hook_destination_dispatch() for the emailkit_address destination.
 */
function emailkit_address_destination_dispatch($destination, $options) {
  dsm($options);
  
  return TRUE;
}
