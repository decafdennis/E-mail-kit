<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the page for the E-mail tab on a node.
 */
function emailkit_node_email($node) {
  return drupal_get_form('emailkit_node_email_form', $node);
}

/**
 * Returns the form for the E-mail tab on a node.
 */
function emailkit_node_email_form(&$form_state, $node) {
  $form = array(
    '#node' => $node,
  );
  
  // TODO: Allow a destination to be selected
  
  $form['send'] = array(
    '#type' => 'submit',
    '#value' => t('Send'),
  );
  
  return $form;
}

/**
 * Handles submission of the form for the E-mail tab on a node.
 */
function emailkit_node_email_form_submit($form, &$form_state) {
  $node = $form['#node'];
  
  // TODO: Send e-mail
  dsm($node);
}
