<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the admin page that lists the different message types for the user to configure.
 */
function emailkit_admin_message_type_list() {
  // Get all message types
  $message_types = emailkit_message_info();
  
  // Categorize the message types by module
  $message_types_by_module = array();
  foreach ($message_types as $message_id => $message_info) {
    $module = $message_info['#module'];

    // Add an array entry for the module, if necessary
    if (!isset($message_types_by_module[$module])) {
      $message_types_by_module[$module] = array(
        '#name' => _emailkit_module_name($module),
        '#message_types' => array(),
      );
    }
    
    // Add an array entry for the message type
    $message_types_by_module[$module]['#message_types'][$message_id] = $message_info;
  }

  // Sort by module name (closures FTW!)
  uksort($message_types_by_module, function($a, $b) {
    return strcasecmp($a['#name'], $b['#name']);
  });

  return theme('emailkit_admin_message_type_list', $message_types_by_module);
}

/**
 * Renders the admin page that lists the different message types for the user to configure.
 */
function theme_emailkit_admin_message_type_list($message_types) {
  drupal_add_css(drupal_get_path('module', 'emailkit') . '/emailkit.admin.css');
  
  $output = "";

  $header = array(t('Name'), array('data' => t('Operations')));
  $rows = array();
  foreach ($message_types as $module => $module_info) {
    // Add a row for the module
    $rows[] = array(
      'class' => 'module',
      'data' => array(
        array(
          'data' => check_plain($module_info['#name']),
          'colspan' => _emailkit_table_colspan($header),
        ),
      ),
    );
    
    foreach ($module_info['#message_types'] as $message_id => $message_info) {
      // Add a row for the message type
      $rows[] = array(
        'class' => 'message-type',
        'data' => array(
          array(
            'class' => 'name',
            'data' => check_plain($message_info['#label']),
          ),
          // TODO: Implement a callback for this path
          l(t('edit'), 'admin/emailkit/messages/' . $message_id . '/edit', array('query' => drupal_get_destination())),
        ),
      );
    }
  }

  if (empty($rows)) {
    $rows[] = array(
      array(
        'data' => t('No messages have been defined.'),
        'colspan' => _emailkit_table_colspan($header),
      ),
    );
  }

  $output .= theme('table', $header, $rows, array('id' => 'emailkit-admin-message-type-list'));

  return $output;
}

/**
 * Returns the human-readable name of the given module. This is unlike module_list(), which returns the system name. If the human-readable name cannot be determined, the system name will be returned.
 */
function _emailkit_module_name($module) {
  $module_info = db_result(db_query("SELECT info FROM {system} WHERE name = '%s'", $module));

  // Do we have a result?
  if ($module_info) {
    $module_info = unserialize($module_info);
    
    if (isset($module_info['name'])) {
      return $module_info['name'];
    }
  }
  
  return $module;
}
