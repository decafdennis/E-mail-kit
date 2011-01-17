<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns a new unsaved list with default values.
 */
function emailkit_list_new() {
  $list = new stdClass();
  $list->name = "";
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
    db_query("UPDATE {emailkit_list} SET name = '%s', weight = %d WHERE lid = %d", $list->name, $list->weight, $list->lid);
    
    if (db_affected_rows() == 1) {
      return SAVED_UPDATED;
    }
    else {
      return FALSE;
    }
  }
  else {
    db_query("INSERT INTO {emailkit_list} (name, weight) VALUES ('%s', %d)", $list->name, $list->weight);

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
  $subscriber->confirmed = FALSE;
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
 * Loads the subscriptions of the given subscriber.
 *
 * @param $sid The identifier of the subscriber.
 *
 * @return An array of list identifiers.
 */
function emailkit_list_subscriber_load_subscriptions($sid) {
  $lids = array();
  
  $result = db_query("SELECT lid FROM {emailkit_list_subscription} WHERE sid = %d", $sid);
  while ($lid = db_result($result)) {
    $lids[] = $lid;
  }
  
  return $lids;
}

/**
 * Saves the given subscriber to the database, inserting a new record or updating an existing record as necessary.
 *
 * @return Returns FALSE, SAVED_NEW or SAVED_UPDATED depending on whether saving was successful. Upon return, a new subscriber will automatically have an identifier assigned to it.
 */
function emailkit_list_subscriber_save(&$subscriber) {
  if (isset($subscriber->sid)) {
    db_query("UPDATE {emailkit_list_subscriber} SET address = '%s', confirmed = %d WHERE sid = %d", $subscriber->address, $subscriber->confirmed, $subscriber->sid);
    
    if (db_affected_rows() == 1) {
      return SAVED_UPDATED;
    }
    else {
      return FALSE;
    }
  }
  else {
    db_query("INSERT INTO {emailkit_list_subscriber} (address, confirmed) VALUES ('%s', %d)", $subscriber->address, $subscriber->confirmed);

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
 * Saves the subscriptions of the given subscriber. 
 *
 * @param $sid The identifier of the subscriber.
 * @param $lids An array of list identifiers.
 *
 * @return A flag indicating whether saving was successful.
 */
function emailkit_list_subscriber_save_subscriptions($sid, $lids) {
  db_query("DELETE FROM {emailkit_list_subscription} WHERE sid = %d", $sid);
  
  if (!empty($lids)) {
    // Start constructing a compound insertion statement
    $sql = "INSERT INTO {emailkit_list_subscription} (sid, lid) VALUES ";
    $sql_args = array();
    $insertions = 0;
    
    foreach ($lids as $lid) {
      $sql .= "(%d, %d), ";
      $sql_args[] = $sid;
      $sql_args[] = $lid;
      $insertions++;
    }
    
    // Remove the final comma delimiter
    $sql = substr($sql, 0, -2);
    
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
 * @param $lids An array of list identifiers.
 *
 * @return A flag indicating whether the message was sent successfully.
 */
function emailkit_list_confirmation_send($address, $lids) {
  $message = emailkit_message('emailkit_list_confirmation', $address, $lids);

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
function emailkit_list_confirmation_message($message_id, $address, $lids) {
  return array(
    '#subject' => t('Confirm your subscriptions'),
    '#body' => array(
      'link' => array(
        '#value' => emailkit_list_confirmation_url($address, $lids),
      ),
    ),
  );
}

/**
 * Generates an absolute URL that can be used to confirm subscription to the given lists.
 *
 * @param $address A subscriber's e-mail address.
 * @param $lids An array of list identifiers.
 *
 * @return An absolute URL.
 */
function emailkit_list_confirmation_url($address, $lids) {
  $timestamp = time();

  $query = array(
    'address' => $address,
    'timestamp' => $timestamp,
    'hash' => emailkit_list_confirmation_hash($address, $lids, $timestamp),
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
  if ($hash != emailkit_list_confirmation_hash($address, $lids, $timestamp)) {
    return drupal_access_denied();
  }
  
  // Check if there is an existing subscriber
  $subscriber = emailkit_list_subscriber_load_by_address($address);
  
  if (isset($subscriber)) {
    if (!$subscriber->confirmed) {
      $subscriber->confirmed = TRUE;
      emailkit_list_subscriber_save($subscriber);
    }
  }
  else {
    // Create a new one
    $subscriber = emailkit_list_subscriber_new($address);
    $subscriber->confirmed = TRUE;
    
    // If we were unable to save the subscriber, set it to NULL so we can check this condition later
    if (!emailkit_list_subscriber_save($subscriber)) {
      unset($subscriber);
    }
  }
  
  // Check whether we have a subscriber and try to save the subscriptions
  if (isset($subscriber) && emailkit_list_subscriber_save_subscriptions($subscriber->sid, $lids)) {
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
function emailkit_list_confirmation_hash($address, $lids, $timestamp) {
  return md5($address . implode(',', $lids) . $timestamp . drupal_get_private_key());
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
    $options[$lid] = $list->name;
  }

  $form['lids'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Lists'),
    '#options' => $options,
    '#default_value' => array(),
    '#description' => t('Select one or more lists to which to send the e-mail. If a subscriber is subscribed to more than one of the selected lists, then he/she will receive just a single e-mail.'),
    '#required' => TRUE,
  );
  
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
