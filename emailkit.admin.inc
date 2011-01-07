<?php
// Developed by Dennis Stevense for Digital Deployment

function emailkit_admin_settings() {
  return drupal_get_form('emailkit_admin_settings_form');
}

function emailkit_admin_settings_form() {
  $form = array();
  
  $form['destination_dispatcher'] = array(
    '#type' => 'fieldset',
    '#title' => t('Dispatchers'),
    '#description' => t('Choose the dispatcher to use for each kind of destination. If a destination type can be serviced by more than one dispatcher, the fields below determine which dispatcher is used.'),
  );
  
  $destinations = emailkit_destination_info();
  $dispatchers = emailkit_dispatcher_info();

  foreach ($destinations as $destination_type => $destination_info) {
    $options = array();
    
    foreach ($dispatchers as $dispatcher_name => $dispatcher_info) {
      if (in_array($destination_type, $dispatcher_info['#destinations'])) {
        $options[$dispatcher_name] = $dispatcher_info['#label'];
      }
    }
    
    $key = 'emailkit_destination_dispatcher_' . $destination_type;

    $form['destination_dispatcher'][$key] = array(
      '#type' => 'radios',
      '#title' => $destination_info['#label'],
      '#options' => $options,
      '#default_value' => emailkit_destination_dispatcher($destination_type),
    );
  }
  
  return system_settings_form($form);
}
