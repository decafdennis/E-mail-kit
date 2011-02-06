<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains callbacks and related functions for user pages.
 */

/**
 * Returns the page for the E-mail tab on a node.
 */
function emailkit_node_email($node) {
  return drupal_get_form('emailkit_node_email_form', $node);
}

/**
 * Returns the form for the E-mail tab on a node.
 */
function emailkit_node_email_form(&$form_state, $node) {
  $form = array(
    '#node' => $node,
  );

  // Note: we can't call this field 'destination', because it collides with Drupal's drupal_get_destination() stuff
  $form['dest'] = array(
    '#type' => 'emailkit_destination_select',
    '#title' => t('Destination'),
    '#required' => TRUE,
    '#description' => t('Specify the destination to which to send the e-mail.'),
  );
  
  $form['send'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );
  
  return $form;
}

/**
 * Handles submission of the form for the E-mail tab on a node.
 */
function emailkit_node_email_form_submit($form, &$form_state) {
  // Build the message
  $message = emailkit_message('emailkit_node', $form['#node']);

  // Send the message
  if (emailkit_destination_select_send($message, $form_state['values']['dest'])) {
    drupal_set_message(t('E-mail was sent successfully.'));
  }
  else {
    drupal_set_message(t('Failed to send e-mail.'), 'error');
  }
}
