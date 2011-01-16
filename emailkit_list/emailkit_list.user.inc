<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the form elements that should be injected into the user profile form.
 */
function emailkit_list_user_form($user) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $lists = emailkit_list_load_all();
  if (empty($lists)) {
    return array();
  }

  $form = array();
  
  $form['emailkit_list_subscriptions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Subscriptions'),
    '#description' => t('Select the lists to which you want to be subscribed to.'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  $options = array();
  foreach ($lists as $lid => $list) {
    $options[$lid] = $list->name;
  }
  
  $subscriber = emailkit_list_subscriber_load_by_address($user->mail);
  
  $form['emailkit_list_subscriptions']['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => isset($subscriber) ? emailkit_list_subscriber_load_subscriptions($subscriber->sid) : array(),
  );
  
  return $form;
}

/**
 * Handles submission of the user profile form.
 */
function emailkit_list_user_form_submit($user, &$values) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  if (isset($values['mail']) && $values['mail'] != $user->mail) {
    // Check if we have an existing subscriber
    if ($subscriber = emailkit_list_subscriber_load_by_address($user->mail)) {
      $subscriber->address = $values['mail'];
      emailkit_list_subscriber_save($subscriber);
    }
  }
  
  if (isset($values['emailkit_list_subscriptions'])) {
    // Create or update the subscriber
    // Note: make sure to use the new e-mail address to find the subscriber ($user->mail has the old e-mail address)
    // Note: since a registered user has submitted this form, we count this is as a confirmed subscription
    $subscriber = emailkit_list_subscriber_load_by_address($values['mail'], TRUE);
    $subscriber->confirmed = TRUE;

    // Update the subscriptions
    $subscriptions = array_keys(array_filter($values['emailkit_list_subscriptions']['subscriptions']));
    if (!(emailkit_list_subscriber_save($subscriber) && emailkit_list_subscriber_save_subscriptions($subscriber->sid, $subscriptions))) {
      drupal_set_message(t('Your subscriptions could not be saved.'), 'warning');
    }
    
    unset($values['emailkit_list_subscriptions']);
  }
}
