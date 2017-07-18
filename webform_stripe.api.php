<?php

/**
 * Alters Stripe Checkout settings.
 *
 * @param $settings
 *   An array of Stripe Checkout settings.
 * @param $context
 *   An array of contextual information with the following keys:
 *   - node: The Webform node object.
 *   - component: The Webform component array.
 *   - data: The array of form values submitted by the user.
 */
function hook_webform_stripe_checkout_settings_alter(&$settings, &$context) {
  // Change Stripe Checkout title to Webform title.
  $settings['name'] = $node->title;

  // Change Stripe Checkout currency based on a "Country" component value.
  if (isset($data['country']) && $data['country'] === 'us') {
    $settings['currency'] = 'USD';
  }
}
