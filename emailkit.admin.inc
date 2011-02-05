<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Returns the admin settings page.
 */
function emailkit_admin_settings() {
  // Yes, the page callback for this menu item could've been set to drupal_get_form with emailkit_admin_settings_form as an argument, but I believe this way is more encapsulated. The implementation of hook_menu() shouldn't have to worry about whether this page is a form or not.
  return drupal_get_form('emailkit_admin_settings_form');
}

/**
 * Returns the admin settings form.
 */
function emailkit_admin_settings_form() {
  $form = array();

  $form['drupal_mail_wrapper_status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use E-mail kit to send all site messages'),
    '#description' => t('If you enable this option, all site messages will be sent through E-mail kit. This way, all site messages will be sent as HTML and get the same appearance.'),
    '#default_value' => (variable_get('smtp_library', '') == _emailkit_smtp_library_path()),
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  
  return $form;
}

/**
 * Handles submission of the admin settings form.
 */
function emailkit_admin_settings_form_submit($form, &$form_state) {
  // Is the checkbox checked?
  if (empty($form_state['values']['drupal_mail_wrapper_status'])) {
    // Clear out the variable only if it is still pointing to our Drupal mail wrapper
    if (variable_get('smtp_library', '') == _emailkit_smtp_library_path()) {
      variable_del('smtp_library');
      
      drupal_set_message(t('E-mail kit will not be used anymore to send all site messages.'));
    }
  }
  else {
    // Set the variable to our Drupal mail wrapper
    if (variable_get('smtp_library', '') != _emailkit_smtp_library_path()) {
      variable_set('smtp_library', _emailkit_smtp_library_path());

      drupal_set_message(t('E-mail kit will now be used to send all site messages.'));
    }
  }
}

/**
 * Private helper function that returns the path to the file that contains our Drupal mail wrapper.
 */
function _emailkit_smtp_library_path() {
  return drupal_get_path('module', 'emailkit') . '/emailkit.drupal.inc';
}

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

  // Sort by module name
  uksort($message_types_by_module, '_emailkit_admin_message_type_list_sort_by_name');

  return theme('emailkit_admin_message_type_list', $message_types_by_module);
}

function _emailkit_admin_message_type_list_sort_by_name($a, $b) {
  return strcasecmp($a['#name'], $b['#name']);
}

/**
 * Returns a page for editing the given message type.
 */
function emailkit_admin_message_type_edit($message_type) {
  // Note: I know we could've just set the menu callback to drupal_get_form with emailkit_admin_message_type_form as an argument, but I believe that this determination should be made here, not in hook_menu()
  return drupal_get_form('emailkit_admin_message_type_form', $message_type);
}

/**
 * Returns a form for editing the given message type.
 */
function emailkit_admin_message_type_form(&$form_state, $message_type) {
  $message_id = $message_type['#id'];
  
  $form = array(
    '#message_type' => $message_type,
  );

  // Build fieldset for choosing dispatchers for destinations
  $form['dispatcher'] = array();
  $destinations = emailkit_destination_info();
  $dispatchers = emailkit_dispatcher_info();
  foreach ($destinations as $destination_type => $destination_info) {
    $default = $destination_info['#default_dispatcher'];
    
    // Build an array of possible dispatchers for this destination
    $options = array();
    foreach ($dispatchers as $dispatcher_name => $dispatcher_info) {
      if (in_array($destination_type, $dispatcher_info['#destinations'])) {
        $label = $dispatcher_info['#label'];
        
        // Add "(default)" to indicate the default dispatcher
        if ($dispatcher_name == $default) {
          $label .= ' (' . t('default') . ')';
        }
        
        $options[$dispatcher_name] = $label;
      }
    }

    // Skip this destination type if there is nothing to choose
    if (count($options) <= 1) {
      continue;
    }

    $key = _emailkit_message_dispatcher_variable($message_id, $destination_type);
    $form['dispatcher'][$key] = array(
      '#type' => 'radios',
      '#title' => $destination_info['#label'],
      '#options' => $options,
      '#default_value' => emailkit_message_dispatcher($message_id, $destination_type),
    );
  }
  
  // If there are any dispatcher fields, surround them with a fieldset
  if (!empty($form['dispatcher'])) {
    $form['dispatcher'] += array(
      '#type' => 'fieldset',
      '#title' => t('Dispatchers'),
      '#description' => t('Some kind of destinations can be serviced by more than one dispatcher. Choose the dispatcher to use for those destinations below.'),
    );
  }

  return system_settings_form($form);
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
