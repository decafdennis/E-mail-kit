<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains functions related to users.
 */

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
    '#description' => t('Select the lists to which you want to be subscribed to. If you deselect all lists, you will be unsubscribed from all lists.'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
  );

  $options = array();
  foreach ($lists as $lid => $list) {
    $options[$lid] = check_plain($list->name);
  }
  
  $subscriber = emailkit_list_subscriber_load_by_address($user->mail);
  
  if (isset($subscriber)) {
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
  }
  else {
    $subscriptions = array();
  }

  $form['emailkit_list_subscriptions']['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => array_keys($subscriptions),
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
    // Fetch or create the subscriber
    // Note: make sure to use the new e-mail address to find the subscriber ($user->mail has the old e-mail address)
    $subscriber = emailkit_list_subscriber_load_by_address($values['mail'], TRUE);
    
    if (isset($subscriber->sid)) {
      $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
    }
    else {
      $subscriptions = array();
    }

    // Update the subscriptions, with confirmation
    $lids = array_keys(array_filter($values['emailkit_list_subscriptions']['subscriptions']));
    $subscriptions = emailkit_list_subscriber_subscription_update_all($subscriptions, $lids, time());

    // Save the subscriber and their subscriptions
    if (!(emailkit_list_subscriber_save($subscriber) && emailkit_list_subscriber_subscription_save_all($subscriber->sid, $subscriptions))) {
      drupal_set_message(t('Your subscriptions could not be saved.'), 'warning');
    }
    
    unset($values['emailkit_list_subscriptions']);
  }
}
