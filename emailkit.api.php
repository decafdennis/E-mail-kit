<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the destinations defined by this module.
 *
 * Destinations represent an addressing domain. For example:
 * - e-mail addresses,
 * - e-mail lists defined in some external system.
 *
 * Destinations can be exposed to the user. If a destination is not exposed, it can only be used internally. However, if a destination is exposed, then the user can configure it and send a message to it. For example, the e-mail address destination might allow the user to enter an e-mail address and then send a message to that address.
 *
 * This is a module-level hook. Implementation is optional.
 *
 * @return An array of destination information keyed by destination name. Destination information is a subarray with the following attributes:
 *   label: The human-readable name of the destination. Capitalize and wrap in t(). Required.
 *   exposed: Flag indicating whether the destination is exposed to the user for sending. Optional. Defaults to FALSE.
 *   file: The file that needs to be included when invoking destination hooks. Optional.
 *   base: The base for destination hooks. Optional. Defaults to the destination name.
 */
function hook_emailkit_destination_info() {
  $info = array();

  $info['my_module_address'] = array(
    'label' => t('My module address'),
    'exposed' => TRUE,
    'file' => 'my_module.address_destination.inc',
  );

  return $info;
}

/**
 * Returns the form for exposed destinations. This form allows the user to configure the destination.
 *
 * This is a destination-level hook. Implementation is optional.
 *
 * @param $destination The name of the destination.
 *
 * @return A form array.
 */
function hook_destination_form($destination) {
  $form = array();

  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Address ID'),
    '#required' => TRUE,
  );
  
  return $form;
}

/**
 * Validates the form for exposed destinations (@see hook_destination_form()).
 *
 * This is a destination-level hook. Implementation is optional. This hook will not be called if #element_validate was set in hook_destination_form().
 *
 * @param &$form A form array. The name of the destination can be found in $form['#destination'], unless this value was set in hook_destination_form().
 */
function hook_destination_form_validate(&$form) {
  if (!empty($form['address']['#value']) && !is_numeric($form['address']['#value'])) {
    form_error($form['address'], t('Address ID must be a number.'));
  }
}

/**
 * Submits the form for exposed destinations (@see hook_destination_form()). The purpose of this hook is to return an array of options that can be passed to the dispatcher.
 *
 * This is a destination-level hook. Implementation is optional. If this hook is not implemented, the form values will be used as options.
 *
 * @param $destination The name of the destination.
 * @param $form_values The values submitted by the form.
 *
 * @return An array of options.
 */
function hook_destination_form_submit($destination, $form_values) {
  // TODO: Process form values into options
  return $form_values;
}

/**
 * Returns the dispatchers defined by this module.
 *
 * Dispatchers send a message to a destination.
 *
 * For each destination, there must be at least one dispatcher that services it. If there is more than one dispatcher for a given destination, then the user can choose which dispatcher is used.
 *
 * This is a module-level hook. Implementation is optional, but must usually be implemented if hook_emailkit_destination_info() is implemented.
 *
 * @return An array of dispatcher information keyed by dispatcher name. Dispatcher information is a subarray with the following attributes:
 *   label: The human-readable name of the dispatcher. Capitalize and wrap in t(). Required.
 *   destinations: An array of names of supported destinations. Required.
 *   file: The file that needs to be included when invoking dispatcher hooks. Optional.
 *   base: The base for dispatcher hooks. Optional. Defaults to the dispatcher name.
 */
function hook_emailkit_dispatcher_info() {
  $info = array();

  $info['my_module_dispatcher'] = array(
    'label' => t('My module'),
    'destinations' => array('my_module_address'),
    'file' => 'my_module.dispatcher.inc',
  );

  return $info;
}

/**
 * Sends a message to a destination.
 *
 * This is a dispatcher-level hook. Implementation is optional, but must be implemented if the dispatcher wants to do anything useful.
 *
 * @param $message The message to send. TODO: What is this?
 * @param $destination The name of the destination.
 * @param $destination_options An array of options for the destination depending on the destination.
 *
 * @return An array with the following attributes:
 *   success: Flag indicating whether sending was successful. Required.
 *   error: Human-readable error message if sending was not successful. Capitalize and wrap in t(). Optional.
 */
function hook_dispatcher_send($message, $destination, $destination_options) {
  return array(
    'success' => FALSE,
    'error' => t('Not implemented yet.'),
  );
}
