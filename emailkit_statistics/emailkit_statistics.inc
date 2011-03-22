<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains code that can be loaded on-demand.
 */

/**
 * The length of an event identifier. This is the length of a hexadecimal MD5 hash.
 */
define('EMAILKIT_STATISTICS_EID_LENGTH', 32);

/**
 * The length of a recipient identifier. This is the length of a hexadecimal MD5 hash.
 */
define('EMAILKIT_STATISTICS_RID_LENGTH', 32);

/**
 * Defines the index of the prefix capture in the match passed into _emailkit_statistics_message_render_url_rewrite().
 */
define('EMAILKIT_STATISTICS_CAPTURE_PREFIX', 1);

/**
 * Defines the index of the attribute name capture in the match passed into _emailkit_statistics_message_render_url_rewrite().
 */
define('EMAILKIT_STATISTICS_CAPTURE_ATTRIBUTE', 2);

/**
 * Defines the index of the URL capture in the match passed into _emailkit_statistics_message_render_url_rewrite().
 */
define('EMAILKIT_STATISTICS_CAPTURE_URL', 3);

/**
 * Generates an absolute URL that can be used to keep statistics on a message.
 *
 * @param $eid The event identifier. Required.
 * @param $rid The recipient identifier. Optional.
 */
function emailkit_statistics_url($eid, $rid = NULL) {
  $query = array();

  // Add the event identifier to the query string
  $query['e'] = $eid;

  // Add the recipient identifier to the query string, if any
  if (isset($rid)) {
    $query['r'] = $rid;
  }

  // Add a hash to the query string to prevent tampering
  if (isset($hash)) {
    // We were given a hash, which is probably a token, so use that
    $query['h'] = $hash;
  }
  else {
    // No hash was given, so generate one
    $query['h'] = emailkit_statistics_hash($eid, $rid);
  }

  return _emailkit_statistics_url($query);
}

/**
 * Generates an absolute URL that can be used to keep statistics on a message for a recipient.
 *
 * @param $eid The event identifier. Required.
 * @param $rid_token_name The name of the recipient token to use for the recipient identifier. Required.
 * @param $hash_token_name The name of the recipient token to use for the hash. Required.
 */
function emailkit_statistics_url_recipient_token($eid, $rid_token_name, $hash_token_name) {
  $query = array();

  // Add the event identifier to the query string
  $query['e'] = $eid;

  // Add the recipient identifier token to the query string
  $query['r'] = "[recipient:$rid_token_name]";

  // Add the hash token to the query string
  $query['h'] = "[recipient:$hash_token_name]";

  return _emailkit_statistics_url($query);
}

/**
 * Private helper for emailkit_statistics_url(). and emailkit_statistics_url_recipient_token().
 */
function _emailkit_statistics_url($query) {
  return url('emailkit/s', array('query' => $query, 'absolute' => TRUE));
}

/**
 * Generates a salted hash based on the given parameters. Since the salt is secret, we can be relatively sure we are the only ones that can generate this hash.
 *
 * @param $eid The event identifier. Required.
 * @param $rid The recipient identifier or the token to use instead. Optional.
 */
function emailkit_statistics_hash($eid, $rid = NULL) {
  // Add the event identifier to the hash input
  $input = $eid;

  // Add the recipient identifier to the hash input, if any
  if (isset($rid)) {
    $input .= $rid;
  }

  // Add the secret salt to the hash input
  $input .= drupal_get_private_key();

  // Return the first 8 characters of the hexadecimal MD5 hash of the input
  return substr(md5($input), 0, 8);
}

/**
 * Callback for the URL used for keeping statistics on a message.
 */
function emailkit_statistics_page() {
  // Get and validate the event identifier
  $eid = isset($_GET['e']) ? $_GET['e'] : NULL;
  if (!isset($eid) || strlen($eid) != EMAILKIT_STATISTICS_EID_LENGTH) {
    return drupal_not_found();
  }

  // Get and validate the recipient identifier
  $rid = isset($_GET['r']) ? $_GET['r'] : NULL;
  if (isset($rid) && strlen($rid) != EMAILKIT_STATISTICS_RID_LENGTH) {
    return drupal_not_found();
  }

  // Get and validate the hash
  $hash = isset($_GET['h']) ? $_GET['h'] : NULL;
  if (!isset($hash) || $hash != emailkit_statistics_hash($eid, $rid)) {
    return drupal_not_found();
  }

  // Try to load the event from the database
  $event = db_fetch_object(db_query("SELECT token, type, url FROM {emailkit_statistics_event} WHERE eid = '%s'", $eid));
  if (!isset($event)) {
    return drupal_not_found();
  }

  // If this recipient hasn't triggered the view event yet, the click event will trigger it implicitly
  if ($event->type == 'click') {
    // Find the view event for this message
    $view_event_eid = db_result(db_query("SELECT eid FROM {emailkit_statistics_event} WHERE token = '%s' AND type = 'view'", $event->token));

    if (!empty($view_event_eid)) {
      // Find out whether the recipient has already triggered this event
      $view_log_timestamp = NULL;
      if (isset($rid)) {
        $view_log_timestamp = db_result(db_query("SELECT timestamp FROM {emailkit_statistics_log} WHERE eid = '%s' AND rid = '%s'", $view_event_eid, $rid));
      }
      else {
        $view_log_timestamp = db_result(db_query("SELECT timestamp FROM {emailkit_statistics_log} WHERE eid = '%s'", $view_event_eid));
      }

      // Trigger the view event if it hasn't been triggerd yet
      if (empty($view_log_timestamp)) {
        _emailkit_statistics_event_log($view_event_eid, $rid);
      }
    }
  }

  // Log the event
  _emailkit_statistics_event_log($eid, $rid);

  // Execute the event
  return _emailkit_statistics_event_execute($event);
}

/**
 * Logs the occurance of the given event for the given recipient.
 *
 * @param $eid The event identifier.
 * @param $rid The recipient identifier.
 */
function _emailkit_statistics_event_log($eid, $rid) {
  // Log the occurance of this event to the database
  $hostname = ip_address();
  $timestamp = time();
  if (isset($rid)) {
    db_query("INSERT INTO {emailkit_statistics_log} (eid, rid, hostname, timestamp) VALUES ('%s', '%s', '%s', %d)", $eid, $rid, $hostname, $timestamp);
  }
  else {
    db_query("INSERT INTO {emailkit_statistics_log} (eid, hostname, timestamp) VALUES ('%s', '%s', %d)", $eid, $hostname, $timestamp);
  }
}

/**
 * Executes the given event.
 *
 * @param $event The event.
 */
function _emailkit_statistics_event_execute($event) {
  // Execute the event
  switch ($event->type) {
    case 'click':
      // Redirect to the URL
      header('Location: ' . $event->url, TRUE);
      exit();
      return;

    case 'view':
      // Output a random image, suppressing any devel output
      $GLOBALS['devel_shutdown'] = FALSE;
      print(_emailkit_statistics_random_image(TRUE));
      exit();
      return;

    default:
      return drupal_not_found();
  }
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_before_send() that prepares the message for statistics.
 */
function _emailkit_statistics_message_before_send(&$message, &$destination) {
  // This global array keeps track of events generated during message sending
  global $emailkit_statistics_message_event;

  // Get the unique identifier of the message
  $token = $message['#token'];

  // Is this the first time we are invoked?
  if ($message['#sending_depth'] == 0) {
    // Initialize the global event array
    if (!isset($emailkit_statistics_message_event)) {
      $emailkit_statistics_message_event = array();
    }
    $emailkit_statistics_message_event[$token] = array();
  }
  
  // Add an event for viewing the message to the global event array
  $eid = md5($token . 'view');
  $emailkit_statistics_message_event[$token][$eid] = array(
    'type' => 'view',
  );

  // Is this a destination that supports recipient token replacement?
  if ($destination['#type'] == 'emailkit_default') {
    $rid_token_name = 'emailkit_statistics_rid';
    $hash_token_name = 'emailkit_statistics_hash_' . $eid;

    foreach ($destination['#recipients'] as $key => $recipient) {
      // Generate a recipient identifier for this recipient
      $rid = md5($recipient['address']);
      $destination['#recipients'][$key][$rid_token_name] = $rid;

      // Also generate a hash for the message viewing event for this recipient
      $hash = emailkit_statistics_hash($eid, $rid);
      $destination['#recipients'][$key][$hash_token_name] = $hash;
    }

    $view_image_url = emailkit_statistics_url_recipient_token($eid, $rid_token_name, $hash_token_name);
  }
  else {
    $view_image_url = emailkit_statistics_url($eid);
  }

  // Trigger the message viewing event using a hidden image
  $message['emailkit_statistics_view'] = array(
    '#value' => '<img style="visibility:hidden" src="' . check_plain($view_image_url) . '" alt="" />',
  );
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_render() that prepares the message for statistics.
 */
function _emailkit_statistics_message_render($message, $format, &$body, &$destination = NULL) {
  // Build the pattern for matching URLs
  $protocol = "https?://";
  $authentication = "[a-zA-Z0-9:%_+*~#?&=.,/;-]+@";
  $ip = "(?:[0-9]{1,3}\.){3}[0-9]{1,3}";
  $domain = "(?:[A-Za-z0-9._+-]+\.)*[A-Za-z0-9._+-]+";
  $port = ":\d{1,5}";
  $path = "[a-zA-Z0-9:%_+*~#&\[\]=/;?\.,-]*[a-zA-Z0-9:%_+*~#&\[\]=/;-]";
  $pattern = "^((?:(href|src)=[\"']\\s*)?)($protocol(?:$authentication)?(?:$ip|$domain)(?:$port)?/?$path)^";

  // Pass arguments to _emailkit_statistics_message_render_url_rewrite() using global variables (ouch!)
  $GLOBALS['emailkit_statistics_message_render_message'] = $message;
  $GLOBALS['emailkit_statistics_message_render_format'] = $format;
  $GLOBALS['emailkit_statistics_message_render_destination'] =& $destination;

  $body = preg_replace_callback($pattern, '_emailkit_statistics_message_render_url_rewrite', $body);
}

/**
 * Callback for preg_replace_callback() used in _emailkit_statistics_message_render().
 */
function _emailkit_statistics_message_render_url_rewrite($match) {
  // Get arguments passed from _emailkit_statistics_message_render() using global variables (ouch!)
  $message = $GLOBALS['emailkit_statistics_message_render_message'];
  $format = $GLOBALS['emailkit_statistics_message_render_format'];
  $destination =& $GLOBALS['emailkit_statistics_message_render_destination'];

  // Decode the URL and get the attribute
  $prefix = $match[EMAILKIT_STATISTICS_CAPTURE_PREFIX];
  $attribute = $match[EMAILKIT_STATISTICS_CAPTURE_ATTRIBUTE];
  $url = $match[EMAILKIT_STATISTICS_CAPTURE_URL];
  if ($format == EMAILKIT_MESSAGE_FORMAT_HTML) {
    $url = htmlspecialchars_decode($url);
  }
  
  // Rewrite the URL, but only if it is not a resource or an emailkit URL
  $emailkit_base_url = url('emailkit', array('absolute' => TRUE));
  if ($attribute != 'src' && strpos($url, $emailkit_base_url) === FALSE) {
    // This global array keeps track of events generated during message sending
    global $emailkit_statistics_message_event;
    
    // Get the unique identifier of the message
    $token = $message['#token'];

    // Add an event for clicking this link to the global event array
    $eid = md5($token . 'click' . $url);
    $emailkit_statistics_message_event[$token][$eid] = array(
      'type' => 'click',
      'url' => $url,
    );

    // Is this message going to be sent to a destination that supports recipient token replacement?
    if (isset($destination) && $destination['#type'] == 'emailkit_default') {
      $rid_token_name = 'emailkit_statistics_rid';
      $hash_token_name = 'emailkit_statistics_hash_' . $eid;

      foreach ($destination['#recipients'] as $key => $recipient) {
        // Get the recipient identifier of this recipient
        $rid = $destination['#recipients'][$key][$rid_token_name];

        // Generate a hash for the event for this recipient
        $hash = emailkit_statistics_hash($eid, $rid);
        $destination['#recipients'][$key][$hash_token_name] = $hash;
      }

      $url = emailkit_statistics_url_recipient_token($eid, $rid_token_name, $hash_token_name);
    }
    else {
      $url = emailkit_statistics_url($eid);
    }
  }
  
  // Encode the URL
  if ($format == EMAILKIT_MESSAGE_FORMAT_HTML) {
    $url = htmlspecialchars($url);
  }
  
  return $prefix . $url;
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_after_send() that prepares the message for statistics.
 */
function _emailkit_statistics_message_after_send($message, $destination, $success) {
  // This static variable keeps track of which messages have been processed by this function already
  static $processed = array();
  
  // Get the unique identifier of the message
  $token = $message['#token'];
  
  // Only process the message if it hasn't been processed yet, and if this is a destination that supports recipient token replacement or if this is our last chance at processing the message
  if (!in_array($token, $processed) && ($destination['#type'] == 'emailkit_default' || $message['#sending_depth'] == 0)) {
    // This global array keeps track of events generated during message sending
    global $emailkit_statistics_message_event;

    // Was the message sent successfully?
    if ($success) {
      // Add an event to the database that we will use to indicate when and to whom the message was sent
      $send_eid = md5($token . 'send');
      db_query("INSERT INTO {emailkit_statistics_event} (eid, token, type) VALUES ('%s', '%s', '%s')", $send_eid, $token, 'send');

      // Add all other events to the database
      foreach ($emailkit_statistics_message_event[$token] as $eid => $event) {
        if (isset($event['url'])) {
          db_query("INSERT INTO {emailkit_statistics_event} (eid, token, type, url) VALUES ('%s', '%s', '%s', '%s')", $eid, $token, $event['type'], $event['url']);
        }
        else {
          db_query("INSERT INTO {emailkit_statistics_event} (eid, token, type) VALUES ('%s', '%s', '%s')", $eid, $token, $event['type']);
        }
      }

      // Add all recipients to the database, if necessary, and trigger the sending event
      $hostname = ip_address();
      $timestamp = time();
      if ($destination['#type'] == 'emailkit_default') {
        foreach ($destination['#recipients'] as $recipient) {
          $rid = $recipient['emailkit_statistics_rid'];
          db_query("INSERT IGNORE INTO {emailkit_statistics_recipient} (rid, address) VALUES ('%s', '%s')", $rid, $recipient['address']);
          db_query("INSERT INTO {emailkit_statistics_log} (eid, rid, hostname, timestamp) VALUES ('%s', '%s', '%s', %d)", $send_eid, $rid, $hostname, $timestamp);
        }
      }
      else {
        db_query("INSERT INTO {emailkit_statistics_log} (eid, hostname, timestamp) VALUES ('%s', '%s', %d)", $eid, $hostname, $timestamp);
      }
    }

    // Clear out the global event array for this particular message
    unset($emailkit_statistics_message_event[$token]);
  
    // Mark this message as having been processed
    $processed[] = $token;
  }
}

/**
 * Private helper function for emailkit_statistics_message_page() that returns a page that displays statistics for a message.
 */
function _emailkit_statistics_message_page($token) {
  // Fetch all events for the message, counting the number times an event has been triggerd and the number of unique recipients that have triggered each event
  $events = array();
  $result = db_query("SELECT e.eid, e.type, e.url, COUNT(l.rid) AS hits, COUNT(DISTINCT l.rid) AS recipients FROM {emailkit_statistics_event} e LEFT JOIN {emailkit_statistics_log} l ON e.eid = l.eid WHERE e.token = '%s' GROUP BY e.eid", $token);
  while ($event = db_fetch_object($result)) {
    if (!isset($events[$event->type])) {
      $events[$event->type] = array();
    }
    
    $events[$event->type][$event->eid] = $event;
  }

  $header = array(t('Description'), t('Value'));
  $rows = array();

  // Add a row that indicates the total number of recipients
  $sent = 0;
  foreach ($events['send'] as $event) {
    $sent = $event->hits;
  }
  $rows[] = array(
    t('Number of recipients'),
    sprintf("%d", $sent),
  );

  // To prevent division by zero
  if ($sent == 0) {
    $sent = 1;
  }

  // Add rows indicating how many times the message has been viewed
  $total_views = 0;
  $unique_views = 0;
  foreach ($events['view'] as $event) {
    $total_views = (int)$event->hits;
    $unique_views = (int)$event->recipients;
  }
  $rows[] = array(
    t('Recipients that have viewed the message'),
    sprintf("%d (%.1f%%)", $unique_views, $unique_views / $sent * 100.0),
  );
  $rows[] = array(
    t('Total number of times viewed'),
    sprintf("%d", $total_views),
  );

  // Add rows indicating how many times links have been clicked
  foreach ($events['click'] as $event) {
    $total_clicks = (int)$event->hits;
    $unique_clicks = (int)$event->recipients;
    
    $rows[] = array(
      array(
        'data' => l($event->url, $event->url),
        'colspan' => _emailkit_table_colspan($header),
      ),
    );
    
    $rows[] = array(
      t('Recipients that have clicked on the link'),
      sprintf("%d (%.1f%%)", $unique_clicks, $unique_clicks / $sent * 100.0),
    );
    $rows[] = array(
      t('Total number of clicks on the link'),
      sprintf("%d", $total_clicks),
    );
  }

  // TODO: Use a theme function?
  return theme('table', $header, $rows);
}

/**
 * Generates a small random image and returns the data. Currently, this function generates a transparent GIF file of random size.
 *
 * @see http://www.w3.org/Graphics/GIF/spec-gif89a.txt
 *
 * @param $headers Whether to set response headers suitable to the image data. Currently, this sets the Content-Type to image/gif.
 *
 * @return Image data.
 */
function _emailkit_statistics_random_image($headers = FALSE) {
  if ($headers) {
    header('Content-Type: image/gif');
  }
  
  // Generate a random width and height so that we look less suspicious to spam filters/virus scanners?
  // Note: the maximum possible size should be constrained so that ($width * $height) * 6 + 3 <= 255 * 8
  $width = mt_rand(2, 32);
  $height = mt_rand(1, 2);
  
  // Output header
  $output = "GIF89a";
  
  // Output logical screen descriptor
  $output .= pack("v2C3", $width, $height, 0x80, 0x00, 0x00);
  
  // Output global color table
  $output .= pack("C3", 0xFF, 0xFF, 0xFF);
  $output .= pack("C3", 0x00, 0x00, 0x00);
  
  // Output graphics control extension
  $output .= pack("C4vC2", 0x21, 0xF9, 0x04, 0x01, 0, 0x00, 0x00);
  
  // Output image descriptor
  $output .= pack("Cv4C", 0x2C, 0, 0, $width, $height, 0x00);
  
  // Compile image data
  $bytes = "";
  $bytes_length = 0;
  $pending_byte = 0;
  $pending_byte_offset = 0;
  for ($i = 0; $i < $width * $height; $i++) {
    // The byte 0x04 trimmed to 6 bits equals 0b000100, where 0b100 is a clear code and 0b000 is the color with index zero
    // Note: we use a clear code before each pixel code, so that we don't need to implement LZW compression
    _emailkit_statistics_append_bits($bytes, $bytes_length, 0x04, 6, $pending_byte, $pending_byte_offset);
  }
  
  // The byte 0x05 trimmed to three bits equals 0b101, which is an end of information code
  _emailkit_statistics_append_bits($bytes, $bytes_length, 0x05, 3, $pending_byte, $pending_byte_offset);
  
  // Terminate the bit string, see _emailkit_statistics_append_bits()
  _emailkit_statistics_append_bits($bytes, $bytes_length, NULL, 0, $pending_byte, $pending_byte_offset);

  // Output image data
  $output .= pack("CC", 2, $bytes_length) . $bytes . "\x00";

  // Output trailer
  $output .= pack("c", 0x3B);
  
  return $output;
}

/**
 * Appends at most 8 bits to a bit string.
 *
 * @param $bytes The bit string.
 * @param $bytes_lenth The length of the bit string in whole bytes.
 * @param $bits The bits to append. If this is NULL, then the pending byte will be written to the bit string to finish it off.
 * @param $bits_length The number of bits to append. Must be less than or equal to 8.
 * @param $pending_byte Used to store bits that do not form a whole byte and have not been added to the bit string. Initially, this should be 0.
 * @param $pending_byte_offset Used to store the number of bits in the pending byte. Initially, this should be 0.
 */
function _emailkit_statistics_append_bits(&$bytes, &$bytes_length, $bits, $bits_length, &$pending_byte, &$pending_byte_offset) {
  // Append the bits to the pending byte
  if ($bits !== NULL && $bits_length > 0) {
    $pending_byte |= $bits << $pending_byte_offset;
    $pending_byte_offset += $bits_length;
  }

  // If the pending byte has 8 or more bits append it to the bit string
  if ($bits === NULL || $pending_byte_offset >= 8) {
    $bytes .= pack("C", $pending_byte);
    $bytes_length++;
    
    $pending_byte = $pending_byte >> 8;
    $pending_byte_offset = $pending_byte_offset - 8;
  }
}
