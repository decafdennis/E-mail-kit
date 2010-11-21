<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the destinations defined by this module.
 *
 * This is a module-level hook.
 *
 * @return An array of destinations keyed by name.
 */
function hook_emailkit_destinations() {
  $destinations = array();
  
  $destinations['my_module_mailbox'] = array(
    // The human-readable name of the destination (required)
    'name' => t('Mailbox'),
    // The base for destination hooks (optional, defaults to the destination name)
    'base' => 'my_module_mailbox',
    // The file that needs to be included when invoking destination hooks (optional)
    'file' => 'my_module.mailbox.inc',
  );
  
  return $destinations;
}

/**
 * Returns the form for destination options.
 *
 * This is a destination-level hook. Implementation of this hook is optional.
 *
 * @param $destination A string indicating the name of the destination.
 *
 * @return A form array.
 */
function hook_destination_form($destination) {
  $form = array();

  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Mailbox number'),
    '#required' => TRUE,
  );
  
  return $form;
}

/**
 * Validates the form for destination options.
 *
 * This is a destination-level hook. Implementation of this hook is optional. This hook will not be called if #element_validate was already set in hook_destination_form().
 *
 * @param &$form A form array. The name of the destination can be found in $form['#destination'], unless this value was already set in hook_destination_form().
 */
function hook_destination_form_validate(&$form) {
  if (!empty($form['address']['#value']) && !is_numeric($form['address']['#value'])) {
    form_error($form['address'], t('Mailbox address must be a number.'));
  }
}

/**
 * Dispatches a message based on the given form values. You only need to implement this hook if the form values need to be processed before they are being passed to hook_destination_dispatch(). At the end of this method, you should call hook_destination_dispatch().
 *
 * This is a destination-level hook. Implementation of this hook is optional. If this hook is not implemented, hook_destination_dispatch() will be called where options is equal to the form values.
 *
 * @param $destination A string indicating the name of the destination.
 * @param $form_values The values from the form for destination options.
 *
 * @return A flag indicating whether dispatch was successful.
 */
function hook_destination_form_dispatch($destination, $form_values) {
  // TODO: Process form values into options
  $options = $form_values;
  
  return my_module_mailbox_destination_dispatch($destination, $options);
}

/**
 * Dispatches a message based on the given options.
 *
 * This is a destination-level hook. Implementation of this hook is required.
 *
 * @param $destination A string indicating the name of the destination.
 * @param $options The options.
 *
 * @return A flag indicating whether dispatch was successful.
 */
function hook_destination_dispatch($destination, $options) {
  // TODO: Send message to destination
  return FALSE;
}
