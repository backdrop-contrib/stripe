<?php
/**
 * Stripe hook examples.
 */

/**
 * Interact with all incoming Stripe webhooks.
 *
 * @param string $type
 *   Webhook type, such as customer.created, charge.captured, etc.
 * @param object $data
 *   Incoming data object.
 * @param Stripe\Event $event
 *   The verified Stripe Event that is being sent.
 *   Only available in live mode with real events (not test events).
 *
 * @throws \Exception
 */
function hook_stripe_webhook($type, $data, Stripe\Event $event = NULL) {
  switch ($type) {
    case 'customer.created':
      // Create a new Backdrop user for this customer.
      $email = $data->object->email;
      if (!user_load_by_mail($email)) {
        user_save(NULL, array(
          'name' => $data->object->id,
          'mail' => $email,
        ));
      }
      break;
  }
}

/**
 * Interact with a specific incoming webhook type.
 *
 * @param object $data
 *   Incoming data object.
 * @param Stripe\Event $event
 *   The verified Stripe Event that is being sent.
 *   Only available in live mode with real events (not test events).
 *
 * @throws \Exception
 */
function hook_stripe_webhook_EVENT_TYPE($data, Stripe\Event $event = NULL) {
  $email = $data->object->email;
  if (!user_load_by_mail($email)) {
    user_save(NULL, array(
      'name' => $data->object->id,
      'mail' => $email,
    ));
  }
}

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
 *     - STRIPE_SETTINGS_CHECKOUT: Before opening Stripe Checkout.
 *       The 'data' array is keyed by component's 'form_key'.
 *     - STRIPE_SETTINGS_PROCESS: After user submits Stripe Checkout
 *       form and before server processes Stripe charge/subscription.
 *       The 'data' array is keyed by component's 'cid'.
 */
function hook_stripe_checkout_settings_alter(&$settings, &$context) {
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
  if ($context['state'] === STRIPE_SETTINGS_CHECKOUT) {
    // Can't think of an example.
  }

  // Do something only when processing Stripe token.
  else if ($context['state'] === STRIPE_SETTINGS_PROCESS) {
    // Set current user ID in Stripe metadata.
    $settings['metadata']['uid'] = user_uid_optional_load()->uid;
  }
}
