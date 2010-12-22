<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_dispatcher_send() for the emailkit_default dispatcher.
 */
function emailkit_default_dispatcher_send($dispatcher, $message, $destination, $destination_options) {
  // TODO
  dsm($destination_options);
}
