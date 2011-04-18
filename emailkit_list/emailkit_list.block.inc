<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains functions related to blocks.
 */

/**
 * Implementation of the view operation for the subscriptions block.
 */
function emailkit_list_block_subscriptions_view() {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $lists = emailkit_list_load_all();
  
  // If we have no lists, just return so the block will disappear
  if (empty($lists)) {
    return;
  }
  else {
    return array(
      'subject' => t('Subscriptions'),
      'content' => drupal_get_form('emailkit_list_block_subscriptions_form', $lists),
    );
  }
}

/**
 * Returns the form for the subscriptions block.
 */
function emailkit_list_block_subscriptions_form(&$form_state, $lists) {
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
    '#description' => t('Select the lists you want to be subscribed to. If you deselect all lists, you will be unsubscribed from all lists.'),
  );

  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
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
 * Handles validation for the subscriptions block form.
 */
function emailkit_list_block_subscriptions_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['address']) && !valid_email_address($form_state['values']['address'])) {
    form_set_error('mail', t('A valid e-mail address is required if you want to update your subscriptions.'));
  }
}

/**
 * Handles submission for the subscriptions block form.
 */
function emailkit_list_block_subscriptions_form_submit($form, &$form_state) {
  global $user;
  
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $lids = array_keys(array_filter($form_state['values']['subscriptions']));
  
  // If we have a logged in user, we update the subscriptions right away
  if ($user->uid) {
    // Fetch or create the subscriber
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail, TRUE);
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
    
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

/**
 * Implementation of the view operation for the subscription block of a specific list.
 */
function emailkit_list_block_subscription_view($list) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');

  return array(
    'subject' => check_plain($list->name),
    'content' => drupal_get_form('emailkit_list_block_subscription_' . $list->lid . '_form', $list),
  );
}

/**
 * Returns the form for the subscription block of a specific list.
 */
function emailkit_list_block_subscription_form(&$form_state, $list) {
  global $user;
  
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $form = array();
  $form['#list'] = $list;
  
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
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
  
  // Try to find the subscriber for the currently logged in user
  if ($user->uid) {
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail);
  }
  
  // Try to load the user's subscriptions
  if (isset($subscriber)) {
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
  }
  
  // Show an unsubscribe button when the user is subscribed or when we don't know
  if (!isset($subscriptions) || isset($subscriptions[$list->lid])) {
    $form['unsubscribe'] = array(
      '#type' => 'submit',
      '#value' => t('Unsubscribe'),
      '#emailkit_subscribe' => FALSE,
    );
  }
  
  // Show an subscribe button when the user is not subscribed or when we don't know
  if (!isset($subscriptions) || !isset($subscriptions[$list->lid])) {
    $form['subscribe'] = array(
      '#type' => 'submit',
      '#value' => t('Subscribe'),
      '#emailkit_subscribe' => TRUE,
    );
  }
  
  // Since this is a form builder used by more than one form, explicitly set the validation and submission callbacks
  $form['#validate'] = array('emailkit_list_block_subscription_form_validate');
  $form['#submit'] = array('emailkit_list_block_subscription_form_submit');
  
  return $form;
}

/**
 * Handles validation for the subscription block of a specific list.
 */
function emailkit_list_block_subscription_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['address']) && !valid_email_address($form_state['values']['address'])) {
    form_set_error('mail', t('A valid e-mail address is required if you want to update your subscription.'));
  }
}

/**
 * Handles submission for the subscription block of a specific list.
 */
function emailkit_list_block_subscription_form_submit($form, &$form_state) {
  global $user;

  module_load_include('inc', 'emailkit_list', 'emailkit_list');

  $list = $form['#list'];
  $subscribe = $form_state['clicked_button']['#emailkit_subscribe'];

  // If we have a logged in user, we update the subscriptions right away
  if ($user->uid) {
    // Fetch or create the subscriber
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail, TRUE);
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
    
    // Update the subscriptions, with confirmation
    if ($subscribe) {
      if (!isset($subscriptions[$list->lid])) {
        $subscription = emailkit_list_subscriber_subscription_new($list->lid);
        $subscription->confirm_date = time();
        $subscriptions[$list->lid] = $subscription;
      }
    }
    else {
      unset($subscriptions[$list->lid]);
    }

    // Save the subscriber and their subscriptions
    if (emailkit_list_subscriber_save($subscriber) && emailkit_list_subscriber_subscription_save_all($subscriber->sid, $subscriptions)) {
      drupal_set_message(t('Your subscription has been updated.'));
    }
    else {
      drupal_set_message(t('Your subscription could not be updated.'), 'error');
    }
  }
  // Otherwise, we send a confirmation e-mail
  else {
    // TODO: check whether this e-mail address is not already associated with a registered user and ask them to login, so that we don't have to send a confirmation e-mail?
    $action = ($subscribe ? 'subscribe' : 'unsubscribe');
    
    if (emailkit_list_confirmation_send($form_state['values']['address'], $action, array($list->lid))) {
      drupal_set_message(t('An e-mail with a confirmation link has been sent to %address. Your subscriptions will be updated as soon as your confirmation has been received.', array('%address' => $form_state['values']['address'])));
    }
    else {
      drupal_set_message(t('Your subscriptions could not be updated.'), 'error');
    }
  }
}
