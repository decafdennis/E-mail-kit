<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Processes and returns the given destination selection element.
 */
function emailkit_destination_select_process($element) {
  $element['#tree'] = TRUE;
  $element['#theme'] = 'emailkit_destination_select_children';

  $element['destination'] = array(
    '#type' => 'radios',
    '#title' => $element['#title'],
    '#required' => $element['#required'],
    '#options' => array(),
  );

  // Add form elements for each exposed destination
  $destinations = emailkit_destination_info();
  foreach ($destinations as $destination => $destination_info) {
    if (!$destination_info['exposed']) {
      continue;
    }
    
    $element['destination']['#options'][$destination] = $destination_info['label'];
    $element['destination_form'][$destination] = emailkit_destination_form($destination);
  }

  return $element;
}

/**
 * Processes and returns the given destination selection element after it has been built.
 */
function emailkit_destination_select_after_build($element) {
  if (isset($element['#post']) && !empty($element['#post'])) {
    // Mark all destination forms as having been validated so they don't generate form errors, except for the selected destination
    $selected_destination = isset($element['#value']['destination']) ? $element['#value']['destination'] : NULL;
    foreach (element_children($element['destination_form']) as $destination) {
      if ($destination != $selected_destination) {
        _emailkit_element_set_validated($element['destination_form'][$destination]);
      }
    }
  }

  return $element;
}

/**
 * Renders the given destination selection element and returns the result.
 */
function theme_emailkit_destination_select($element) {
  drupal_add_css(drupal_get_path('module', 'emailkit') . '/emailkit.destination_select.css');
  drupal_add_js(drupal_get_path('module', 'emailkit') . '/emailkit.destination_select.js');
  
  return theme('form_element', $element, $element['#children']);
}

/**
 * Renders all child elements of the given destination selection element and returns the result.
 */
function theme_emailkit_destination_select_children($element) {
  $output = "";

  $output .= '<dl class="emailkit-destination-select-items">';
  foreach ($element['destination']['#options'] as $key => $value) {
    $class = str_replace(array('][', '_', ' '), '-', $key);
    
    $output .= '<dt class="emailkit-destination-select-item-' . $class . '">';
    $output .= drupal_render($element['destination'][$key]);
    $output .= '</dt>';

    if (isset($element['destination_form'][$key])) {
      $output .= '<dd class="emailkit-destination-select-item-' . $class . '">';
      $output .= drupal_render($element['destination_form'][$key]);
      $output .= '</dd>';
    }
  }
  $output .= '</dl>';

  return $output;
}
