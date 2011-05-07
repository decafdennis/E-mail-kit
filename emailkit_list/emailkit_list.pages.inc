<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains callbacks for non-admin pages.
 */

/**
 * Callback for the page that allows a subscriber to update their subscriptions.
 */
function emailkit_list_subscriber_subscriptions_page() {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $lists = emailkit_list_load_all();
  
  // If we have no lists, just return so the block will disappear
  if (empty($lists)) {
    return;
  }
  else {
    return drupal_get_form('emailkit_list_subscriber_subscriptions_form', $lists);
  }
}

/**
 * Returns the form that allows a subscriber to update their subscriptions.
 */
function emailkit_list_subscriber_subscriptions_form(&$form_state, $lists) {
  global $user;
  
  module_load_include('inc', 'emailkit_list', 'emailkit_list');

  $form = array();

  // Build an array of options to choose from
  $options = array();
  foreach ($lists as $lid => $list) {
    $options[$lid] = check_plain($list->name);
  }
  
  // Try to find the subscriber for the currently logged in user
  if ($user->uid) {
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail);
  }
  
  if (isset($subscriber)) {
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
  }
  else {
    $subscriptions = array();
  }

  $form['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => array_keys($subscriptions),
    '#description' => t('Select the lists you want to be subscribed to. You will be unsubscribed from any lists you have not selected. If you deselect all lists, you will be unsubscribed from all lists.'),
  );
  
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#description' => t('Your e-mail address.'),
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#size' => 24,
    '#required' => TRUE,
  );

  // If we are dealing with a registered user, display the e-mail address field for verification but don't allow the user to change it from here
  if ($user->uid) {
    $form['address']['#value'] = $user->mail;
    $form['address']['#attributes']['disabled'] = 'disabled';
    $form['address']['#description'] = t('This is the e-mail address associated with your account. Visit !page to change it.', array('!page' => l(t('your account page'), 'user/' . $user->uid . '/edit')));
  }

  $form['update'] = array(
    '#type' => 'submit',
    '#value' => t('Update subscriptions'),
  );
  
  return $form;
}

/**
 * Validates the form that allows a subscriber to update their subscriptions.
 */
function emailkit_list_subscriber_subscriptions_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['address']) && !valid_email_address($form_state['values']['address'])) {
    form_set_error('mail', t('A valid e-mail address is required if you want to update your subscriptions.'));
  }
}

/**
 * Submits the form that allows a subscriber to update their subscriptions.
 */
function emailkit_list_subscriber_subscriptions_form_submit($form, &$form_state) {
  global $user;
  
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $lids = array_keys(array_filter($form_state['values']['subscriptions']));
  
  // If we have a logged in user, we update the subscriptions right away
  if ($user->uid) {
    // Fetch or create the subscriber
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail, TRUE);
    
    if (isset($subscriber->sid)) {
      $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
    }
    else {
      $subscriptions = array();
    }
    
    // Update the subscriptions, with confirmation
    $subscriptions = emailkit_list_subscriber_subscription_update_all($subscriptions, $lids, time());

    // Save the subscriber and their subscriptions
    if (emailkit_list_subscriber_save($subscriber) && emailkit_list_subscriber_subscription_save_all($subscriber->sid, $subscriptions)) {
      drupal_set_message(t('Your subscriptions have been updated.'));
    }
    else {
      drupal_set_message(t('Your subscriptions could not be updated.'), 'error');
    }
  }
  // Otherwise, we send a confirmation e-mail
  else {
    // TODO: check whether this e-mail address is not already associated with a registered user and ask them to login, so that we don't have to send a confirmation e-mail?
    if (emailkit_list_confirmation_send($form_state['values']['address'], 'update_all', $lids)) {
      drupal_set_message(t('An e-mail with a confirmation link has been sent to %address. Your subscriptions will be updated as soon as your confirmation has been received.', array('%address' => $form_state['values']['address'])));
    }
    else {
      drupal_set_message(t('Your subscriptions could not be updated.'), 'error');
    }
  }
}
