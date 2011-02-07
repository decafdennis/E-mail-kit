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
  // TODO: Include some kind of hash to prevent tampering with the URL?
  return url('emailkit/statistics', array('query' => $query, 'absolute' => TRUE));
}

/**
 * Callback for the URL used for keeping statistics on a message.
 */
function emailkit_statistics_page() {
  $type = isset($_GET['type']) ? $_GET['type'] : NULL;
  
  switch ($type) {
    case 'click':
      $url = isset($_GET['url']) ? $_GET['url'] : '';

      if (empty($url)) {
        drupal_goto();
      }
      else {
        // TODO: Do something more useful
        watchdog('emailkit_statistics', t('A link in a message has been clicked.'));
        
        // Redirect to the URL, prevent header injection attacks
        $url = str_replace(array("\n", "\r"), '', $url);
        header('Location: ' . $url, TRUE);
        exit();
      }
      break;
    
    case 'view':
      // TODO: Do something more useful
      watchdog('emailkit_statistics', t('A message has been viewed.'));

      // Output a random image, suppressing any devel output
      $GLOBALS['devel_shutdown'] = FALSE;
      print(_emailkit_statistics_random_image(TRUE));
      exit();

      break;
  }
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_build() that prepares the message for statistics.
 */
function _emailkit_statistics_message_build(&$message) {
  // Add an image that will be requested when the message is opened
  $message['emailkit_statistics_open'] = array(
    '#value' => '<img src="' . check_plain(emailkit_statistics_url(array('type' => 'view'))) . '" alt="" />',
  );
}

/**
 * Private helper function for emailkit_statistics_emailkit_message_render() that prepares the message for statistics.
 */
function _emailkit_statistics_message_render(&$output, $format) {
  global $emailkit_statistics_message_render_format;
  
  $emailkit_statistics_message_render_format = $format;
  
  $protocol = "https?://";
  $authentication = "[a-zA-Z0-9:%_+*~#?&=.,/;-]+@";
  $ip = "(?:[0-9]{1,3}\.){3}[0-9]{1,3}";
  $domain = "(?:[A-Za-z0-9._+-]+\.)*[A-Za-z0-9._+-]+";
  $port = ":\d{1,5}";
  $path = "[a-zA-Z0-9:%_+*~#&\[\]=/;?\.,-]*[a-zA-Z0-9:%_+*~#&\[\]=/;-]";
  $pattern = "^$protocol(?:$authentication)?(?:$ip|$domain)(?:$port)?/?$path^";

  $output = preg_replace_callback($pattern, '_emailkit_statistics_url_rewrite', $output);
}

/**
 * Callback for preg_replace_callback() used in _emailkit_statistics_message_render().
 */
function _emailkit_statistics_url_rewrite($match) {
  global $emailkit_statistics_message_render_format;
  
  // Decode the URL
  $url = $match[0];
  if ($emailkit_statistics_message_render_format == EMAILKIT_MESSAGE_FORMAT_HTML) {
    $url = htmlspecialchars_decode($url);
  }
  
  // Rewrite the URL, but only if it is not an emailkit URL
  if (strpos($url, url('emailkit', array('absolute' => TRUE))) === FALSE) {
    $url = emailkit_statistics_url(array('type' => 'click', 'url' => $url));
  }
  
  // Encode the URL
  if ($emailkit_statistics_message_render_format == EMAILKIT_MESSAGE_FORMAT_HTML) {
    $url = htmlspecialchars($url);
  }
  
  return $url;
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
