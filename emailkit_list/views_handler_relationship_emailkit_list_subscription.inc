<?php
// Developed by Dennis Stevense for Digital Deployment

class views_handler_relationship_emailkit_list_subscription extends views_handler_relationship  {

  function query() {
    $this->ensure_my_table();
    
    // Determine the base table for the relationship
    $base_table = $this->definition['base'];

    // Determine the base field for the relationship
    if (empty($this->definition['base field'])) {
      // Default to the base field of the base table
      $base_table_data = views_fetch_data($base_table);
      $base_field = $base_table_data['table']['base']['field'];
    }
    else {
      $base_field = $this->definition['base field'];
    }

    // Add the subscriptions table
    $subscription_table = $this->query->add_table('emailkit_list_subscription', $this->relationship);

    // Build a definition for the join between the subscriptions table and the base table for the relationship
    $join_definition = $this->definition;
    $join_definition['table'] = $base_table;
    $join_definition['field'] = $base_field;
    $join_definition['left_table'] = $subscription_table;
    $join_definition['left_field'] = $base_field;

    if (!empty($this->options['required'])) {
      $join_definition['type'] = 'INNER';
    }

    // Build the join
    $join = new views_join();
    $join->definition = $join_definition;
    $join->construct();
    
    // I have no idea why this is necessary
    $join->adjusted = TRUE;

    // Add a relationship with the join
    $this->alias = $this->query->add_relationship($this->table . '_' . $base_table, $join, $base_table, $this->relationship);
  }

}
