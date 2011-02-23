<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This file documents all the email hooks that can be implemented other modules.
 */

/**
 * Returns the destination types defined by this module.
 *
 * A destination type represents an addressing domain. For example, the domain of:
 * - e-mail addresses, or the domain of
 * - e-mail lists defined in some external system.
 *
 * Destination types can be exposed to the user. If a destination type is not exposed, it can only be used internally. However, if a destination type is exposed, then the user can use it to specify a concrete destination and send a message to it. For example, the e-mail address destination type might allow the user to enter an e-mail address in order to send a message to that address.
 *
 * When using emailkit_send(), the $destination argument is a structured array with its #type set to one of the destination types defined by implementations of this hook.
 *
 * This is a module-level hook. Implementation is optional.
 *
 * @return An array of destination type information keyed by destination type name. Destination type information is a subarray with the following attributes:
 *   #label: The human-readable name of the destination type. Capitalize and wrap in t(). Required.
 *   #exposed: Flag indicating whether the destination type is exposed to the user for sending. Optional. Defaults to FALSE.
 *   #default_dispatcher: The default dispatcher for this destination type. Required.
 *   #file: The file that needs to be included when invoking destination type hooks. Optional.
 *   #base: The base for destination type hooks. Optional. Defaults to the destination type name.
 */
function hook_emailkit_destination_info() {
  $info = array();

  $info['my_module_room'] = array(
    '#label' => t('Room'),
    '#exposed' => TRUE,
    '#default_dispatcher' => 'my_module_room',
    '#file' => 'my_module.room.inc',
  );

  return $info;
}

/**
 * Returns the form for an exposed destination type. This form allows the user to specify a concrete destination.
 *
 * This is a destination type-level hook. Implementation is optional. If it is implemented, it is only called if #exposed is TRUE for the destination type.
 *
 * @param $destination_type The name of the destination type.
 *
 * @return A form array.
 */
function hook_destination_form($destination_type) {
  $form = array();

  $form['room_number'] = array(
    '#type' => 'textfield',
    '#title' => t('Room number'),
    '#required' => TRUE,
  );
  
  return $form;
}

/**
 * Validates the form for an exposed destination type (@see hook_destination_form()).
 *
 * This is a destination type-level hook. Implementation is optional. This hook may not be called if #element_validate was modified in hook_destination_form().
 *
 * @param &$form A form array. The name of the destination type can be found in $form['#destination_type'], unless this value was already set in hook_destination_form().
 */
function hook_destination_form_validate(&$form) {
  if (!empty($form['room_number']['#value']) && !is_numeric($form['room_number']['#value'])) {
    form_error($form['room_number'], t('Room number must be numeric.'));
  }
}

/**
 * Submits the form for an exposed destination type (@see hook_destination_form()). The purpose of this hook is to extend the structured destination array with destination type-specific attributes.
 *
 * This is a destination type-level hook. Implementation is optional.
 *
 * @param $destination The structured destination array, with just the #type attribute set. This array should be extended with information specific to the destination type based on the form values.
 * @param $form_values The values submitted by the form.
 */
function hook_destination_form_submit(&$destination, $form_values) {
  $destination['#room_number'] = $form_values['room_number'];
}

/**
 * Returns the dispatchers defined by this module.
 *
 * Dispatchers send a message to a destination. For each destination type, there must at least be one dispatcher that services it. If there is more than one dispatcher for a given destination type, then the user can choose which dispatcher is used for each message.
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
  // TODO: Make the delivery boy deliver $message to $destination['#room_number']
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
 *   #label: The human-readable name of the message type. It is not necessary to include the module name. Capitalize and wrap in t(). Required.
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
 * This hook will be invoked when emailkit_message() is called with the given message id. The result of emailkit_message() can be passed into emailkit_send() for sending.
 *
 * This is a message type-level hook. Implementation is required.
 *
 * @param $message_id The message id.
 * @param ... Any additional arguments that were passed into emailkit_message().
 *
 * @return A structured message array. This will ultimately be rendered using drupal_render() for the HTML version and then converted using drupal_html_to_text() for the plain text version. Relative URLs generated by url() or l() will automatically be converted to absolute URLs. All content should be placed in header, content and footer regions by convention. In addition, the footer should contain a rationale explaining why the recipient is receiving the message.
 */
function hook_message($message_id, $node) {
  return array(
    '#subject' => $node->title,
    'header' => array(
      // ...
    ),
    'content' => array(
      'node' => array(
        '#value' => node_view($node),
      ),
    )
    'footer' => array(
      'rationale' => array(
        '#value' => '<p>' . t('You are receiving this message because...') . '</p>',
      ),
    ),
  );
}

/**
 * Invoked during the message building process to allow other modules to add their elements to the message.
 *
 * This hook is invoked before hook_emailkit_message_alter(). If you are only adding content to the message and/or if your changes do not depend on another module's changes, you should implement this hook.
 */
function hook_emailkit_message_build($message_id, &$message) {
  $message['footer']['environment_notice'] = array(
    '#value' => '<p>' . t('Save the environment. Please do not print this message.') . '</p>',
  );
}

/**
 * Invoked during the message building process to allow other modules to alter the message.
 *
 * This hook is invoked after hook_emailkit_message_build(). If you want to make changes to other modules' elements, you should implement this hook.
 */
function hook_emailkit_message_alter($message_id, &$message) {
  if ($message_id == 'my_module_node') {
    $message['content']['node']['']
  }
}

/**
 * Invoked right before a message is sent to allow other modules to alter the message, possibly based on the destination.
 *
 * Note that this hook may be invoked more than once with different destinations, since a dispatcher can choose to forward it to another destination.
 */
function hook_emailkit_message_before_send(&$message, &$destination) {
  // Only act if this is not a recursive invocation of emailkit_send()
  if ($message['#sending_depth'] == 0) {
    // TODO: Do something to the message independent of the destination
  }
}

/**
 * Invoked after a message has been rendered to allow other modules to apply last minute filters to the message.
 */
function hook_emailkit_message_render($message, $format, &$output) {
  $output = str_replace($output, 'replace this', 'with this');
}

/**
 * Invoked right after a message is sent to allow other modules to save information about the message.
 *
 * Note that this hook may be invoked more than once with different destinations, since a dispatcher can choose to forward it to another destination.
 */
function hook_emailkit_message_after_send($message, $destination, $success) {
  // Only act if this is not a recursive invocation of emailkit_send()
  if ($success && $message['#sending_depth'] == 0) {
    // TODO: Save information about the message using it's #token
  }
}
