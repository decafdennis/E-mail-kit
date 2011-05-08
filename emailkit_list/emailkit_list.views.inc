<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This include provides views integration.
 */

/**
 * Implementation of hook_views_data().
 */
function emailkit_list_views_data() {
  $data = array();

  $data['emailkit_list'] = array(
    'table' => array(
      'group' => t('E-mail list'),
      'base' => array(
        'field' => 'lid',
        'title' => t('E-mail list'),
        'help' => t('E-mail lists that subscribers can subscribe to.'),
      ),
      'join' => array(
        'emailkit_list_subscriber' => array(
          'left_table' => 'emailkit_list_subscription',
          'left_field' => 'lid',
          'field' => 'lid',
        ),
        'users' => array(
          'left_table' => 'emailkit_list_subscription',
          'left_field' => 'lid',
          'field' => 'lid',
        ),
      ),
    ),
    'lid' => array(
      'title' => t('Lid'),
      'help' => t('The list ID.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_numeric',
      ),
    ),
    'name' => array(
      'title' => t('Name'),
      'help' => t('The name of the list.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_string',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_string',
      ),
    ),
    'description' => array(
      'title' => t('Description'),
      'help' => t('The description of the list.'),
      'field' => array(
        'handler' => 'views_handler_field',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_string',
      ),
    ),
    'weight' => array(
      'title' => t('Weight'),
      'help' => t('The sort order of the list.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
      ),
    ),
    'edit_link' => array(
      'title' => t('Edit link'),
      'help' => t('Provides a link to edit the e-mail list.'),
      'field' => array(
        'handler' => 'emailkit_list_views_handler_field_link_list_edit',
      ),
    ),
    'delete_link' => array(
      'title' => t('Delete link'),
      'help' => t('Provides a link to delete the e-mail list.'),
      'field' => array(
        'handler' => 'emailkit_list_views_handler_field_link_list_delete',
      ),
    ),
    'sid' => array(
      'title' => t('Subscribers'),
      'help' => t('The subscribers who are subscribed to the list.'),
      'relationship' => array(
        'handler' => 'emailkit_list_views_handler_relationship_subscription',
        'label' => t('subscriber'),
        'base' => 'emailkit_list_subscriber',
      ),
    ),
  );
  
  $data['emailkit_list_subscriber'] = array(
    'table' => array(
      'group' => t('E-mail list subscriber'),
      'base' => array(
        'field' => 'sid',
        'title' => t('E-mail list subscriber'),
        'help' => t('Subscribers who can subscribe to e-mail lists.'),
      ),
      'join' => array(
        'emailkit_list' => array(
          'left_table' => 'emailkit_list_subscription',
          'left_field' => 'sid',
          'field' => 'sid',
        ),
        'users' => array(
          'left_field' => 'mail',
          'field' => 'address',
        ),
      ),
    ),
    'sid' => array(
      'title' => t('Sid'),
      'help' => t('The subscriber ID.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_numeric',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_numeric',
      ),
    ),
    'address' => array(
      'title' => t('Address'),
      'help' => t('The e-mail address of the subscriber.'),
      'field' => array(
        'handler' => 'views_handler_field',
        'click sortable' => TRUE,
      ),
      'sort' => array(
        'handler' => 'views_handler_sort',
      ),
      'filter' => array(
        'handler' => 'views_handler_filter_string',
      ),
      'argument' => array(
        'handler' => 'views_handler_argument_string',
      ),
      'relationship' => array(
        'handler' => 'views_handler_relationship',
        'title' => t('User'),
        'help' => t('The user associated with the subscriber.'),
        'label' => t('user'),
        'base' => 'users',
        'base field' => 'mail',
      ),
    ),
    'edit_link' => array(
      'title' => t('Edit link'),
      'help' => t('Provides a link to edit the subscriber.'),
      'field' => array(
        'handler' => 'emailkit_list_views_handler_field_link_subscriber_edit',
      ),
    ),
    'delete_link' => array(
      'title' => t('Delete link'),
      'help' => t('Provides a link to delete the subscriber.'),
      'field' => array(
        'handler' => 'emailkit_list_views_handler_field_link_subscriber_delete',
      ),
    ),
    'lid' => array(
      'title' => t('Lists'),
      'help' => t('The lists which the subscriber is subscribed to.'),
      'relationship' => array(
        'handler' => 'emailkit_list_views_handler_relationship_subscription',
        'label' => t('list'),
        'base' => 'emailkit_list',
      ),
    ),
  );
  
  $data['emailkit_list_subscription'] = array(
    'table' => array(
      'join' => array(
        'emailkit_list' => array(
          'left_field' => 'lid',
          'field' => 'lid',
        ),
        'emailkit_list_subscriber' => array(
          'left_field' => 'sid',
          'field' => 'sid',
        ),
        'users' => array(
          'left_table' => 'emailkit_list_subscriber',
          'left_field' => 'sid',
          'field' => 'sid',
        ),
      ),
    ),
  );

  return $data;
}

/**
 * Implementation of hook_views_data_alter().
 */
function emailkit_list_views_data_alter(&$data) {
  $data['users']['table']['join']['emailkit_list'] = array(
    'left_table' => 'emailkit_list_subscriber',
    'left_field' => 'address',
    'field' => 'mail',
  );

  $data['users']['table']['join']['emailkit_list_subscriber'] = array(
    'left_field' => 'address',
    'field' => 'mail',
  );
  
  $data['users']['mail']['relationship'] = array(
    'handler' => 'views_handler_relationship',
    'group' => t('E-mail list'),
    'title' => t('Subscriber'),
    'help' => t('The subscriber associated with the user.'),
    'label' => t('subscriber'),
    'base' => 'emailkit_list_subscriber',
    'base field' => 'address',
  );
}

/**
 * Implementation of hook_views_handlers().
 */
function emailkit_list_views_handlers() {
  return array(
    'handlers' => array(
      'emailkit_list_views_handler_field_link' => array(
        'parent' => 'views_handler_field',
      ),
      'emailkit_list_views_handler_field_link_list_edit' => array(
        'parent' => 'emailkit_list_views_handler_field_link',
      ),
      'emailkit_list_views_handler_field_link_list_delete' => array(
        'parent' => 'emailkit_list_views_handler_field_link',
      ),
      'emailkit_list_views_handler_field_link_subscriber_edit' => array(
        'parent' => 'emailkit_list_views_handler_field_link',
      ),
      'emailkit_list_views_handler_field_link_subscriber_delete' => array(
        'parent' => 'emailkit_list_views_handler_field_link',
      ),
      'emailkit_list_views_handler_relationship_subscription' => array(
        'parent' => 'views_handler_relationship',
      ),
    ),
  );
}
