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
 * Implementation of hook_destination_form_dispatch() for the emailkit_list destination.
 */
function emailkit_list_destination_form_dispatch($destination, $form_values) {
  $options = array();
  $options['lists'] = array_keys(array_filter($form_values['lists']));
  
  return emailkit_list_destination_dispatch($destination, $options);
}

/**
 * Implementation of hook_destination_dispatch() for the emailkit_list destination.
 */
function emailkit_list_destination_dispatch($destination, $options) {
  dsm($options);
  
  return TRUE;
}
