<?php
// Developed by Dennis Stevense for Digital Deployment

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
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');

  return drupal_get_form('emailkit_list_admin_list_form', EmailkitList::loadAll());
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

  foreach ($lists as $lid => $list) {
    $form['lists'][$lid]['#list'] = $list;
    
    if ($weights) {
      $form['lists'][$lid]['weight'] = array(
        '#type' => 'weight',
        '#default_value' => $list->getWeight(),
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
  foreach ($form_state['values']['lists'] as $lid => $list_values) {
    $list = $form['lists'][$lid]['#list'];
    
    // Update the list if necessary
    if ($list_values['weight'] != $list->getWeight()) {
      $list->setWeight($list_values['weight']);
      $list->save();
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
    '#default_value' => isset($list) ? $list->getName() : '',
    '#required' => TRUE,
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
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
  
  // Update the existing list or create a new one
  if (isset($form['#list'])) {
    $list = $form['#list'];
    $list->setName($form_state['values']['name']);
  }
  else {
    $list = new EmailkitList($form_state['values']['name']);
  }
  
  $result = $list->save();  
  switch ($result) {
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
    '#list' => $list,
  );

  return confirm_form($form,
    t('Are you sure you want to delete %name?', array('%name' => $list->getName())),
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
  if ($form_state['values']['confirm']) {
    $list = $form['#list'];
    
    $result = $list->delete();
    if ($result) {
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
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
  
  $form = array();
  
  $header = array(
    array('data' => t('E-mail address'), 'field' => 'address'),
    array('data' => t('User account'), 'field' => 'uid'),
  );
  $subscribers = EmailkitListSubscriber::loadAll($header, EMAILKIT_LIST_ADMIN_SUBSCRIBER_LIST_LIMIT);
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
  $form = array();
  
  // New, or existing list?
  if (isset($subscriber)) {
    $form['#subscriber'] = $subscriber;
  }
  
  $form['address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#description' => t('The e-mail address of the subscriber.'),
    '#default_value' => isset($subscriber) ? $subscriber->getAddress() : '',
    '#required' => TRUE,
  );
  
  if (isset($subscriber)) {
    $user = $subscriber->getUser();
  }
  $form['username'] = array(
    '#type' => 'textfield',
    '#title' => t('User account'),
    '#maxlength' => 60,
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => isset($user) ? $user->name : '',
    '#description' => t('The username of the subscriber. Leave blank if this subscriber has no user account on the site.'),
  );
  
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
  if (!empty($form_state['values']['address'])) {
    $address = $form_state['values']['address'];
    
    // Check whether it is a valid address
    if (!valid_email_address($address)) {
      form_set_error('address', t('E-mail address is invalid.'));
    }
    else {
      module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
      
      // Check whether the e-mail address is already associated with a different subscriber
      $address_subscriber = EmailkitListSubscriber::loadWithAddress($address);
      if ($address_subscriber != NULL && !(isset($form['#subscriber']) && $form['#subscriber']->getId() == $address_subscriber->getId())) {
        form_set_error('address', t('The e-mail address %address is already associated with another subscriber.', array('%address' => $address)));
      }
    }
  }

  if (!empty($form_state['values']['username'])) {
    // Check whether a user with that username exists
    $user = user_load(array('name' => $form_state['values']['username']));
    if (!$user) {
      form_set_error('username', t('The user account %username does not exist.', array('%username' => $form_state['values']['username'])));
    }
    else {
      module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
      
      // Check whether the user account is already associated with a different subscriber
      $user_subscriber = EmailkitListSubscriber::loadWithUserId($user->uid);
      if ($user_subscriber != NULL && !(isset($form['#subscriber']) && $form['#subscriber']->getId() == $user_subscriber->getId())) {
        form_set_error('username', t('The user account %username is already associated with another subscriber.', array('%username' => $form_state['values']['username'])));
      }
    }
  }
}

/**
 * Handles submission of a subscriber admin form.
 */
function emailkit_list_admin_subscriber_form_submit($form, &$form_state) {
  module_load_include('inc', 'emailkit_list', 'emailkit_list.model');
  
  // Determine the entered user ID
  if (!empty($form_state['values']['username']) && ($user = user_load(array('name' => $form_state['values']['username'])))) {
    $uid = $user->uid;
  }
  else {
    $uid = 0;
  }
  
  // Update the existing list or create a new one
  if (isset($form['#subscriber'])) {
    $subscriber = $form['#subscriber'];
    $subscriber->setAddress($form_state['values']['address']);
    $subscriber->setUserId($uid);
  }
  else {
    $subscriber = new EmailkitListSubscriber($form_state['values']['address'], $uid);
  }
  
  $result = $subscriber->save();  
  switch ($result) {
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
    '#subscriber' => $subscriber,
  );

  return confirm_form($form,
    t('Are you sure you want to delete %address?', array('%address' => $subscriber->getAddress())),
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
  if ($form_state['values']['confirm']) {
    $subscriber = $form['#subscriber'];
    
    $result = $subscriber->delete();
    if ($result) {
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
    $row[] = check_plain($form['lists'][$lid]['#list']->getName());
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
  $output = "";

  // Start building the subscribers table
  $header = array(
    array('data' => t('E-mail address'), 'field' => 'address'),
    array('data' => t('User account'), 'field' => 'uid'),
    array('data' => t('Operations'), 'colspan' => 2)
  );
  $rows = array();
  foreach (element_children($form['subscribers']) as $sid) {
    $subscriber = $form['subscribers'][$sid]['#subscriber'];
    
    $row = array();
    $row[] = check_plain($subscriber->getAddress());
    $row[] = theme('username', $subscriber->getUser());
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
