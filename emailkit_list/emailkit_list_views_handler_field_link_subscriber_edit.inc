<?php
// Developed by Dennis Stevense for Digital Deployment

class emailkit_list_views_handler_field_link_subscriber_edit extends emailkit_list_views_handler_field_link {
  
  function get_link_access() {
    return user_access('administer subscribers');
  }
  
  function get_link_path($table_base_field_value) {
    $lid = $table_base_field_value;
    
    return 'admin/emailkit/list/subscribers/' . $lid . '/edit';
  }
  
  function get_default_link_text() {
    return t('edit subscriber');
  }

}
