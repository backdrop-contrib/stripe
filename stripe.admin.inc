<?php
/**
 * @file
 * Contains forms/functions for admin use.
 */

/**
 * Stripe API admin settings form.
 */
function stripe_admin_settings_form($form, $form_state) {
  $config = config('stripe.settings');
  $secretconfig = config('stripe_secret.settings');
  $form['mode'] = array(
    '#type' => 'radios',
    '#title' => t('Transaction mode'),
    '#options' => array(
      'test' => t('Test'),
      'live' => t('Live'),
    ),
    '#default_value' => $config->get('mode'),
  );
  $form['test_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Test Secret Key'),
    '#default_value' => $secretconfig->get('test_secret_key'),
  );
  $form['test_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Test Publishable Key'),
    '#default_value' => $config->get('test_public_key'),
  );
  $form['live_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Secret Key'),
    '#default_value' => $secretconfig->get('live_secret_key'),
  );
  $form['live_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Publishable Key'),
    '#default_value' => $config->get('live_public_key'),
  );

  $form['webhook_url'] = array(
    '#type' => 'textfield',
    '#disabled' => TRUE,
    '#title' => t('Webhook URL'),
    '#default_value' => url('stripe/webhook', array('absolute' => TRUE)),
    '#description' => t('Add this webhook path in the !link', array(
      '!link' => l(t('Stripe Dashboard'), 'https://dashboard.stripe.com/account/webhooks', array('attributes' => array('target' => '_blank'))),
    )),
  );

  $form['log_webhooks'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log incoming webhooks'),
    '#default_value' => $config->get('log_webhooks'),
  );

  $form['link'] = array(
    '#markup' => t('Stripe resources: !dashboard | !api_keys | !docs<br /><br />', array(
      '!dashboard' => l(t('Dashboard'), 'https://dashboard.stripe.com', array('attributes' => array('target' => '_blank'))),
      '!api_keys' => l(t('API Keys'), 'https://dashboard.stripe.com/account/apikeys', array('attributes' => array('target' => '_blank'))),
      '!docs' => l(t('Docs'), 'https://stripe.com/docs/api', array('attributes' => array('target' => '_blank'))),
    )),
  );

  if (stripe_get_secret_key()) {
    $form['stripe_test'] = array(
      '#type' => 'button',
      '#value' => t('Test Stripe Connection'),
      '#ajax' => array(
        'callback' => 'stripe_test_connection',
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
 * Submit handler for stripe_checkout_admin_settings_form().
 */
function stripe_admin_settings_form_submit($form, &$form_state) {
  $config = config('stripe.settings');
  $secretconfig = config('stripe_secret.settings');
  $values = $form_state['values'];

  $config->set('mode' , $values['mode']);
  $config->set('test_public_key' , $values['test_public_key']);
  $config->set('live_public_key' , $values['live_public_key']);
  $secretconfig->set('test_secret_key' , $values['test_secret_key']);
  $secretconfig->set('live_secret_key' , $values['live_secret_key']);
  $config->set('log_webhooks' , $values['log_webhooks']);
  $config->save();
  $secretconfig->save();

  backdrop_set_message(t('Stripe API settings have been saved.'));
}

/**
 * AJAX callback to test the Stripe connection.
 */
function stripe_test_connection($form, $form_state) {
  $account = stripe_api_call('account', 'retrieve');
  if ($account && $account->email) {
    return 'Success! Account email: ' . $account->email;
  }
  else {
    return 'Error! Could not connect!';
  }
}
