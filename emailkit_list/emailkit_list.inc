<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains code that does not need to be in the main module file and can be loaded on-demand.
 */

/**
 * Returns a new unsaved list with default values.
 */
function emailkit_list_new() {
  $list = new stdClass();
  $list->name = "";
  $list->description = "";
  $list->weight = 0;
  return $list;
}

/**
 * Loads all lists from the database.
 *
 * @return An array of lists, keyed by their identifier and ordered by weight and name.
 */
function emailkit_list_load_all() {
  $lists = array();
  
  $result = db_query("SELECT * FROM {emailkit_list} ORDER BY weight ASC, name ASC");
  while ($list = db_fetch_object($result)) {
    $lists[$list->lid] = $list;
  }
  
  return $lists;
}

/**
 * Loads all subscribers that are subscribed to the given list from the database.
 *
 * @param $lid The identifier of the list.
 *
 * @return An array of subscribers, keyed and ordered by their unique identifier.
 */
function emailkit_list_load_subscribers($lid) {
  $subscribers = array();
  
  $result = db_query("SELECT sr.* FROM {emailkit_list_subscriber} sr INNER JOIN {emailkit_list_subscription} sn ON sr.sid = sn.sid WHERE sn.lid = %d", $lid);
  while ($subscriber = db_fetch_object($result)) {
    $subscribers[$subscriber->sid] = $subscriber;
  }
  
  return $subscribers;
}

/**
 * Saves the given list to the database, inserting a new record or updating an existing record as necessary.
 *
 * @return Returns FALSE, SAVED_NEW or SAVED_UPDATED depending on whether saving was successful. Upon return, a new list will automatically have an identifier assigned to it.
 */
function emailkit_list_save(&$list) {
  if (isset($list->lid)) {
    db_query("UPDATE {emailkit_list} SET name = '%s', description = '%s', weight = %d WHERE lid = %d", $list->name, $list->description, $list->weight, $list->lid);
    
    if (db_affected_rows() == 1) {
      return SAVED_UPDATED;
    }
    else {
      return FALSE;
    }
  }
  else {
    db_query("INSERT INTO {emailkit_list} (name, description, weight) VALUES ('%s', '%s', %d)", $list->name, $list->description, $list->weight);

    if (db_affected_rows() == 1) {
      $list->lid = db_last_insert_id('emailkit_list', 'lid');
      
      return SAVED_NEW;
    }
    else {
      return FALSE;
    }
  }
}

/**
 * Deletes the list with the given identifier from the database.
 *
 * @return A flag indicating whether the list was deleted.
 */
function emailkit_list_delete($lid) {
  db_query("DELETE FROM {emailkit_list} WHERE lid = %d", $lid);
  
  if (db_affected_rows() == 1) {
    db_query("DELETE FROM {emailkit_list_subscription} WHERE lid = %d", $lid);
    
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Returns a new unsaved subscriber with default values.
 *
 * @param $address The address of the new subscriber.
 *
 * @return An unsaved subscriber object.
 */
function emailkit_list_subscriber_new($address) {
  $subscriber = new stdClass();
  $subscriber->address = $address;
  return $subscriber;
}

/**
 * Loads the subscriber with the given address from the database, or optionally creates a new one.
 *
 * @param $address The e-mail address to look for.
 * @param $new Whether to create a new (unsaved) subscriber
 *
 * @return A subscriber object or NULL.
 */
function emailkit_list_subscriber_load_by_address($address, $new = FALSE) {
  if ($subscriber = db_fetch_object(db_query("SELECT * FROM {emailkit_list_subscriber} WHERE address = '%s'", $address))) {
    return $subscriber;
  }
  else if ($new) {
    return emailkit_list_subscriber_new($address);
  }
}

/**
 * Loads all subscribers from the database.
 *
 * @return An array of subscribers, keyed and ordered by their unique identifier.
 */
function emailkit_list_subscriber_load_all($tablesort_header = NULL, $pager_limit = NULL, $pager_element = 0) {
  $subscribers = array();
  
  // Build the SQL query
  $sql = "SELECT * FROM {emailkit_list_subscriber}";
  if (isset($tablesort_header)) {
    $sql .= tablesort_sql($tablesort_header);
  }
  else {
    $sql .= " ORDER BY sid ASC";
  }

  // Run the SQL query
  if (isset($pager_limit) && isset($pager_element)) {
    $result = pager_query($sql, $pager_limit, $pager_element);
  }
  else {
    $result = db_query($sql);
  }
  
  while ($subscriber = db_fetch_object($result)) {
    $subscribers[$subscriber->sid] = $subscriber;
  }
  
  return $subscribers;
}

/**
 * Saves the given subscriber to the database, inserting a new record or updating an existing record as necessary.
 *
 * @return Returns FALSE, SAVED_NEW or SAVED_UPDATED depending on whether saving was successful. Upon return, a new subscriber will automatically have an identifier assigned to it.
 */
function emailkit_list_subscriber_save(&$subscriber) {
  if (isset($subscriber->sid)) {
    db_query("UPDATE {emailkit_list_subscriber} SET address = '%s' WHERE sid = %d", $subscriber->address, $subscriber->sid);
    
    if (db_affected_rows() == 1) {
      return SAVED_UPDATED;
    }
    else {
      return FALSE;
    }
  }
  else {
    db_query("INSERT INTO {emailkit_list_subscriber} (address) VALUES ('%s')", $subscriber->address);

    if (db_affected_rows() == 1) {
      $subscriber->sid = db_last_insert_id('emailkit_list_subscriber', 'sid');
      
      return SAVED_NEW;
    }
    else {
      return FALSE;
    }
  }
}

/**
 * Deletes the subscriber with the given identifier from the database.
 *
 * @return A flag indicating whether the subscriber was deleted.
 */
function emailkit_list_subscriber_delete($sid) {
  db_query("DELETE FROM {emailkit_list_subscriber} WHERE sid = %d", $sid);
  
  if (db_affected_rows() == 1) {
    db_query("DELETE FROM {emailkit_list_subscription} WHERE sid = %d", $sid);
    
    return TRUE;
  }
  else {
    return FALSE;
  }
}

/**
 * Returns a new unsaved subscription with default values.
 *
 * @param $lid The identifier of the list.
 *
 * @return An unsaved subscription object.
 */
function emailkit_list_subscriber_subscription_new($lid) {
  $subscription = new stdClass();
  $subscription->lid = $lid;
  return $subscription;
}

/**
 * Loads the subscriptions of the given subscriber.
 *
 * @param $sid The identifier of the subscriber.
 *
 * @return An array of subscription objects keyed by list identifier.
 */
function emailkit_list_subscriber_subscription_load_all($sid) {
  $subscriptions = array();
  
  $result = db_query("SELECT lid, confirm_date FROM {emailkit_list_subscription} WHERE sid = %d", $sid);
  while ($subscription = db_fetch_object($result)) {
    $subscriptions[$subscription->lid] = $subscription;
  }
  
  return $subscriptions;
}

/**
 * Returns a new array of subscriptions objects by adding subscriptions for lists that exist in $lids but not in $subscriptions and removing subscriptions for lists that exist in $subscriptions but not in $lids.
 *
 * @param $subscriptions The original array of subscriptions, which does not need to be keyed by list identifier.
 * @param $lids The array of lists that the new array of subscriptions should denote.
 * @param $confirm_date If set, unconfirmed subscriptions will be confirmed using this confirmation date.
 *
 * @return An array of subscription objects keyed by list identifier.
 */
function emailkit_list_subscriber_subscription_update_all($subscriptions, $lids, $confirm_date = NULL) {
  $new_subscriptions = array();
  
  foreach ($subscriptions as $subscription) {
    if (in_array($subscription->lid, $lids)) {
      if (!empty($confirm_date) && empty($subscription->confirm_date)) {
        $subscription->confirm_date = $confirm_date;
      }
      
      $new_subscriptions[$subscription->lid] = $subscription;
    }
  }
  
  foreach ($lids as $lid) {
    if (!isset($new_subscriptions[$lid])) {
      $subscription = emailkit_list_subscriber_subscription_new($lid);
      
      if (!empty($confirm_date)) {
        $subscription->confirm_date = $confirm_date;
      }
      
      $new_subscriptions[$lid] = $subscription;
    }
  }
  
  return $new_subscriptions;
}

/**
 * Saves the subscriptions of the given subscriber. 
 *
 * @param $sid The identifier of the subscriber.
 * @param $subscriptions An array of subscription objects.
 *
 * @return A flag indicating whether saving was successful.
 */
function emailkit_list_subscriber_subscription_save_all($sid, $subscriptions) {
  db_query("DELETE FROM {emailkit_list_subscription} WHERE sid = %d", $sid);
  
  if (!empty($subscriptions)) {
    // Start constructing a compound insertion statement
    $sql = "INSERT INTO {emailkit_list_subscription} (sid, lid, confirm_date) VALUES ";
    $sql_args = array();
    $insertions = 0;
    
    $values_sql = array();
    foreach ($subscriptions as $subscription) {
      $subscription_values_sql = "(%d, %d, ";
      $sql_args[] = $sid;
      $sql_args[] = $subscription->lid;
      
      if (!empty($subscription->confirm_date)) {
        $subscription_values_sql .= "%d";
        $sql_args[] = $subscription->confirm_date;
      }
      else {
        $subscription_values_sql .= "NULL";
      }
      
      $subscription_values_sql .= ")";
      $values_sql[] = $subscription_values_sql;
      
      $insertions++;
    }
    $sql .= implode(", ", $values_sql);

    db_query($sql, $sql_args);
    
    // Verify the number of affected rows
    if (db_affected_rows() == $insertions) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  else {
    return TRUE;
  }
}

/**
 * Loads the user with the given address from the database. Unlike user_load(), this function returns the anonymous user or NULL when no such user exists.
 *
 * @param $address The e-mail address to look for.
 * @param $null Whether to return NULL instead of the anonymous user when the user is not found.
 *
 * @return A user object.
 */
function emailkit_list_user_load_by_address($address, $null = FALSE) {
  static $anonymous_user;
  
  $user = user_load(array('mail' => $address));
  
  // Fallback to the anonymous user if no user was found with that address
  if (!$user) {
    // Lazily load the anonymous user
    if (!isset($anonymous_user)) {
      $anonymous_user = user_load(0);
    }
    
    $user = $anonymous_user;
  }
  
  return $user;
}

/**
 * Sends a message to the given address that contains a link to confirm subscription to the given lists.
 *
 * @param $address A subscriber's e-mail address.
 * @param $action The action to confirm. One of update_all, subscribe, unsubscribe.
 * @param $lids An array of list identifiers.
 *
 * @return A flag indicating whether the message was sent successfully.
 */
function emailkit_list_confirmation_send($address, $action, $lids) {
  assert(in_array($action, array('update_all', 'subscribe', 'unsubscribe')));
  
  $message = emailkit_message('emailkit_list_confirmation', $address, $action, $lids);

  $destination = array(
    '#type' => 'emailkit_default',
    '#recipients' => array(
      array(
        'address' => $address,
      ),
    ),
  );

  return emailkit_send($message, $destination);
}

/**
 * Implementation of hook_message() for the emailkit_list_confirmation message.
 */
function emailkit_list_confirmation_message($message_id, $address, $action, $lids) {
  $message = array(
    '#subject' => t('Confirm your subscriptions'),
  );

  $show_items = FALSE;

  if ($action == 'subscribe' || ($action == 'update_all' && !empty($lids))) {
    $message['content']['subscription_info'] = array(
      '#value' => '<p>' . t('You have requested to be subscribed to the following lists:') . '</p>',
    );
    
    $show_items = TRUE;
  }
  else if ($action == 'unsubscribe') {
    $message['content']['subscription_info'] = array(
      '#value' => '<p>' . t('You have requested to be unsubscribed from the following lists:') . '</p>',
    );
    
    $show_items = TRUE;
  }
  else {
    $message['content']['subscription_info'] = array(
      '#value' => '<p>' . t('You have requested to be unsubscribed from all lists.') . '</p>',
    );
  }

  if ($show_items) {
    $items = array();
    $lists = emailkit_list_load_all();
    foreach ($lids as $lid) {
      $items[] = check_plain($lists[$lid]->name);
    }
    
    $message['content']['subscription_list'] = array(
      '#value' => theme('item_list', $items),
    );
  }
  
  $message['content']['confirmation_link'] = array(
    '#value' => '<p>' . l(t('Confirm your subscriptions'), emailkit_list_confirmation_url($address, $action, $lids)) . '</p>',
  );

  $rationale_replacements = array(
    '!site_name' => l(variable_get('site_name', 'Drupal'), '<front>'),
    '!contact_us' => l(t('contact us'), 'mailto:' . variable_get('site_mail', ini_get('sendmail_from'))),
  );
  
  $message['footer']['rationale'] = array(
    '#value' => '<p>' . t('You are receiving this one-time message because you have requested to update your subscriptions via our website !site_name. If you have not requested to update your subscriptions, please ignore this message or !contact_us to resolve the problem.', $rationale_replacements) . '</p>',
  );
  
  return $message;
}

/**
 * Generates an absolute URL that can be used to confirm subscription to the given lists.
 *
 * @param $address A subscriber's e-mail address.
 * @param $lids An array of list identifiers.
 *
 * @return An absolute URL.
 */
function emailkit_list_confirmation_url($address, $action, $lids) {
  $timestamp = time();

  $query = array(
    'address' => $address,
    'action' => $action,
    'timestamp' => $timestamp,
    'hash' => emailkit_list_confirmation_hash($address, $action, $lids, $timestamp),
  );

  // Empty arrays don't translate well into query strings
  if (!empty($lids)) {
    $query['lids'] = $lids;
  }

  return url('emailkit/list/confirm', array('query' => $query, 'absolute' => TRUE));
}

/**
 * Handles a confirmation URL.
 */
function emailkit_list_confirmation_page() {
  // Get the address and ensure it is a valid e-mail address
  $address = $_GET['address'];
  if (!valid_email_address($address)) {
    return drupal_access_denied();
  }
  
  // Get the action and ensure it is valid
  $action = $_GET['action'];
  if (!in_array($action, array('update_all', 'subscribe', 'unsubscribe'))) {
    return drupal_access_denied();
  }
  
  // Get the list identifiers and make sure they are all numeric
  $lids = isset($_GET['lids']) ? $_GET['lids'] : array();
  foreach ($lids as $lid) {
    if (!is_numeric($lid)) {
      return drupal_access_denied();
    }
  }
  
  // Get the timestamp and check whether it is numeric and does not lie in the future or too far in the past
  $timestamp = $_GET['timestamp'];
  if (!is_numeric($timestamp) || $timestamp > time()) {
    return drupal_access_denied();
  }
  else if ($timestamp < time() - EMAILKIT_LIST_CONFIRMATION_URL_TIMEOUT) {
    drupal_set_message(t('This URL has expired. Please retry to update your subscriptions.'), 'error');
    
    return drupal_goto();
  }
  
  // Get the hash and check whether it is valid, so that we know the URL has not been altered
  $hash = $_GET['hash'];
  if ($hash != emailkit_list_confirmation_hash($address, $action, $lids, $timestamp)) {
    return drupal_access_denied();
  }
  
  // Fetch or create a subscriber
  $subscriber = emailkit_list_subscriber_load_by_address($address, TRUE);
  
  if (isset($subscriber->sid)) {
    $subscriptions = emailkit_list_subscriber_subscription_load_all($subscriber->sid);
  }
  else {
    $subscriptions = array();
  }

  if ($action == 'subscribe') {
    $lids = array_merge($lids, array_keys($subscriptions));
  }
  else if ($action == 'unsubscribe') {
    $lids = array_diff(array_keys($subscriptions), $lids);
  }

  // Update the subscriptions, with confirmation
  $subscriptions = emailkit_list_subscriber_subscription_update_all($subscriptions, $lids, time());
  
  // Try to save the subscriber and its subscriptions
  if (emailkit_list_subscriber_save($subscriber) && emailkit_list_subscriber_subscription_save_all($subscriber->sid, $subscriptions)) {
    if (empty($lids)) {
      drupal_set_message(t('You have successfully been unsubscribed.'));
    }
    else {
      drupal_set_message(t('Your confirmation has been received and your subscriptions have been updated.'));
    }
  }
  else {
    drupal_set_message(t('Your confirmation has been received, but your subscriptions could not be updated. Please try again later.'), 'error');
  }
  
  return drupal_goto();
}

/**
 * Generates a salted hash based on the given parameters. Since the salt is secret, we can be relatively sure we are the only ones that can generate this hash.
 */
function emailkit_list_confirmation_hash($address, $action, $lids, $timestamp) {
  return md5($address . $action . '|' . implode('|', $lids) . '|' . $timestamp . drupal_get_private_key());
}

/**
 * Implementation of hook_destination_form() for the emailkit_list destination type.
 */
function emailkit_list_destination_form($destination_type) {
  $form = array();

  // Build an array of options
  $options = array();
  $lists = emailkit_list_load_all();
  foreach ($lists as $lid => $list) {
    $options[$lid] = check_plain($list->name);
  }
  
  if (empty($options)) {
    $form['lids'] = array(
      '#type' => 'item',
      '#title' => t('Lists'),
      '#value' => t('No e-mail lists available.'),
    );
  }
  else {
    $form['lids'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Lists'),
      '#options' => $options,
      '#default_value' => array(),
      '#description' => t('Select one or more lists to which to send the e-mail. If a subscriber is subscribed to more than one of the selected lists, then he/she will receive just a single e-mail.'),
      '#required' => TRUE,
    );
  }
  
  return $form;
}

/**
 * Implementation of hook_destination_form_submit() for the emailkit_list destination type.
 */
function emailkit_list_destination_form_submit(&$destination, $form_values) {
  $destination['#lids'] = array_keys(array_filter($form_values['lids']));
}

/**
 * Implementation of hook_dispatcher_send() for the emailkit_list dispatcher.
 */
function emailkit_list_dispatcher_send($dispatcher, $message, $destination) {
  $lids = $destination['#lids'];
  
  // Translate the emailkit_list destination to the more generic emailkit_default destination
  $destination = array(
    '#type' => 'emailkit_default',
    '#recipients' => array(),
  );

  foreach ($lids as $lid) {
    // Add all subscribers to the list of recipients
    $subscribers = emailkit_list_load_subscribers($lid);
    foreach ($subscribers as $subscriber) {
      $destination['#recipients'][] = array(
        'address' => $subscriber->address,
      );
    }
  }
  
  // Resend the message
  return emailkit_send($message, $destination);
}
