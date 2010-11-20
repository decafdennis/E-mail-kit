<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the form elements that should be injected into the user profile form.
 */
function emailkit_list_user_form($user) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
  
  $lists = EmailkitList::loadAll();
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
    $options[$lid] = $list->getName();
  }
  $subscriber = EmailkitListSubscriber::loadWithUserId($user->uid);
  $form['emailkit_list_subscriptions']['subscriptions'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => isset($subscriber) ? $subscriber->getSubscriptionListIds() : array(),
  );
  
  return $form;
}

/**
 * Handles submission of the form elements that have been injected into the user profile form.
 */
function emailkit_list_user_form_submit($user, &$values) {
  if (isset($values['emailkit_list_subscriptions'])) {
    $subscriber = EmailkitListSubscriber::loadWithUserId($user->uid);

    if (!isset($subscriber)) {
      $subscriber = new EmailkitListSubscriber($user->mail, $user->uid);
    }
    $subscriber->setSubscriptionListIds(array_keys(array_filter($values['emailkit_list_subscriptions']['subscriptions'])));
    $subscriber->save();
    
    unset($values['emailkit_list_subscriptions']);
  }
}
