<?php
/**
 * @file
 * Drush commands for Stripe module.
 */

/**
 * Implements hook_drush_help().
 */
function stripe_api_drush_help($section) {
  switch ($section) {
    case 'drush:stripe-library-download':
      return dt('Download the latest stable Stripe PHP Library.');
  }
}

/**
 * Implements hook_drush_command().
 */
function stripe_api_drush_command() {
  $items['stripe-api-download'] = array(
    'description' => 'Download the latest stable Stripe PHP Library.',
    'options' => array(
      'version' => 'Specify API library version to download. Example: 2.2.0',
    ),
  );
  $items['stripe-api'] = array(
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
      'drush stripe-api customer retrieve cus_id123123123' => 'Load a Stripe customer object.',
      'drush stripe-api account retrieve' => 'Get account information.',
      'drush stripe-api customer create --params="email=test@example.com&description=Testing"' => 'Create a new customer.',
      'drush stripe-api customer all --params="limit=5"' => 'Get 5 Stripe customers.',
      'drush stripe-api plan all' => 'List all plans',
      'drush stripe-api plan create --params="amount=2000&interval=month&name=TEST&currency=usd&id=test-plan"' => 'Create a new plan',
      'drush stripe-api plan retrieve test-plan --update="name=new name&amount=2500"' => 'Update plan information.',
      'drush stripe-api customer retrieve cus_id123123123 --delete' => 'Delete the given customer.',
      'drush stripe-api customer retrieve cus_id123123123 --update="email=new@email.com"' => 'Update a customers email.',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  return $items;
}

/**
 * Perform a Stripe API call.
 */
function drush_stripe_api($obj, $method, $params = NULL) {
  if (!$params && drush_get_option('params')) {
    $params = drush_get_option('params');
    $params = drupal_get_query_array($params);
  }
  $stripe_object = stripe_api_call($obj, $method, $params);
  if ($stripe_object) {
    if ($update = drush_get_option('update')) {
      $update = drupal_get_query_array($update);
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

/**
 * Command callback for drush_stripe_library_download.
 */
function drush_stripe_api_download() {
  $destination = DRUPAL_ROOT . '/sites/all/libraries';
  drush_log($destination);

  // Create sites/all/libraries if necessary.
  if (!is_dir($destination)) {
    drush_log(dt('The sites/all/libraries directory does not exist.'), 'warning');
    $confirm = drush_confirm(dt('Would you like to create it?'));
    if (!$confirm || !drush_mkdir($destination)) {
      drush_log(dt('Unable to download Stripe to sites/all/libraries.'), 'error');
      return FALSE;
    }
  }
  $version = drush_get_option('version') ? drush_get_option('version') : '2.2.0';
  $path = drush_download_file('https://github.com/stripe/stripe-php/archive/v' . $version . '.tar.gz');
  drush_log($path);

  $file_list = drush_tarball_extract($path, $destination, TRUE);
  dlm($file_list);
  if (empty($file_list)) {
    drush_log(dt('Unable to extract the Stripe PHP Library.'), 'error');
    return FALSE;
  }

  // Now move the directory to sites/all/libraries/stripe.
  $directory = reset($file_list);
  if (!drush_move_dir("$destination/$directory", "$destination/stripe", TRUE)) {
    drush_log(dt('Unable to move the Stripe PHP Library to sites/all/libraries/stripe'), 'error');
    return FALSE;
  }

  // Parse the version number from the original directory.
  $version = str_replace('stripe-php-', '', $directory);
  $dt_args = array('@version' => rtrim($version, '/'));
  $message = dt('Copied the Stripe PHP Library version @version to sites/all/libraries/stripe.', $dt_args);
  drush_log($message, 'success');
}
