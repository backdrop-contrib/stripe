<?php
/**
 * @file
 * Contains forms/functions for admin use.
 */

/**
 * Stripe API admin settings form.
 */
function stripe_api_admin_settings_form($form, $form_state) {
  $config = config('stripe_api.settings');
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
    '#default_value' => $config->get('stripe_api_test_secret_key'),
  );
  $form['stripe_api_test_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Public Key (test)'),
    '#default_value' => $config->get('stripe_api_test_public_key'),
  );
  $form['stripe_api_live_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Secret Key (live)'),
    '#default_value' => $config->get('stripe_api_live_secret_key'),
  );
  $form['stripe_api_live_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Stripe Public Key (live)'),
    '#default_value' => $config->get('stripe_api_live_public_key'),
  );
  $form['stripe_api_mode'] = array(
    '#type' => 'radios',
    '#title' => t('Mode'),
    '#options' => array(
      'test' => t('Test'),
      'live' => t('Live'),
    ),
    '#default_value' => $config->get('stripe_api_mode'),
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
    '#default_value' => $config->get('stripe_api_log_webhooks'),
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

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  
  return $form;
}

/**
 * Submit handler for stripe_checkout_admin_form().
 */
function stripe_checkout_admin_form_submit($form, &$form_state) {
  $config = config('stripe_api.settings');
  $secretconfig = config('stripe_api_secret.settings');
  $values = $form_state['values'];

  $config->set('stripe_api_mode' , $values['stripe_api_mode']);
  $config->set('stripe_api_test_public_key' , $values['stripe_api_test_public_key']);
  $config->set('stripe_api_live_public_key' , $values['stripe_api_live_public_key']);
  $secretconfig->set('stripe_api_test_secret_key' , $values['stripe_api_test_secret_key']);
  $secretconfig->set('stripe_api_live_secret_key' , $values['stripe_api_live_secret_key']);
  $config->set('stripe_api_log_webhooks' , $values['stripe_api_log_webhooks']);
  $config->save();
  $secretconfig->save();

  backdrop_set_message(t('Stripe API settings have been saved.'));
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
