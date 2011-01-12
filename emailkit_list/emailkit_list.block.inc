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
  
  // Try to find the subscriber for the currently logged in user
  if ($user->uid) {
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail);
  }

  $form['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => isset($subscriber) ? emailkit_list_subscriber_load_subscriptions($subscriber->sid) : array(),
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
    '#value' => t('Update'),
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
  
  $subscriptions = array_keys(array_filter($form_state['values']['subscriptions']));
  
  // If we have a logged in user, we update the subscriptions right away
  if ($user->uid) {
    // Create or update the subscriber
    // Note: since a registered user has submitted this form, we count this is as a confirmed subscription
    $subscriber = emailkit_list_subscriber_load_by_address($user->mail, TRUE);
    $subscriber->confirmed = TRUE;

    // Save the subscriber and their subscriptions
    if (emailkit_list_subscriber_save($subscriber) && emailkit_list_subscriber_save_subscriptions($subscriber->sid, $subscriptions)) {
      drupal_set_message(t('Your subscriptions have been updated.'));
    }
    else {
      drupal_set_message(t('Your subscriptions could not be updated.'), 'error');
    }
  }
  // Otherwise, we send a confirmation e-mail
  else {
    // TODO: check whether this e-mail address is not already associated with a registered user and ask them to login, so that we don't have to send a confirmation e-mail?
    if (emailkit_list_confirmation_send($form_state['values']['address'], $subscriptions)) {
      drupal_set_message(t('An e-mail with a confirmation link has been sent to %address. Your subscriptions will be updated as soon as your confirmation has been received.', array('%address' => $form_state['values']['address'])));
    }
    else {
      drupal_set_message(t('Your subscriptions could not be updated.'), 'error');
    }
  }
}
