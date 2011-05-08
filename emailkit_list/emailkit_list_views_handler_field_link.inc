<?php
// Developed by Dennis Stevense for Digital Deployment

class emailkit_list_views_handler_field_link extends views_handler_field {
  var $table_base_field = NULL;

  function init(&$view, $options) {
    parent::init($view, $options);

    $table_data = views_fetch_data($this->table);
    $this->table_base_field = $table_data['table']['base']['field'];

    $this->additional_fields[$this->table_base_field] = $this->table_base_field;
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['text'] = array('default' => '', 'translatable' => TRUE);

    return $options;
  }

  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['text'] = array(
      '#type' => 'textfield',
      '#title' => t('Text to display'),
      '#description' => t('Leave empty to use the default.'),
      '#default_value' => $this->options['text'],
    );
  }

  function query() {
    $this->ensure_my_table();
    $this->add_additional_fields();
  }
  
  function get_link_access() {
    return FALSE;
  }
  
  function get_link_path($table_base_field_value) {
    return '<front>';
  }
  
  function get_link_text() {
    return !empty($this->options['text']) ? $this->options['text'] : $this->get_default_link_text();
  }
  
  function get_default_link_text() {
    return t('view');
  }

  function render($values) {
    $table_base_field_alias = $this->aliases[$this->table_base_field];
    
    if (isset($values->$table_base_field_alias) && $this->get_link_access()) {
      return l($this->get_link_text(), $this->get_link_path($values->$table_base_field_alias), array('query' => drupal_get_destination()));
    }
  }

}
