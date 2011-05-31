<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains code that can be loaded on-demand.
 */

/**
 * Implementation of hook_message() for the emailkit_node message.
 */
function emailkit_node_message($message_id, $node, $preview = FALSE) {
  global $user;
  
  return array(
    '#node' => $node, // For later reference
    '#emailkit_statistics' => !$preview, // Enables statistics, if the emailkit_statistics module is enabled
    '#from' => array(
      'name' => $user->name,
      'address' => $user->mail,
    ),
    '#subject' => $node->title,
    'content' => array(
      'node' => array(
        '#theme' => 'emailkit_node',
        '#node' => $node,
      ),
    ),
  );
}

/**
 * Themes a node in a message.
 *
 * This function should be overridden by themers that want to customize how a node looks in an e-mail message.
 */
function theme_emailkit_node($element) {
  // Clone the node, since node_view() might alter it preventing it from being rendered again
  $node = drupal_clone($element['#node']);
  
  // Set the node's build mode. This is currently used in emailkit_node_preprocess_node(), but it can also be used by CCK
  $node->build_mode = 'emailkit';

  return node_view($node);
}

/**
 * Formats the given nodereference element.
 */
function theme_emailkit_formatter_emailkit($element) {
  // Keeps track of nodes that we have formatted already
  static $formatted_nodes = array();

  $output = "";

  // Clear out the array of formatted nodes, if the referencing node is at the root of the reference tree
  if (!isset($element['#node']->referencing_node)) {
    $formatted_nodes = array();
  }

  // Try to load the reference node
  if (isset($element['#item']['safe']['nid'])) {
    $referenced_node = node_load($element['#item']['safe']['nid']);
  }

  // Only continue if the node can be found and if we haven't already formatted it, to prevent infinite recursion
  if (isset($referenced_node) && !in_array($referenced_node->nid, $formatted_nodes)) {
    // Perform the primary purpose of this formatter: setting the node's build mode
    $referenced_node->build_mode = 'emailkit';

    $referenced_node->referencing_node = $element['#node'];
    $referenced_node->referencing_field = content_fields($element['#field_name'], $element['#type_name']);

    $output = node_view($referenced_node);

    // Remember that we have formatted this node, to prevent infinite recursion
    $formatted_nodes[] = $referenced_node->nid;
  }

  return $output;
}
