<?php

/**
 * Alters Stripe Checkout settings.
 *
 * @param $settings
 *   An array of Stripe Checkout settings.
 * @param $context
 *   An array of contextual information with the following keys:
 *   - node: The Webform node object.
 *   - component: The Webform component array for the Stripe payment component.
 *   - data: The array of submitted form values (see "state" for how this is
 *     keyed).
 *   - form: The array of submitted form values, keyed by form key.
 *   - state: One of the following values:
 *     - WEBFORM_STRIPE_SETTINGS_CHECKOUT: Before opening Stripe Checkout.
 *       The 'data' array is keyed by component's 'form_key'.
 *     - WEBFORM_STRIPE_SETTINGS_PROCESS: After user submits Stripe Checkout
 *       form and before server processes Stripe charge/subscription.
 *       The 'data' array is keyed by component's 'cid'.
 */
function hook_webform_stripe_checkout_settings_alter(&$settings, &$context) {
  // Change Stripe Checkout title to Webform title.
  $settings['name'] = $context['node']->title;

  // Change Stripe Checkout currency based on a "Country" component value.
  if (isset($context['form']['country']) && $context['form']['country'] === 'us') {
    $settings['currency'] = 'USD';
  }

  // Dynamically calculate amount with custom logic. Don't forget about the
  // amount multiplier!
  $settings['amount'] = ((int) $context['form']['amount'] * (int) $context['form']['qty']) * $context['component']['extra']['amount_value_multiplier'];

  // Do something only at checkout but not when processing Stripe token.
  if ($context['state'] === WEBFORM_STRIPE_SETTINGS_CHECKOUT) {
    // Can't think of an example!
  }

  // Do something only when processing Stripe token.
  else if ($context['state'] === WEBFORM_STRIPE_SETTINGS_PROCESS) {
    // Set current user ID in Stripe metadata.
    $settings['metadata']['uid'] = user_uid_optional_load()->uid;
  }
}
