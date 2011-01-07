<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of the view operation for the subscriptions block.
 */
function emailkit_list_block_subscriptions_view() {
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

  $form = array();
  
  // Build an array of options to choose from
  $options = array();
  foreach ($lists as $lid => $list) {
    $options[$lid] = $list->name;
  }
  
  // Try to find the subscriber
  if ($user->uid) {
    $subscriber = emailkit_list_subscriber_load_by_user($user->uid);
  }

  $form['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => isset($subscriber) ? emailkit_list_subscriber_load_subscriptions($subscriber->sid) : array(),
  );

  $form['mail'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#size' => 24,
    '#required' => TRUE,
  );

  // If we are dealing with a registered user, display the e-mail address field for verification but don't allow the user to change it from here
  if ($user->uid) {
    $form['mail']['#value'] = $user->mail;
    $form['mail']['#attributes']['disabled'] = 'disabled';
    $form['mail']['#description'] = t('This is the e-mail address associated with your account. Visit !page to change it.', array('!page' => l(t('your account page'), 'user/' . $user->uid . '/edit')));
  }

  $form['update'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  
  return $form;
}

/**
 * Handles validation for the subscriptions block form.
 */
function emailkit_list_block_subscriptions_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['mail']) && !valid_email_address($form_state['values']['mail'])) {
    form_set_error('mail', t('A valid e-mail address is required if you want to update your subscriptions.'));
  }
}

/**
 * Handles submission for the subscriptions block form.
 */
function emailkit_list_block_subscriptions_form_submit($form, &$form_state) {
  global $user;
  
  // Find the subscriber or create one
  if ($user->uid) {
    $subscriber = emailkit_list_subscriber_load_by_user($user->uid);
    
    if (!isset($subscriber)) {
      $subscriber = emailkit_list_subscriber_new();
      $subscriber->address = $user->mail;
      $subscriber->uid = $user->uid;
      
      // If we were unable to save the subscriber, set it to NULL so we can check this condition later
      if (!emailkit_list_subscriber_save($subscriber)) {
        unset($subscriber);
      }
    }
  }
  else {
    $subscriber = emailkit_list_subscriber_load_by_address($form_state['values']['mail']);

    if (!isset($subscriber)) {
      $subscriber = emailkit_list_subscriber_new();
      $subscriber->address = $form_state['values']['mail'];
      
      // If we were unable to save the subscriber, set it to NULL so we can check this condition later
      if (!emailkit_list_subscriber_save($subscriber)) {
        unset($subscriber);
      }
    }
  }
  
  $subscriptions = array_keys(array_filter($form_state['values']['subscriptions']));

  // Check whether we have a subscriber and try to save the subscriptions
  // TODO: If no user is currently logged in, we must send an e-mail first to confirm the change!
  if (!isset($subscriber) || !emailkit_list_subscriber_save_subscriptions($subscriber->sid, $subscriptions)) {
    drupal_set_message(t('Your subscriptions could not be updated.'), 'error');
  }
  else {
    drupal_set_message(t('Your subscriptions have been updated.'));
  }
}
