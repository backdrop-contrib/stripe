<?php

/**
 * Implements hook_drush_command().
 */
function webform_stripe_drush_command() {
  $items['webform-stripe-download'] = array(
    'description' => dt('Downloads the Stripe PHP library from https://github.com/stripe/stripe-php.'),
  );
  return $items;
}

/**
 * Webform Stripe Download command.
 */
function drush_webform_stripe_download() {
  // Require Libraries module.
  if (!module_exists('libraries')) {
    drush_log(dt('Webform Stripe requires the Libraries module (https://www.drupal.org/project/libraries) in order to install the Stripe PHP library.'), 'warning');
    return;
  }

  // Download with Git.
  $path = DRUPAL_ROOT . '/sites/all/libraries/stripe-php';
  if (drush_shell_exec('git clone git://github.com/stripe/stripe-php.git ' . $path)) {
    drush_log(dt('Stripe PHP library has been cloned via git to @path.', array('@path' => $path)), 'success');
  }
  else {
    drush_log(dt('Drush was unable to clone Stripe PHP library to @path.', array('@path' => $path)), 'warning');
  }
}
