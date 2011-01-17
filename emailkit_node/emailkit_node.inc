<?php
// Developed by Dennis Stevense for Digital Deployment


/**
 * Implementation of hook_message() for the emailkit_node message.
 */
function emailkit_node_message($message_id, $node) {
  return array(
    '#subject' => $node->title,
    '#body' => array(
      'node' => array(
        '#value' => $node->teaser,
      ),
    ),
  );
}
