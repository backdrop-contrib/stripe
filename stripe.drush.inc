<?php
/**
 * @file
 * Drush commands for Stripe module.
 */

/**
 * Implements hook_drush_help().
 */
function stripe_drush_help($section) {
  switch ($section) {
    case 'drush:stripe':
      return dt('Make a Stripe API call.');
  }
}

/**
 * Implements hook_drush_command().
 */
function stripe_drush_command() {
  $items['stripe'] = array(
    'description' => "Make a Stripe API call.",
    'arguments' => array(
      'object' => 'Stripe object, such as customer, account, subscription, etc...',
      'method' => 'Object method to call, su"ch as retrieve, create, all.',
      'param' => 'String param to pass to the API call.',
    ),
    'options' => array(
      'params' => 'Key/value pairs to pass to the API call.',
      'delete' => 'Delete the object that is retrieved.',
      'update' => 'Update the object that is retrieved with these new key/value pairs.',
    ),
    'examples' => array(
      'drush stripe customer retrieve cus_id123123123' => 'Load a Stripe customer object.',
      'drush stripe account retrieve' => 'Get account information.',
      'drush stripe customer create --params="email=test@example.com&description=Testing"' => 'Create a new customer.',
      'drush stripe customer all --params="limit=5"' => 'Get 5 Stripe customers.',
      'drush stripe plan all' => 'List all plans',
      'drush stripe plan create --params="amount=2000&interval=month&name=TEST&currency=usd&id=test-plan"' => 'Create a new plan',
      'drush stripe plan retrieve test-plan --update="name=new name&amount=2500"' => 'Update plan information.',
      'drush stripe customer retrieve cus_id123123123 --delete' => 'Delete the given customer.',
      'drush stripe customer retrieve cus_id123123123 --update="email=new@email.com"' => 'Update a customers email.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  return $items;
}

/**
 * Perform a Stripe API call.
 */
function drush_stripe($obj, $method, $params = NULL) {
  if (!$params && drush_get_option('params')) {
    $params = drush_get_option('params');
    $params = backdrop_get_query_array($params);
  }
  $stripe_object = stripe_api_call($obj, $method, $params);
  if ($stripe_object) {
    if ($update = drush_get_option('update')) {
      $update = backdrop_get_query_array($update);
      foreach ($update as $key => $val) {
        $stripe_object->$key = $val;
      }
      $stripe_object->save();
      drush_print('Updated!');
    }
    if (drush_get_option('delete')) {
      $stripe_object->delete();
      drush_print('Deleted!');
    }
    drush_print($stripe_object);
  }
  else {
    drush_set_error('Object not found.');
  }
}
