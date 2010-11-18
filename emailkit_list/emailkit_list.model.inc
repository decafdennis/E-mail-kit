<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Defines an e-mail list.
 */
class EmailkitList {
  
  // These correspond to the fields in the emailkit_list table
  private $lid;
  private $name;
  private $weight = 0;
  
  /**
   * Creates a new e-mail list that has the given name.
   *
   * @param $name The name of the e-mail list.
   *
   * @return An e-mail list.
   */
  public function __construct($name) {
    $this->setName($name);
  }
  
  /**
   * Creates and returns a e-mail list with the given database record.
   *
   * @param $record The record containing all parameters of the e-mail list.
   *
   * @return An e-mail list.
   */
  protected static function listWithRecord($record) {
    $list = new EmailkitList($record->name);
    $list->setId($record->lid);
    $list->setWeight($record->weight);
    return $list;
  }
  
  /**
   * Returns a flag indicating whether this e-mail list does not exist in the database yet.
   */
  public function isNew() {
    return $this->getId() == NULL;
  }

  /**
   * Returns the unique identifier of this e-mail list.
   */
  public function getId() {
    return $this->lid;
  }
  
  /**
   * Sets the unique identifier of this e-mail list.
   */
  private function setId($lid) {
    if ($lid != NULL && !is_numeric($lid)) {
      trigger_error(sprintf("List identifier must be NULL or a number: %d", $lid), E_USER_ERROR);
      return;
    }
    
    $this->lid = $lid;
  }
  
  /**
   * Returns the name of this e-mail list.
   */
  public function getName() {
    return $this->name;
  }
  
  /**
   * Sets the name of this e-mail list.
   */
  public function setName($name) {
    if (!is_string($name) || strlen($name) == 0) {
      trigger_error(sprintf("List name must be a non-empty string: %s", $name), E_USER_ERROR);
      return;
    }
    
    $this->name = $name;
  }

  /**
   * Returns the weight of this e-mail list.
   */
  public function getWeight() {
    return $this->weight;
  }

  /**
   * Sets the weight of this e-mail list.
   */
  public function setWeight($weight) {
    if (!is_numeric($weight)) {
      trigger_error(sprintf("List weight must be a number: %d", $weight), E_USER_ERROR);
      return;
    }
    
    $this->weight = $weight;
  }

  /**
   * Loads all e-mail lists from the database.
   *
   * @return An array of e-mail lists, keyed by their unique identifier and ordered by weight and name.
   */
  public static function loadAll() {
    $lists = array();
    
    $result = db_query("SELECT * FROM {emailkit_list} ORDER BY weight ASC, name ASC");
    while ($record = db_fetch_object($result)) {
      $list = EmailkitList::listWithRecord($record);
      $lists[$list->getId()] = $list;
    }
    
    return $lists;
  }

  /**
   * Load the e-mail list with the given identifier from the database.
   *
   * @return An e-mail list, or NULL if it cannot be found.
   */
  public static function loadWithId($lid) {
    if (!is_numeric($lid)) {
      trigger_error(sprintf("List identifier must be a number: %d", $lid), E_USER_ERROR);
      return NULL;
    }

    $result = db_query("SELECT * FROM {emailkit_list} WHERE lid = %d", $lid);
    if ($record = db_fetch_object($result)) {
      return EmailkitList::listWithRecord($record);
    }
    else {
      return NULL;
    }
  }

  /**
   * Deletes the e-mail list with the given identifier from the database.
   *
   * @return A flag indicating whether the e-mail list was deleted.
   */
  public static function deleteWithId($lid) {
    if (!is_numeric($lid)) {
      trigger_error(sprintf("List identifier must be a number: %d", $lid), E_USER_ERROR);
      return FALSE;
    }

    db_query("DELETE FROM {emailkit_list} WHERE lid = %d", $lid);
    return db_affected_rows() == 1;
  }

  /**
   * Saves this e-mail list to the database, inserting a new record or updating an existing record as necessary.
   *
   * @return Returns FALSE, SAVED_NEW or SAVED_UPDATED depending on whether saving was successful.
   */
  public function save() {
    if ($this->isNew()) {
      db_query("INSERT INTO {emailkit_list} (name, weight) VALUES ('%s', %d)", $this->getName(), $this->getWeight());
      $this->setId(db_last_insert_id('emailkit_list', 'lid'));
      
      $rows = db_affected_rows();
      if ($rows == 1) {
        return SAVED_NEW;
      }
      else {
        trigger_error(sprintf("Number of affected rows should be one: %d", $rows), E_USER_WARNING);
        return FALSE;
      }
    }
    else {
      db_query("UPDATE {emailkit_list} SET name = '%s', weight = %d WHERE lid = %d", $this->getName(), $this->getWeight(), $this->getId());

      $rows = db_affected_rows();
      if ($rows == 1) {
        return SAVED_UPDATED;
      }
      else {
        trigger_error(sprintf("Number of affected rows should be one: %d", $rows), E_USER_WARNING);
        return FALSE;
      }
    }
  }

  /**
   * Deletes this e-mail list from the database.
   *
   * @return A flag indicating whether the e-mail list was deleted.
   */
  public function delete() {
    db_query("DELETE FROM {emailkit_list} WHERE lid = %d", $this->getId());
    
    $rows = db_affected_rows();
    if ($rows == 1) {
      return TRUE;
    }
    else {
      trigger_error(sprintf("Number of affected rows should be one: %d", $rows), E_USER_WARNING);
      return FALSE;
    }
  }
  
}

/**
 * Defines a subscriber that can subscribe to any number of e-mail lists. A subscriber has an e-mail address and may or may not be associated with a user account.
 */
class EmailkitListSubscriber {

  // These correspond to the fields in the emailkit_list_subscriber table
  private $sid;
  private $address;
  private $uid = 0;

  /**
   * Creates a new subscriber with the given address and user ID.
   *
   * @param $address The e-mail address of the subscriber.
   * @param $uid The user ID or 0 if this subscriber is anonymous. (optional)
   *
   * @return A subscriber
   */
  public function __construct($address, $uid = 0) {
    $this->setAddress($address);
    $this->setUserID($uid);
  }
  
  /**
   * Creates and returns a subscriber with the given database record.
   *
   * @param $record The record containing all parameters of the subscriber.
   *
   * @return A subscriber.
   */
  protected static function subscriberWithRecord($record) {
    $list = new EmailkitListSubscriber($record->address, $record->uid);
    $list->setId($record->sid);
    return $list;
  }

  /**
   * Returns a flag indicating whether this subscriber does not exist in the database yet.
   */
  public function isNew() {
    return $this->getId() == NULL;
  }

  /**
   * Returns the unique identifier of this subscriber.
   */
  public function getId() {
    return $this->sid;
  }

  /**
   * Sets the unique identifier of this subscriber.
   */
  private function setId($sid) {
    if ($sid != NULL && !is_numeric($sid)) {
      trigger_error(sprintf("Subscriber identifier must be NULL or a number: %d", $sid), E_USER_ERROR);
      return;
    }
    
    $this->sid = $sid;
  }

  /**
   * Returns the e-mail address of this subscriber.
   */
  public function getAddress() {
    return $this->address;
  }

  /**
   * Sets the e-mail address of this subscriber.
   */
  public function setAddress($address) {
    if (!valid_email_address($address)) {
      trigger_error(sprintf("Subscriber address must be a valid e-mail address: %s", $address), E_USER_ERROR);
      return;
    }
    
    $this->address = $address;
  }

  /**
   * Returns the user ID of this subscriber.
   */
  public function getUserId() {
    return $this->uid;
  }

  /**
   * Sets the user ID of this subscriber.
   */
  public function setUserId($uid) {
    if (!is_numeric($uid)) {
      trigger_error(sprintf("Subscriber user ID must be a number: %d", $uid), E_USER_ERROR);
      return;
    }
    
    $this->uid = $uid;
  }

  /**
   * Returns the user of this subscribert, or NULL if this subscriber has no user ID.
   */
  public function getUser() {
    $user = user_load($this->getUserId());
    if ($user !== FALSE) {
      return $user;
    }
    else {
      return NULL;
    }
  }
  
  /**
   * Loads all subscribers from the database.
   *
   * @return An array of subscribers, keyed and ordered by their unique identifier.
   */
  public static function loadAll() {
    $subscribers = array();
    
    $result = db_query("SELECT * FROM {emailkit_list_subscriber} ORDER BY sid ASC");
    while ($record = db_fetch_object($result)) {
      $subscriber = EmailkitListSubscriber::subscriberWithRecord($record);
      $subscribers[$subscriber->getId()] = $subscriber;
    }
    
    return $subscribers;
  }

  /**
   * Load the subscriber with the given identifier from the database.
   *
   * @return A subscriber, or NULL if it cannot be found.
   */
  public static function loadWithId($sid) {
    if (!is_numeric($sid)) {
      trigger_error(sprintf("Subscriber identifier must be a number: %d", $sid), E_USER_ERROR);
      return;
    }

    $result = db_query("SELECT * FROM {emailkit_list_subscriber} WHERE sid = %d", $sid);
    if ($record = db_fetch_object($result)) {
      return EmailkitListSubscriber::subscriberWithRecord($record);
    }
    else {
      return NULL;
    }
  }
  
  /**
   * Load the subscriber associated with the given e-mail address from the database.
   *
   * @param $address A non-empty string.
   *
   * @return A subscriber, or NULL if it cannot be found.
   */
  public static function loadWithAddress($address) {
    if (!is_string($address) || empty($address)) {
      trigger_error(sprintf("E-mail address must be a non-empty string: %s", $address), E_USER_ERROR);
      return;
    }

    $result = db_query("SELECT * FROM {emailkit_list_subscriber} WHERE address = '%s'", $address);
    if ($record = db_fetch_object($result)) {
      return EmailkitListSubscriber::subscriberWithRecord($record);
    }
    else {
      return NULL;
    }
  }
  
  /**
   * Load the subscriber associated with the given user ID from the database.
   *
   * @return A subscriber, or NULL if it cannot be found.
   */
  public static function loadWithUserId($uid) {
    if (!is_numeric($uid)) {
      trigger_error(sprintf("User identifier must be a number: %d", $uid), E_USER_ERROR);
      return;
    }

    $result = db_query("SELECT * FROM {emailkit_list_subscriber} WHERE uid = %d", $uid);
    if ($record = db_fetch_object($result)) {
      return EmailkitListSubscriber::subscriberWithRecord($record);
    }
    else {
      return NULL;
    }
  }
  
  /**
   * Deletes the subscriber with the given identifier from the database.
   *
   * @return A flag indicating whether the subscriber was deleted.
   */
  public static function deleteWithId($sid) {
    if (!is_numeric($sid)) {
      trigger_error(sprintf("Subscriber identifier must be a number: %d", $sid), E_USER_ERROR);
      return FALSE;
    }

    db_query("DELETE FROM {emailkit_list_subscriber} WHERE sid = %d", $sid);
    return db_affected_rows() == 1;
  }

  /**
   * Saves this subscriber to the database, inserting a new record or updating an existing record as necessary.
   *
   * @return Returns FALSE, SAVED_NEW or SAVED_UPDATED depending on whether saving was successful.
   */
  public function save() {
    if ($this->isNew()) {
      db_query("INSERT INTO {emailkit_list_subscriber} (address, uid) VALUES ('%s', %d)", $this->getAddress(), $this->getUserId());
      $this->setId(db_last_insert_id('emailkit_list_subscriber', 'sid'));
      
      $rows = db_affected_rows();
      if ($rows == 1) {
        return SAVED_NEW;
      }
      else {
        trigger_error(sprintf("Number of affected rows should be one: %d", $rows), E_USER_WARNING);
        return FALSE;
      }
    }
    else {
      db_query("UPDATE {emailkit_list_subscriber} SET address = '%s', uid = %d WHERE sid = %d", $this->getAddress(), $this->getUserId(), $this->getId());

      $rows = db_affected_rows();
      if ($rows == 1) {
        return SAVED_UPDATED;
      }
      else {
        trigger_error(sprintf("Number of affected rows should be one: %d", $rows), E_USER_WARNING);
        return FALSE;
      }
    }
  }

  /**
   * Deletes this subscriber from the database.
   *
   * @return A flag indicating whether the subscriber was deleted.
   */
  public function delete() {
    db_query("DELETE FROM {emailkit_list_subscriber} WHERE sid = %d", $this->getId());
    
    $rows = db_affected_rows();
    if ($rows == 1) {
      return TRUE;
    }
    else {
      trigger_error(sprintf("Number of affected rows should be one: %d", $rows), E_USER_WARNING);
      return FALSE;
    }
  }

}
