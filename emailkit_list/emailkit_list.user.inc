<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the form elements that should be injected into the user profile form.
 */
function emailkit_list_user_form($user) {
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
  
  $subscriber = emailkit_list_subscriber_load_by_user($user->uid);
  
  $form['emailkit_list_subscriptions']['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => isset($subscriber) ? emailkit_list_subscriber_load_subscriptions($subscriber->sid) : array(),
  );
  
  return $form;
}

/**
 * Handles submission of the form elements that have been injected into the user profile form.
 */
function emailkit_list_user_form_submit($user, &$values) {
  if (isset($values['emailkit_list_subscriptions'])) {
    $subscriber = emailkit_list_subscriber_load_by_user($user->uid);

    if (!isset($subscriber)) {
      $subscriber = new stdClass();
      $subscriber->mail = $user->mail;
      $subscriber->uid = $user->uid;
      emailkit_list_subscriber_save($subscriber);
    }
    
    $subscriptions = array_keys(array_filter($values['emailkit_list_subscriptions']['subscriptions']));
    
    if (!emailkit_list_subscriber_save_subscriptions($subscriber->sid, $subscriptions)) {
      drupal_set_message(t('Your subscriptions could not be saved.'), 'warning');
    }
    
    unset($values['emailkit_list_subscriptions']);
  }
}
