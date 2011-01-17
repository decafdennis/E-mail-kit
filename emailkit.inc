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

    // Determine domain-specific entity defaults
    $defaults_function = '_emailkit_' . $domain . '_info_defaults';
    if (function_exists($defaults_function)) {
      $defaults = call_user_func($defaults_function);
    }
    else {
      $defaults = array();
    }
    
    // Invoke hook_emailkit_DOMAIN_info() on all modules that implement it
    $hook = 'emailkit_' . $domain . '_info';
    foreach (module_implements($hook) as $module) {
      $module_info = module_invoke($module, $hook);

      foreach ($module_info as $entity_identifier => $entity_info) {
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
