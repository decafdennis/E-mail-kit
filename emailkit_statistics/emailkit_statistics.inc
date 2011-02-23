<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains code that can be loaded on-demand.
 */

/**
 * Generates an absolute URL that can be used to keep statistics on a message.
 *
 * @return An absolute URL.
 */
function emailkit_statistics_url($query) {
  // TODO: Include some kind of hash to prevent tampering with the URL
  return url('emailkit/statistics', array('query' => $query, 'absolute' => TRUE));
}

/**
 * Callback for the URL used for keeping statistics on a message.
 */
function emailkit_statistics_page() {
  // Get and validate the tracking id
  $tracking_id = isset($_GET['tracking_id']) ? $_GET['tracking_id'] : NULL;
  if (!isset($tracking_id) || strlen($tracking_id) != 32) {
    return drupal_not_found();
  }

  // Get and validate the recipient id
  $recipient_id = isset($_GET['recipient_id']) ? $_GET['recipient_id'] : NULL;
  if (isset($recipient_id) && strlen($recipient_id) != 32) {
    return drupal_not_found();
  }

  // Get and process the type
  $type = isset($_GET['type']) ? $_GET['type'] : NULL;
  switch ($type) {
    case 'click':
      // Get and validate the URL
      $url = isset($_GET['url']) ? $_GET['url'] : '';
      if (empty($url)) {
        return drupal_not_found();
      }

      // Tally the click
      $link_id = md5($url);
      if (isset($recipient_id)) {
        db_query("INSERT IGNORE INTO {emailkit_statistics_message_recipient_link} (tracking_id, recipient_id, link_id) VALUES ('%s', '%s', '%s')", $tracking_id, $recipient_id, $link_id);
        db_query("UPDATE {emailkit_statistics_message_recipient_link} SET hits = hits + 1 WHERE tracking_id = '%s' AND recipient_id = '%s' AND link_id = '%s'", $tracking_id, $recipient_id, $link_id);
      }
      else {
        db_query("UPDATE {emailkit_statistics_message_link} SET hits = hits + 1 WHERE tracking_id = '%s' AND link_id = '%s'", $tracking_id, $link_id);
      }

      // Redirect to the URL, preventing header injection attacks
      $url = str_replace(array("\n", "\r"), '', $url);
      header('Location: ' . $url, TRUE);
      exit();
      return;
    
    case 'view':
      // Tally the view
      if (isset($recipient_id)) {
        db_query("UPDATE {emailkit_statistics_message_recipient} SET hits = hits + 1 WHERE tracking_id = '%s' AND recipient_id = '%s'", $tracking_id, $recipient_id);
      }
      else {
        db_query("UPDATE {emailkit_statistics_message} SET hits = hits + 1 WHERE tracking_id = '%s'", $tracking_id);
      }

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
  // This global array keeps track of whether recipient tracking is enabled for a message
  global $emailkit_statistics_message_recipient;

  // This global array keeps track of links replaced during message rendering
  global $emailkit_statistics_message_link;
  
  // Get the unique identifier of the message
  $tracking_id = $message['#tracking_id'];

  // Do this only once, i.e. in the first invocation of emailkit_send()
  if ($message['#sending_depth'] == 0) {
    // Initialize the global arrays
    if (!isset($emailkit_statistics_message_recipient)) {
      $emailkit_statistics_message_recipient = array();
    }
    if (!isset($emailkit_statistics_message_link)) {
      $emailkit_statistics_message_link = array();
    }

    // Initialize the global arrays for this particular message
    $emailkit_statistics_message_recipient[$tracking_id] = FALSE;
    $emailkit_statistics_message_link[$tracking_id] = array();
  }
  
  // Is this a known destination, i.e. is this a destination we can work with?
  if ($destination['#type'] == 'emailkit_default') {
    // Enable recipient tracking
    $emailkit_statistics_message_recipient[$tracking_id] = TRUE;

    // Add tokens to the recipients
    foreach ($destination['#recipients'] as $key => $recipient) {
      $destination['#recipients'][$key]['emailkit_statistics_id'] = md5($recipient['address']);
    }
  }
  else {
    // Disable recipient tracking
    // Note: this implies that recipient tracking is only enabled if the final destination is a known destination
    $emailkit_statistics_message_recipient[$tracking_id] = FALSE;
  }

  // Add an image that will be requested when the message is viewed
  $view_query = array(
    'type' => 'view',
    'tracking_id' => $tracking_id,
  );

  // Add information about the recipient if recipient tracking is enabled
  if ($emailkit_statistics_message_recipient[$tracking_id]) {
    $view_query['recipient_id'] = '[recipient:emailkit_statistics_id]';
  }

  $message['emailkit_statistics_view'] = array(
    '#value' => '<img src="' . check_plain(emailkit_statistics_url($view_query)) . '" alt="" />',
  );
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_render() that prepares the message for statistics.
 */
function _emailkit_statistics_message_render($message, $format, &$output) {
  // These global variables are used for passing arguments to _emailkit_statistics_message_render_url_rewrite()
  global $emailkit_statistics_message_render_message, $emailkit_statistics_message_render_format;
  
  // Argument passing by global variables, bah!
  $emailkit_statistics_message_render_message = $message;
  $emailkit_statistics_message_render_format = $format;
  
  // Build the pattern for matching URLs
  $protocol = "https?://";
  $authentication = "[a-zA-Z0-9:%_+*~#?&=.,/;-]+@";
  $ip = "(?:[0-9]{1,3}\.){3}[0-9]{1,3}";
  $domain = "(?:[A-Za-z0-9._+-]+\.)*[A-Za-z0-9._+-]+";
  $port = ":\d{1,5}";
  $path = "[a-zA-Z0-9:%_+*~#&\[\]=/;?\.,-]*[a-zA-Z0-9:%_+*~#&\[\]=/;-]";
  $pattern = "^$protocol(?:$authentication)?(?:$ip|$domain)(?:$port)?/?$path^";

  $output = preg_replace_callback($pattern, '_emailkit_statistics_message_render_url_rewrite', $output);
}

/**
 * Callback for preg_replace_callback() used in _emailkit_statistics_message_render().
 */
function _emailkit_statistics_message_render_url_rewrite($match) {
  // This global array keeps track of whether recipient tracking is enabled for a message
  global $emailkit_statistics_message_recipient;

  // This global array keeps track of links replaced during message rendering
  global $emailkit_statistics_message_link;
  
  // These global variables are used for passing arguments from _emailkit_statistics_message_render()
  global $emailkit_statistics_message_render_message, $emailkit_statistics_message_render_format;
  
  // Argument passing by global variables, bah!
  $message = $emailkit_statistics_message_render_message;
  $format = $emailkit_statistics_message_render_format;
  
  // Decode the URL
  $url = $match[0];
  if ($format == EMAILKIT_MESSAGE_FORMAT_HTML) {
    $url = htmlspecialchars_decode($url);
  }
  
  // Rewrite the URL, but only if it is not an emailkit URL
  if (strpos($url, url('emailkit', array('absolute' => TRUE))) === FALSE) {
    // Add this URL to the global link array
    $tracking_id = $message['#tracking_id'];
    $link_id = md5($url);
    $emailkit_statistics_message_link[$tracking_id][$link_id] = array(
      'url' => $url,
    );
    
    // Rewrite the URL
    $click_query = array(
      'type' => 'click',
      'url' => $url,
      'tracking_id' => $tracking_id,
    );

    // Add information about the recipient if recipient tracking is enabled
    if ($emailkit_statistics_message_recipient[$tracking_id]) {
      $click_query['recipient_id'] = '[recipient:emailkit_statistics_id]';
    }

    $url = emailkit_statistics_url($click_query);
  }
  
  // Encode the URL
  if ($format == EMAILKIT_MESSAGE_FORMAT_HTML) {
    $url = htmlspecialchars($url);
  }
  
  return $url;
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_after_send() that prepares the message for statistics.
 */
function _emailkit_statistics_message_after_send($message, $destination, $success) {
  // This static variable keeps track of which messages have been processed by this function already
  static $processed = array();
  
  // Get the unique identifier of the message
  $tracking_id = $message['#tracking_id'];
  
  // Only process the message if it hasn't been processed yet and if this is a known destination or our last chance at processing the message
  if (!in_array($tracking_id, $processed) && ($destination['#type'] == 'emailkit_default' || $message['#sending_depth'] == 0)) {
    // This global array keeps track of links replaced during message rendering
    global $emailkit_statistics_message_link;

    // Was the message sent successfully?
    if ($success) {
      // Add the message to the database
      db_query("INSERT INTO {emailkit_statistics_message} (tracking_id) VALUES ('%s')", $tracking_id);

      // Add all links in the message to the database
      foreach ($emailkit_statistics_message_link[$tracking_id] as $link_id => $link) {
        db_query("INSERT INTO {emailkit_statistics_message_link} (tracking_id, link_id, url) VALUES ('%s', '%s', '%s')", $tracking_id, $link_id, $link['url']);
      }

      // Add all recipients of the message to the database, if applicable
      if ($destination['#type'] == 'emailkit_default') {
        foreach ($destination['#recipients'] as $recipient) {
          db_query("INSERT INTO {emailkit_statistics_message_recipient} (tracking_id, recipient_id, address) VALUES ('%s', '%s', '%s')", $tracking_id, $recipient['emailkit_statistics_id'], $recipient['address']);
        }
      }
    }

    // Clear out the global link array for this particular message
    unset($emailkit_statistics_message_link[$tracking_id]);
  
    // Mark this message as having been processed
    $processed[] = $tracking_id;
  }
}

/**
 * Private helper function for emailkit_statistics_message_page() that returns a page that displays statistics for a message.
 */
function _emailkit_statistics_message_page($tracking_id) {
  $message_stats = db_fetch_object(db_query("SELECT * FROM {emailkit_statistics_message} WHERE tracking_id = '%s'", $tracking_id));
  if (!isset($message_stats)) {
    return;
  }
  
  // TODO: Make the output prettier
  $output = '<pre>';
  
  $output .= "Number of hits: $message_stats->hits\n";

  $result = db_query("SELECT * FROM {emailkit_statistics_message_link} WHERE tracking_id = '%s'", $tracking_id);
  while ($link_stats = db_fetch_object($result)) {
    $output .= "\n";
    $output .= check_plain($link_stats->url) . "\n";
    $output .= "\tNumber of clicks: $link_stats->hits\n";
  }
  
  $output .= '</pre>';
  return $output;
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
