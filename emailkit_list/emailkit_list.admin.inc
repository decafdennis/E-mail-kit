<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains callbacks and related functions for admin pages.
 */

/**
 * The HTML id used for the lists table.
 */
define('EMAILKIT_LIST_ADMIN_LIST_FORM_LISTS_ID', 'lists');

/**
 * The HTML class used for a weight field in the lists table.
 */
define('EMAILKIT_LIST_ADMIN_LIST_FORM_LIST_WEIGHT_CLASS', 'list-weight');

/**
 * The number of subscribers displayed at once on the subscribers admin page.
 */
define('EMAILKIT_LIST_ADMIN_SUBSCRIBER_LIST_LIMIT', 50);

/**
 * Returns the admin page that lists the e-mail lists.
 */
function emailkit_list_admin_list() {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  return drupal_get_form('emailkit_list_admin_list_form', emailkit_list_load_all());
}

/**
 * Returns the e-mail lists admin form.
 */
function emailkit_list_admin_list_form(&$form_state, $lists) {
  $form = array(
    '#tree' => TRUE,
  );
  
  // Only show weight fields if there is more than one list
  $weights = count($lists) > 1;

  $form['lists'] = array();
  foreach ($lists as $lid => $list) {
    $form['lists'][$lid]['#list'] = $list;
    
    if ($weights) {
      $form['lists'][$lid]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $list->weight,
      );
    }
  }
  
  // We need a submit button only when we have weight fields
  if ($weights) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
  }

  return $form;
}

/**
 * Handles submission of the e-mail lists admin form.
 */
function emailkit_list_admin_list_form_submit($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  foreach ($form_state['values']['lists'] as $lid => $list_values) {
    $list = $form['lists'][$lid]['#list'];
    
    // Update the list if necessary
    if ($list_values['weight'] != $list->weight) {
      $list->weight = $list_values['weight'];
      emailkit_list_save($list);
    }
  }
}

/**
 * Returns the admin page for adding an e-mail list.
 */
function emailkit_list_admin_add() {
  return drupal_get_form('emailkit_list_admin_form');
}

/**
 * Returns the admin page for editing an e-mail list.
 */
function emailkit_list_admin_edit($list) {
  return drupal_get_form('emailkit_list_admin_form', $list);
}

/**
 * Returns an e-mail list admin form.
 */
function emailkit_list_admin_form(&$form_state, $list = NULL) {
  $form = array();
  
  // New, or existing list?
  if (isset($list)) {
    $form['#list'] = $list;
  }
  
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('The name of the list.'),
    '#default_value' => isset($list) ? $list->name : '',
    '#required' => TRUE,
  );
  
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#description' => t('A brief description of the list. This text will be displayed to the user, as part of the subscription blocks, for example. This text may contain HTML. In fact, you should use HTML if you want to include multiple paragraphs of text.'),
    '#default_value' => isset($list) ? $list->description : "",
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  return $form;
}

/**
 * Handles submission of an e-mail list admin form.
 */
function emailkit_list_admin_form_submit($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  // Get the list
  if (isset($form['#list'])) {
    $list = $form['#list'];
  }
  else {
    $list = emailkit_list_new();
  }
  
  // Update the name and description
  $list->name = $form_state['values']['name'];
  $list->description = $form_state['values']['description'];
  
  // Save the list
  switch (emailkit_list_save($list)) {
    case SAVED_NEW;
      drupal_set_message(t('The list has been added.'));
      break;
      
    case SAVED_UPDATED:
      drupal_set_message(t('The list has been updated.'));
      break;

    default:
      drupal_set_message(t('The list could not be saved.'), 'error');
      break;
  }
  
  $form_state['redirect'] = 'admin/emailkit/list/lists';
}

/**
 * Returns the admin page for deleting an e-mail list.
 */
function emailkit_list_admin_delete($list) {
  return drupal_get_form('emailkit_list_admin_delete_confirm_form', $list);
}

/**
 * Returns a confirmation form for deleting an e-mail list.
 */
function emailkit_list_admin_delete_confirm_form(&$form_state, $list) {
  $form = array(
    '#lid' => $list->lid,
  );

  return confirm_form($form,
    t('Are you sure you want to delete %name?', array('%name' => $list->name)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/emailkit/list/lists',
    t('All information about subscriptions to this e-mail list will be removed as well. This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Handles submission of a confirmation form for deleting an e-mail list.
 */
function emailkit_list_admin_delete_confirm_form_submit($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  if ($form_state['values']['confirm']) {
    if (emailkit_list_delete($form['#lid'])) {
      drupal_set_message(t('The list has been deleted.'));
    }
    else {
      drupal_set_message(t('The list could not be deleted.'), 'error');
    }
  }
  
  $form_state['redirect'] = 'admin/emailkit/list/lists';
}

/**
 * Returns the admin page that lists subscribers.
 */
function emailkit_list_admin_subscriber_list() {
  return drupal_get_form('emailkit_list_admin_subscriber_list_form');
}

/**
 * Returns the subscribers admin form.
 */
function emailkit_list_admin_subscriber_list_form(&$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $form = array();
  
  $header = array(
    array('data' => t('E-mail address'), 'field' => 'address'),
  );

  $form['subscribers'] = array();
  $subscribers = emailkit_list_subscriber_load_all($header, EMAILKIT_LIST_ADMIN_SUBSCRIBER_LIST_LIMIT);
  foreach ($subscribers as $sid => $subscriber) {
    $form['subscribers'][$sid]['#subscriber'] = $subscriber;
  }
  
  $form['pager'] = array('#value' => theme('pager', NULL, EMAILKIT_LIST_ADMIN_SUBSCRIBER_LIST_LIMIT));

  return $form;
}

/**
 * Returns the admin page for adding a subscriber.
 */
function emailkit_list_admin_subscriber_add() {
  return drupal_get_form('emailkit_list_admin_subscriber_form');
}

/**
 * Returns the admin page for editing a subscriber.
 */
function emailkit_list_admin_subscriber_edit($subscriber) {
  return drupal_get_form('emailkit_list_admin_subscriber_form', $subscriber);
}

/**
 * Returns a subscriber admin form.
 */
function emailkit_list_admin_subscriber_form(&$form_state, $subscriber = NULL) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $form = array();

  // New, or existing subscriber?
  if (isset($subscriber)) {
    // Load the associated user
    $user = emailkit_list_user_load_by_address($subscriber->address);
    
    // Load the subscriber's subscriptions
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);

    $form['#subscriber'] = $subscriber;
    $form['#user'] = $user;
    $form['#subscriptions'] = $subscriptions;
  }
  else {
    $user = NULL;
    $subscriptions = array();
  }

  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
  );
  
  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('User account'),
  );
  
  if (isset($user) && $user->uid) {
    $form['address'] += array(
      '#value' => $user->mail,
      '#attributes' => array('disabled' => 'disabled'),
      '#description' => t('This e-mail address is associated with a user account. Visit !page to change it.', array('!page' => l(t('the account edit page'), 'user/' . $user->uid . '/edit'))),
    );
        
    $form['username'] += array(
      '#value' => $user->name,
      '#attributes' => array('disabled' => 'disabled'),
      '#description' => t('The user account associated with this subscriber.'),
    );
  }
  else {
    $form['address'] += array(
      '#default_value' => isset($subscriber) ? $subscriber->address : '',
      '#description' => t('The e-mail address of the subscriber. If you enter the address of a registered user, the subscriber will automatically be associated with that user account.'),
    );
     
    $form['username'] += array(
      '#maxlength' => 60,
      '#autocomplete_path' => 'user/autocomplete',
      '#default_value' => isset($user) && $user->uid ? $user->name : '',
      '#description' => t('The username of the subscriber. If you enter the name of a registered user, the subscriber will be associated with that user account and its e-mail address.'),
    );
  }

  $options = array();
  $lists = emailkit_list_load_all();
  foreach ($lists as $lid => $list) {
    $option = check_plain($list->name);
    
    // If the subscriber has been subscribed to this list, indicate whether it is a confirmed or unconfirmed subscription
    if (isset($subscriptions[$lid])) {
      $subscription = $subscriptions[$lid];
      
      if (!empty($subscription->confirm_date)) {
        $confirm_subtext = t('confirmed on %date', array('%date' => format_date($subscription->confirm_date, 'small')));
      }
      else {
        $confirm_subtext = t('unconfirmed');
      }
      
      $option .= " (" . $confirm_subtext . ")";
    }
    
    $options[$lid] = $option;
  }
  
  if (empty($options)) {
    $form['subscriptions'] = array(
      '#type' => 'value',
      '#value' => array(),
    );
    
    $form['subscriptions_empty'] = array(
      '#type' => 'item',
      '#title' => t('Subscriptions'),
      '#value' => t('No lists have been defined yet.'),
    );
  }
  else {
    $form['subscriptions'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Subscriptions'),
      '#options' => $options,
      '#default_value' => array_keys($subscriptions),
      '#description' => t('Select the lists to which the subscriber is subscribed to.'),
    );
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

/**
 * Handles validating of a subscriber admin form.
 */
function emailkit_list_admin_subscriber_form_validate($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $subscriber = isset($form['#subscriber']) ? $form['#subscriber'] : NULL;
  $user = isset($form['#user']) ? $form['#user'] : NULL;
  
  // We only need to validate the address and username fields if the subscriber is not already associated with a user account
  if (!isset($user) || !$user->uid) {
    // Process the address field
    if (!empty($form_state['values']['address'])) {
      $address = $form_state['values']['address'];
      
      if (valid_email_address($address)) {
        // Collect things that are associated with this address
        $address_subscriber = emailkit_list_subscriber_load_by_address($address);
        $address_user = emailkit_list_user_load_by_address($address, TRUE);
      }
      else {
        form_set_error('address', t('E-mail address is invalid.'));
        
        unset($address);
      }
    }
    
    // Process the username field
    if (!empty($form_state['values']['username'])) {
      $username = $form_state['values']['username'];
      
      if ($username_user = user_load(array('name' => $username))) {
        // Collect things that are associated with this user
        $username_subscriber = emailkit_list_subscriber_load_by_address($username_user->mail);
      }
      else {
        form_set_error('username', t('The user account %username does not exist.', array('%username' => $form_state['values']['username'])));
        
        unset($username);
        unset($username_user);
      }
    }
    
    // Check whether we don't have nothing
    if (empty($form_state['values']['address']) && empty($form_state['values']['username'])) {
      form_set_error('address', t('Either an e-mail address or a user account is required.'));
    }
    // Check whether things are consistent
    else if (isset($address_subscriber) && (!isset($subscriber) || $subscriber->sid != $address_subscriber->sid)) {
      form_set_error('address', t('The e-mail address %address is already associated with another subscriber.', array('%address' => $address)));
    }
    else if (isset($username_subscriber) && (!isset($subscriber) || $subscriber->sid != $username_subscriber->sid)) {
      form_set_error('username', t('The user account %username is already associated with another subscriber.', array('%username' => $username)));
    }
    else if (isset($address_user) && isset($username_user) && $address_user->uid != $username_user->uid) {
      form_set_error('address', t('The e-mail address %address does not match the user account %username. Please leave either the e-mail address or the user account blank or make them consistent.', array('%address' => $address, '%username' => $username)));
    }
  }
}

/**
 * Handles submission of a subscriber admin form.
 */
function emailkit_list_admin_subscriber_form_submit($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $subscriber = isset($form['#subscriber']) ? $form['#subscriber'] : NULL;
  $user = isset($form['#user']) ? $form['#user'] : NULL;
  $subscriptions = isset($form['#subscriptions']) ? $form['#subscriptions'] : array();
  
  // We only need to process the address and username fields if the subscriber is not already associated with a user account
  if (!isset($user) || !$user->uid) {
    if (!empty($form_state['values']['address'])) {
      // Associate this subscriber with the given e-mail address
      $address = $form_state['values']['address'];
    }
    else if (!empty($form_state['values']['username'])) {
      // Associate this subscriber with the e-mail address of the given user account
      $user = user_load(array('name' => $form_state['values']['username']));
      $address = $user->mail;
    }
    
    // Create or update the subscriber
    if (isset($subscriber)) {
      $subscriber->address = $address;
    }
    else {
      $subscriber = emailkit_list_subscriber_new($address);
    }
  }
  
  // Update the subscriptions
  $lids = array_keys(array_filter($form_state['values']['subscriptions']));
  $subscriptions = emailkit_list_subscriber_subscription_update_all($subscriptions, $lids);

  // Save the subscriber
  switch (emailkit_list_subscriber_save($subscriber)) {
    case SAVED_NEW;
      drupal_set_message(t('The subscriber has been added.'));
      break;
      
    case SAVED_UPDATED:
      drupal_set_message(t('The subscriber has been updated.'));
      break;

    default:
      drupal_set_message(t('The subscriber could not be saved.'), 'error');
      break;
  }

  // Update the subscriptions
  if (!emailkit_list_subscriber_subscription_save_all($subscriber->sid, $subscriptions)) {
    drupal_set_message(t('The subscriber\'s subscriptions could not be saved.'), 'warning');
  }
  
  $form_state['redirect'] = 'admin/emailkit/list/subscribers';
}

/**
 * Returns the admin page for deleting a subscriber.
 */
function emailkit_list_admin_subscriber_delete($subscriber) {
  return drupal_get_form('emailkit_list_admin_subscriber_delete_confirm_form', $subscriber);
}

/**
 * Returns a confirmation form for deleting a subscriber.
 */
function emailkit_list_admin_subscriber_delete_confirm_form(&$form_state, $subscriber) {
  $form = array(
    '#sid' => $subscriber->sid,
  );

  return confirm_form($form,
    t('Are you sure you want to delete %address?', array('%address' => $subscriber->address)),
    isset($_GET['destination']) ? $_GET['destination'] : 'admin/emailkit/list/subscribers',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Handles submission of a confirmation form for deleting a subscriber.
 */
function emailkit_list_admin_subscriber_delete_confirm_form_submit($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  if ($form_state['values']['confirm']) {
    if (emailkit_list_subscriber_delete($form['#sid'])) {
      drupal_set_message(t('The subscriber has been deleted.'));
    }
    else {
      drupal_set_message(t('The subscriber could not be deleted.'), 'error');
    }
  }
  
  $form_state['redirect'] = 'admin/emailkit/list/subscribers';
}

/**
 * Returns the rendered the e-mail lists admin form.
 *
 * @param $form The e-mail lists admin form.
 */
function theme_emailkit_list_admin_list_form($form) {
  $output = "";

  // Start building the lists table
  $header = array(t('Name'), array('data' => t('Operations'), 'colspan' => 2));
  $rows = array();
  $attributes = array('id' => EMAILKIT_LIST_ADMIN_LIST_FORM_LISTS_ID);
  $weights = FALSE;
  foreach (element_children($form['lists']) as $lid) {
    $row = array();
    $row[] = check_plain($form['lists'][$lid]['#list']->name);
    $row[] = l(t('edit'), 'admin/emailkit/list/lists/' . $lid . '/edit', array('query' => drupal_get_destination()));
    $row[] = l(t('delete'), 'admin/emailkit/list/lists/' . $lid . '/delete', array('query' => drupal_get_destination()));
    
    // Do we have a weight field?
    if (isset($form['lists'][$lid]['weight'])) {
      $weights = TRUE;
      
      _emailkit_list_element_add_class($form['lists'][$lid]['weight'], EMAILKIT_LIST_ADMIN_LIST_FORM_LIST_WEIGHT_CLASS);
      $row[] = drupal_render($form['lists'][$lid]['weight']);
    }
    
    $rows[] = array('data' => $row, 'class' => 'draggable');
  }

  // If we don't have any rows, add a placeholder
  if (empty($rows)) {
    $row = array();
    $row[] = array('data' => t('No lists have been defined yet.'), 'colspan' => 3);
    $rows[] = $row;
  }
  // Otherwise, if we have weights, add a table column to the header and enable tabledrag
  elseif ($weights) {
    $header[] = t('Weight');
    
    drupal_add_tabledrag(EMAILKIT_LIST_ADMIN_LIST_FORM_LISTS_ID, 'order', 'sibling', EMAILKIT_LIST_ADMIN_LIST_FORM_LIST_WEIGHT_CLASS);
  }
  
  $output .= theme('table', $header, $rows, $attributes);
  
  // Render any remaining elements in the form, such as the Save button
  $output .= drupal_render($form);

  return $output;
}

/**
 * Returns the rendered the subscribers admin form.
 *
 * @param $form The subscribers admin form.
 */
function theme_emailkit_list_admin_subscriber_list_form($form) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list');
  
  $output = "";

  // Start building the subscribers table
  $header = array(
    array('data' => t('E-mail address'), 'field' => 'address'),
    array('data' => t('User account')),
    array('data' => t('Operations'), 'colspan' => 2)
  );
  $rows = array();
  foreach (element_children($form['subscribers']) as $sid) {
    $subscriber = $form['subscribers'][$sid]['#subscriber'];
    $user = emailkit_list_user_load_by_address($subscriber->address);
    
    $row = array();
    $row[] = check_plain($subscriber->address);
    $row[] = theme('username', $user);
    $row[] = l(t('edit'), 'admin/emailkit/list/subscribers/' . $sid . '/edit', array('query' => drupal_get_destination()));
    $row[] = l(t('delete'), 'admin/emailkit/list/subscribers/' . $sid . '/delete', array('query' => drupal_get_destination()));
    $rows[] = $row;
  }

  // If we don't have any rows, add a placeholder
  if (empty($rows)) {
    $row = array();
    $row[] = array('data' => t('No subscribers have been defined yet.'), 'colspan' => 4);
    $rows[] = $row;
  }
  
  $output .= theme('table', $header, $rows);
  
  // Render any remaining elements in the form, such as the Save button
  $output .= drupal_render($form);

  return $output;
}
