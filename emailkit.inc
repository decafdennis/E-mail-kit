<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include contains internal code that does not need to be in the main module file.
 */

/**
 * Implementation of hook_dispatcher_send() for the emailkit_default dispatcher.
 */
function emailkit_default_dispatcher_send($dispatcher, $message, $destination) {
  // Create the text part
  $text_part = array(
    'headers' => array(
      'Content-Type' => 'text/plain; charset=utf-8; format=flowed; delsp=yes',
      'Content-Transfer-Encoding' => 'quoted-printable',
    ),
    'body' => _emailkit_mime_quoted_printable(emailkit_message_render($message, EMAILKIT_MESSAGE_FORMAT_TEXT)),
  );
  
  // Create the HTML part
  $html_part = array(
    'headers' => array(
      'Content-Type' => 'text/html; charset=utf-8',
      'Content-Transfer-Encoding' => 'quoted-printable',
    ),
    'body' => _emailkit_mime_quoted_printable(emailkit_message_render($message, EMAILKIT_MESSAGE_FORMAT_HTML)),
  );
  
  // Prepare the multipart/alternative body
  $boundary = _emailkit_mime_content_boundary();
  $body = _emailkit_mime_multipart_alternative($boundary, array($text_part, $html_part));
  
  // Prepend a message for clients that don't support MIME
  $body = "This is a message in MIME format.  If you see this, your e-mail client does not support this format.\n\n$body";
  
  // Prepare the origin
  $from = variable_get('site_mail', ini_get('sendmail_from'));

  // Prepare the destination
  if ($destination['#type'] == 'emailkit_imf') {
    $to = $destination['#to'];
  }
  else {
    // TODO: Don't simply put all recipients in the To field
    $to_array = array();
    foreach ($destination['#recipients'] as $recipient) {
      $to_array[] = $recipient['address'];
    }

    $to = implode(", ", $to_array);
  }

  // Prepare the headers
  $headers = array(
    'From' => $from,
    'Sender' => $from,
    'Return-Path' => $from,
    'Errors-To' => $from,
    'MIME-Version' => '1.0',
    'Content-Type' => 'multipart/alternative; boundary="' . $boundary . '"',
  );

  return _emailkit_mail($to, $message['#subject'], $body, $headers);
}

/**
 * Private function that invokes the info hooks, caches the results and returns the requested information.
 *
 * @param $domain The domain from which to return information. Currently valid domains are 'destination' and 'dispatcher'.
 * @param $identifier The identifier of the entity to return information about, or NULL to return information about all entities. Defaults to NULL.
 * @param $reset A flag indicating whether to reset the cache and call the info hooks again. Defaults to FALSE.
 */
function _emailkit_entity_info($domain, $identifier = NULL, $reset = FALSE) {
  assert('in_array($domain, array(\'destination\', \'dispatcher\', \'message\')) /* should be a recognized domain */');
  
  static $info = NULL;

  // Check whether we need to (re)load the cached information
  if (!isset($info[$domain]) || $reset) {
    $info[$domain] = array();

    // Determine domain info
    $domain_info_function = '_emailkit_' . $domain . '_domain_info';
    if (function_exists($domain_info_function)) {
      $domain_info = call_user_func($domain_info_function);
    }
    else {
      $domain_info = array();
    }
    
    // Determine entity identifier key
    if (isset($domain_info['#identifier'])) {
      $identifier_key = $domain_info['#identifier'];
    }
    else {
      $identifier_key = '#identifier';
    }
    
    // Determine entity defaults
    if (isset($domain_info['#defaults'])) {
      $defaults = $domain_info['#defaults'];
    }
    else {
      $defaults = array();
    }
    
    // Invoke hook_emailkit_DOMAIN_info() on all modules that implement it
    $hook = 'emailkit_' . $domain . '_info';
    foreach (module_implements($hook) as $module) {
      $module_info = module_invoke($module, $hook);

      foreach ($module_info as $entity_identifier => $entity_info) {
        // Force the identifier and module parameters
        $entity_info[$identifier_key] = $entity_identifier;
        $entity_info['#module'] = $module;
        
        // Add the domain-specific entity defaults as well as the general defaults
        $entity_info += $defaults + array(
          '#file' => NULL,
          '#base' => $entity_identifier,
        );
        
        // Make sure to unset this if it was returned from the hook, to be safe
        unset($entity_info['#_include']);

        // Determine the full path to the include file, if necessary
        if (isset($entity_info['#file'])) {
          $module_path = drupal_get_path('module', $module);
          $include_path = $module_path . '/' . $entity_info['#file'];
          
          // Only allow the file to be inside the module directory, to be safe
          if (file_check_location($include_path, $module_path)) {
            $entity_info['#_include'] = $include_path;
          }
        }
        
        $info[$domain][$entity_identifier] = $entity_info;
      }
    }
  }

  // Return information about a specific entity or about all identities, depending on whether an identifier was given
  if (isset($identifier)) {
    if (isset($info[$domain][$identifier])) {
      return $info[$domain][$identifier];
    }
    else {
      return NULL;
    }
  }
  else {
    return $info[$domain];
  }
}

/**
 * Private function that returns the name of the function that implements the given hook.
 *
 * @param $domain The domain from which to return information.
 * @param $identifier The identifier of the entity to return information about.
 * @param $hook The name of the hook to return information about.
 * @param $check Whether to check the existence of the function. NULL will be returned if the function does not exist. Defaults to TRUE.
 *
 * @return The name of the function that implements the hook or NULL.
 */
function _emailkit_entity_identifier_hook($domain, $identifier, $hook, $check = TRUE) {
  assert('isset($identifier) /* expected $identifier that is non-NULL */');
  
  return _emailkit_entity_info_hook(_emailkit_entity_info($domain, $identifier), $hook, $check);
}

/**
 * Private function that returns the name of the function that implements the given hook.
 *
 * @param $entity_info A structured entity array containing at least the #base key.
 * @param $hook The name of the hook to return information about.
 * @param $check Whether to check the existence of the function. NULL will be returned if the function does not exist. Defaults to TRUE.
 *
 * @return The name of the function that implements the hook or NULL.
 */
function _emailkit_entity_info_hook($entity_info, $hook, $check = TRUE) {
  assert('is_array($entity_info) /* expected $entity_info that is non-NULL and is an array */');
  assert('is_string($hook) /* expected $hook that is non-NULL and is a string */');
  
  // Load the include file, if necessary
  if ($check && isset($entity_info['#_include'])) {
    require_once($entity_info['#_include']);
  }
  
  // Return the function name, if it exists or if checking is disabled
  $function = $entity_info['#base'] . '_' . $hook;
  if (!$check || function_exists($function)) {
    return $function;
  }
  else {
    return NULL;
  }
}

/**
 * Private function used by emailkit_message().
 */
function _emailkit_message($message_id) {
  // Get information about the message type
  $info = emailkit_message_info($message_id);
  if (!isset($info)) {
    return NULL;
  }

  $function = _emailkit_entity_info_hook($info, 'message');
  if (!isset($function)) {
    return NULL;
  }

  // Retrieve the message by invoking hook_message() and passing on our arguments
  $args = func_get_args();
  $message = call_user_func_array($function, $args);
  if (!isset($message)) {
    return NULL;
  }
  
  // Set default message regions
  $message['header']['#weight'] = -0.1;
  $message['content']['#weight'] = 0;
  $message['footer']['#weight'] = 0.1;
  
  // Allow other modules to add their stuff to the message
  foreach (module_implements('emailkit_message_build') as $module) {
    $function = $module . '_emailkit_message_build';
    $function($message_id, $message);
  }
  
  // Allow all modules to alter the message
  foreach (module_implements('emailkit_message_alter') as $module) {
    $function = $module . '_emailkit_message_alter';
    $function($message_id, $message);
  }
  
  // Finish the message and prepare it so that it is ready for rendering
  $message['#type'] = 'emailkit_message';
  $message['#id'] = $message_id;
  _emailkit_message_prepare($message);
  
  return $message;
}

/**
 * Prepares the given message (or part of a message) for rendering.
 */
function _emailkit_message_prepare(&$message_subtree) {
  $count = 0;
  $has_weights = FALSE;
  foreach (element_children($message_subtree) as $key) {
    // Check if this element has a weight set
    if (isset($message_subtree[$key]['#weight'])) {
      $has_weights = TRUE;
    }
    else {
      // Assign a decimal weight so that the original order is preserved when drupal_render() does an unstable sort
      $message_subtree[$key]['#weight'] = $count * 1e-4;
    }
    
    // Recurse
    _emailkit_message_prepare($message_subtree[$key]);
    
    $count++;
  }
  
  // Leave a cue for drupal_render() about whether this subtree needs to be sorted
  $message_subtree['#sorted'] = !$has_weights;
}

/**
 * Private function used by emailkit_message_render().
 */
function _emailkit_message_render($message, $format) {
  $html = _emailkit_resolve_urls(drupal_render($message));

  if ($format == EMAILKIT_MESSAGE_FORMAT_TEXT) {
    // Apply a very simply HTML to text conversion
    // TODO: Allow stuff to be rendered as plain text, or allow stuff to be specified in plain text and be converted to HTML?
    return drupal_html_to_text($html);
  }
  else {
    return $html;
  }
}

/**
 * Replaces all relative URLs generated by url() with absolute URLs.
 */
function _emailkit_resolve_urls($html) {
  // Find all tags with a href or src attribute containing a URL that starts with the base path (which is always at least /)
  $pattern = '/(<[^>]+?(?:href|src)=")' . preg_quote(base_path(), '/') . '([^"]*"[^>]*?>)/';
  return preg_replace_callback($pattern, '_emailkit_resolve_url', $html);
}

/**
 * Callback for preg_replace_callback() in _emailkit_resolve_urls().
 */
function _emailkit_resolve_url($match) {
  global $base_url;

  // Replace the base path with the base URL
  return $match[1] . $base_url . '/' . $match[2];
}

/**
 * Returns a unique string that can be used as a content boundary in MIME mesages.
 */
function _emailkit_mime_content_boundary() {
  return 'emailkit=_' . md5(uniqid(mt_rand(), TRUE));
}

/**
 * Returns multipart/alternative content using the given boundary and consisting of the given parts.
 *
 * @param $boundary A string used as a content boundary.
 * @param $parts An array of parts, which by itself are arrays containing headers and a body, similar to what drupal_mail_send() accepts.
 */
function _emailkit_mime_multipart_alternative($boundary, $parts) {
  $output = '';
  
  foreach ($parts as $part) {
    // Add the boundary
    $output .= "--$boundary\r\n";
    
    // Add the content header
    if (isset($part['headers'])) {
      foreach ($part['headers'] as $key => $value) {
        $output .= $key . ": " . mime_header_encode($value) . "\r\n";
      }
    }
    $output .= "\r\n";
    
    // Add the content itself
    if (isset($part['body'])) {
      $output .= $part['body'] . "\r\n";
    }
  }
  
  // Add the final boundary
  if (!empty($parts)) {
    $output .= "--$boundary--\r\n";
  }
  
  return $output;
}

/**
 * Encodes the given string using quoted-printable encoding.
 *
 * @param $input The input string, which may have CRLF or LF line endings.
 */
function _emailkit_mime_quoted_printable($input) {
  $output = '';
  $output_line_length = 0;
  
  // Just make absolutely sure we don't have any CR line endings around
  $input = str_replace("\r", '', $input);
  $input_length = strlen($input);
  
  // Iterate each character of the input string
  for ($i = 0; $i < $input_length; $i++) {
    // Get the character and its ASCII value
    $c = $input[$i];
    $ascii = ord($c);
    
    // Encode line endings
    if ($ascii == 0x0A) {
      $c = "\r\n";
      $output_line_length = -strlen($c);
    }
    // Encode spaces at the end of a line
    else if ($ascii == 0x20 && $i + 1 < $input_length && $input[$i + 1] == "\n") {
      $c = "=20";
    }
    // Encode equal signs and non-printable ASCII characters
    else if ($ascii == 0x3D || $ascii < 0x20 || $ascii > 0x7E) {
      $c = sprintf("=%02X", $ascii);
    }
    
    // Add a soft line break, if necessary
    if ($output_line_length + strlen($c) >= 76) {
      $output .= "=\r\n";
      $output_line_length = 0;
    }
    
    $output .= $c;
    $output_line_length += strlen($c);
  }
  
  return $output;
}

/**
 * Wrapper for PHP's mail() function that processes the subject, body and headers before passing them into mail().
 *
 * @param $to The recipient, compliant with RFC 2822.
 * @param $subject The subject, which will MIME encoded by this function.
 * @param $body The body, which may or may not have CRLF line endings.
 * @param $headers An associated array of header names and values. The values will be MIME encoded by this function.
 *
 * @return A flag indicating whether sending was successful.
 */
function _emailkit_mail($to, $subject, $body, $headers = array()) {
  // Prepare the subject
  $subject = mime_header_encode($subject);

  // Prepare the body
  // Note: mail() wants LF line endings
  $body = str_replace("\r", '', $body);
  
  // Prepare the headers
  if (!isset($headers['X-Mailer'])) {
    $headers['X-Mailer'] = 'E-mail kit (Drupal)';
  }
  
  $headers_encoded = array();
  foreach ($headers as $key => $value) {
    $headers_encoded[] = $key . ': ' . mime_header_encode($value);
  }

  // Note: follow drupal_mail_send()'s recommendation of using LF
  $headers = implode("\n", $headers_encoded);

  return mail($to, $subject, $body, $headers);
}
