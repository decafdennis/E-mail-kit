<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the destination types defined by this module.
 *
 * A destination type represents an addressing domain. For example, the domain of:
 * - e-mail addresses, or the domain of
 * - e-mail lists defined in some external system.
 *
 * Destination types can be exposed to the user. If a destination type is not exposed, it can only be used internally. However, if a destination type is exposed, then the user can use it to specify a concrete destination and send a message to it. For example, the e-mail address destination type might allow the user to enter an e-mail address in order to send a message to that address.
 *
 * This is a module-level hook. Implementation is optional.
 *
 * @return An array of destination type information keyed by destination type name. Destination type information is a subarray with the following attributes:
 *   #label: The human-readable name of the destination type. Capitalize and wrap in t(). Required.
 *   #exposed: Flag indicating whether the destination type is exposed to the user for sending. Optional. Defaults to FALSE.
 *   #file: The file that needs to be included when invoking destination type hooks. Optional.
 *   #base: The base for destination type hooks. Optional. Defaults to the destination type name.
 */
function hook_emailkit_destination_info() {
  $info = array();

  $info['my_module_room'] = array(
    '#label' => t('Room'),
    '#exposed' => TRUE,
    '#file' => 'my_module.room.inc',
  );

  return $info;
}

/**
 * Returns the form for an exposed destination type. This form allows the user to specify a concrete destination.
 *
 * This is a destination type-level hook. Implementation is optional.
 *
 * @param $destination_type The name of the destination type.
 *
 * @return A form array.
 */
function hook_destination_form($destination_type) {
  $form = array();

  $form['number'] = array(
    '#type' => 'textfield',
    '#title' => t('Room number'),
    '#required' => TRUE,
  );
  
  return $form;
}

/**
 * Validates the form for an exposed destination type (@see hook_destination_form()).
 *
 * This is a destination type-level hook. Implementation is optional. This hook will not be called if #element_validate was set in hook_destination_form().
 *
 * @param &$form A form array. The name of the destination type can be found in $form['#destination_type'], unless this value was already set in hook_destination_form().
 */
function hook_destination_form_validate(&$form) {
  if (!empty($form['number']['#value']) && !is_numeric($form['number']['#value'])) {
    form_error($form['number'], t('Room number must be numeric.'));
  }
}

/**
 * Submits the form for an exposed destination type (@see hook_destination_form()). The purpose of this hook is to extend the structured destination array with destination type-specific attributes.
 *
 * This is a destination type-level hook. Implementation is optional.
 *
 * @param $destination The structured destination array, with just the #type attribute set.
 * @param $form_values The values submitted by the form.
 */
function hook_destination_form_submit(&$destination, $form_values) {
  $destination['#number'] = $form_values['number'];
}

/**
 * Returns the dispatchers defined by this module.
 *
 * Dispatchers send a message to a destination.
 *
 * For each destination type, there must be at least one dispatcher that services it. If there is more than one dispatcher for a given destination type, then the user can choose which dispatcher is used.
 *
 * This is a module-level hook. Implementation is optional, but must usually be implemented if hook_emailkit_destination_info() is implemented.
 *
 * @return An array of dispatcher information keyed by dispatcher name. Dispatcher information is a subarray with the following attributes:
 *   #label: The human-readable name of the dispatcher. Capitalize and wrap in t(). Required.
 *   #destinations: An array of names of supported destination types. Required.
 *   #file: The file that needs to be included when invoking dispatcher hooks. Optional.
 *   #base: The base for dispatcher hooks. Optional. Defaults to the dispatcher name.
 */
function hook_emailkit_dispatcher_info() {
  $info = array();

  $info['my_module_room'] = array(
    '#label' => t('Delivery boy'),
    '#destinations' => array('my_module_room'),
    '#file' => 'my_module.room.inc',
  );

  return $info;
}

/**
 * Sends a message to a destination.
 *
 * This is a dispatcher-level hook. Implementation is optional, but must be implemented if the dispatcher wants to do anything useful.
 *
 * @param $message The structured message array.
 * @param $destination The structured destination array, with at least the #type attribute set.
 *
 * @return Flag indicating whether sending was successful.
 */
function hook_dispatcher_send($dispatcher_name, $message, $destination) {
  return FALSE;
}

/**
 * Returns the message types defined by this module.
 *
 * All types of messages sent by a module must be defined by the implementation of this hook, so that users can enable/disable various features for each messages type.
 *
 * This is a module-level hook. Implementation is optional.
 *
 * @return An array of message type information keyed by message id. Message type information is a subarray with the following attributes:
 *   #label: The human-readable name of the message type. Capitalize and wrap in t(). Required.
 *   #file: The file that needs to be included when invoking message type hooks. Optional.
 *   #base: The base for message type hooks. Optional. Defaults to the message id.
 */
function hook_emailkit_message_info() {
  $info = array();

  $info['my_module_node'] = array(
    '#label' => t('Content'),
    '#file' => 'my_module.node.inc',
  );

  return $info;
}

/**
 * Returns a structured message array for the given message id.
 *
 * This is a message type-level hook. Implementation is required.
 *
 * @param $message_id The message id.
 * @param ... Any additional arguments that were passed into emailkit_message_get().
 *
 * @return A structured message array.
 */
function hook_message($message_id, $node) {
  return array(
    '#subject' => $node->title,
    '#body' => array(
      'node' => array(
        '#value' => node_view($node),
      ),
    ),
  );
}
