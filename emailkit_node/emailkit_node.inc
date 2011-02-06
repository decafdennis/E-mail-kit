<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains code that can be loaded on-demand.
 */

/**
 * Implementation of hook_message() for the emailkit_node message.
 */
function emailkit_node_message($message_id, $node) {
  return array(
    '#subject' => $node->title,
    'content' => array(
      'node' => array(
        '#type' => 'emailkit_node',
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
  
  return node_view($node);
}
