<?php
/**
 * @file
 * Contains forms/functions for admin use.
 */

/**
 * Stripe API admin settings form.
 */
function stripe_api_admin_settings_form($form, $form_state) {
  $form['stripe_api_link'] = array(
    '#markup' => t('!dashboard | !api_keys | !docs<br /><br />', array(
      '!dashboard' => l(t('Dashboard'), 'https://dashboard.stripe.com', array('attributes' => array('target' => '_blank'))),
      '!api_keys' => l(t('API Keys'), 'https://dashboard.stripe.com/account/apikeys', array('attributes' => array('target' => '_blank'))),
      '!docs' => l(t('Docs'), 'https://stripe.com/docs/api', array('attributes' => array('target' => '_blank'))),
    )),
  );
  $form['stripe_api_test_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Secret Key (test)'),
    '#default_value' => variable_get('stripe_api_test_secret_key', ''),
  );
  $form['stripe_api_test_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Public Key (test)'),
    '#default_value' => variable_get('stripe_api_test_public_key', ''),
  );
  $form['stripe_api_live_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Secret Key (live)'),
    '#default_value' => variable_get('stripe_api_live_secret_key', ''),
  );
  $form['stripe_api_live_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Public Key (live)'),
    '#default_value' => variable_get('stripe_api_live_public_key', ''),
  );
  $form['stripe_api_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Mode'),
    '#options' => array(
      'test' => t('Test'),
      'live' => t('Live'),
    ),
    '#default_value' => variable_get('stripe_api_mode', 'test'),
  );

  $form['stripe_api_webhook_url'] = array(
    '#type' => 'textfield',
    '#disabled' => TRUE,
    '#title' => t('Webhook URL'),
    '#default_value' => url('stripe/webhook', array('absolute' => TRUE)),
    '#description' => t('Add this webhook path in the !link', array(
      '!link' => l(t('Stripe Dashboard'), 'https://dashboard.stripe.com/account/webhooks', array('attributes' => array('target' => '_blank'))),
    )),
  );

  $form['stripe_api_log_webhooks'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log incoming webhooks'),
    '#default_value' => variable_get('stripe_api_log_webhooks', TRUE),
  );

  if (_stripe_api_secret_key()) {
    $form['stripe_test'] = array(
      '#type' => 'button',
      '#value' => t('Test Stripe Connection'),
      '#ajax' => array(
        'callback' => '_stripe_api_admin_test_connection',
        'wrapper' => 'stripe-connect-results',
        'method' => 'append',
      ),
      '#suffix' => '<div id="stripe-connect-results"></div>',
    );
  }

  return system_settings_form($form);
}

/**
 * AJAX callback to test the Stripe connection.
 */
function _stripe_api_admin_test_connection($form, $form_state) {
  $account = stripe_api_call('account', 'retrieve');
  if ($account && $account->email) {
    return 'Success! Account email: ' . $account->email;
  }
  else {
    return 'Error! Could not connect!';
  }
}
