<?php
// Developed by Dennis Stevense for Digital Deployment

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
  
  // Allow other modules to add their elements to the message
  foreach (module_implements('emailkit_message_build') as $module) {
    $function = $module . '_emailkit_message_build';
    $function($message_id, $message);
  }

  // Give all modules a last chance to alter the message
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

function _emailkit_resolve_url($match) {
  global $base_url;

  // Replace the base path with the base URL
  return $match[1] . $base_url . '/' . $match[2];
}
