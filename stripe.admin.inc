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

  $form['api'] = array(
    '#type' => 'fieldset',
    '#title' => t('Stripe API Settings'),
  );

  $form['api']['mode'] = array(
    '#type' => 'radios',
    '#title' => t('Transaction mode'),
    '#options' => array(
      'test' => t('Test'),
      'live' => t('Live'),
    ),
    '#default_value' => $config->get('mode'),
  );
  $form['api']['test_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Test Publishable Key'),
    '#default_value' => $config->get('test_public_key'),
  );
  $form['api']['test_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Test Secret Key'),
    '#default_value' => $secretconfig->get('test_secret_key'),
  );
  $form['api']['live_public_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Publishable Key'),
    '#default_value' => $config->get('live_public_key'),
  );
  $form['api']['live_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Secret Key'),
    '#default_value' => $secretconfig->get('live_secret_key'),
  );

  $form['api']['webhook_url'] = array(
    '#type' => 'textfield',
    '#disabled' => TRUE,
    '#title' => t('Webhook URL'),
    '#default_value' => url('stripe/webhook', array('absolute' => TRUE)),
    '#description' => t('Add this webhook path in the !link', array(
      '!link' => l(t('Stripe Dashboard'), 'https://dashboard.stripe.com/account/webhooks', array('attributes' => array('target' => '_blank'))),
    )),
  );

  $form['api']['log_webhooks'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log incoming webhooks'),
    '#default_value' => $config->get('log_webhooks'),
  );

  $form['api']['link'] = array(
    '#markup' => t('Stripe resources: !dashboard | !api_keys | !docs<br /><br />', array(
      '!dashboard' => l(t('Dashboard'), 'https://dashboard.stripe.com', array('attributes' => array('target' => '_blank'))),
      '!api_keys' => l(t('API Keys'), 'https://dashboard.stripe.com/account/apikeys', array('attributes' => array('target' => '_blank'))),
      '!docs' => l(t('Docs'), 'https://stripe.com/docs/api', array('attributes' => array('target' => '_blank'))),
    )),
  );

  if (stripe_get_secret_key()) {
    $form['api']['stripe_test'] = array(
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

  // Checkout settings.
  $form['checkout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Checkout Settings'),
    '#tree' => TRUE,
  );
  $default_name = $config->get('name');
  if ($default_name == '') {
    $default_name = config_get('system.core', 'site_name');
  }
  $form['checkout']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Company or Website name'),
    '#default_value' => $default_name,
  );
  $form['checkout']['currency'] = array(
    '#type' => 'textfield',
    '#title' => t('Currency'),
    '#size' => 10,
    '#description' => t('Enter the 3-letter <a href="!url">ISO code</a>.', array(
      '!url' => 'https://support.stripe.com/questions/which-currencies-does-stripe-support',
    )),
    '#default_value' => $config->get('currency'),
    '#required' => TRUE,
  );
  if (module_exists('locale')) {
    $form['checkout']['locale'] = array(
      '#type' => 'select',
      '#title' => t('Locale'),
      '#description' => t('Specify "auto" to display Checkout in the <a href="!url">user\'s preferred language</a>, if available.', array(
        '!url' => 'https://support.stripe.com/questions/what-languages-does-stripe-checkout-support',
      )),
      '#default_value' => $config->get('locale'),
      '#options' => array(
        'auto' => t('auto'),
        'zh' => t('Chinese'),
        'nl' => t('Dutch'),
        'en' => t('English'),
        'fr' => t('French'),
        'de' => t('German'),
        'it' => t('Italian'),
        'ja' => t('Japanese'),
        'es' => t('Spanish'),
      ),
    );
  }
  else {
    $form['checkout']['locale'] = array(
      '#type' => 'value',
      '#value' => 'auto',
    );
  }
  $form['checkout']['zip_code'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate the billing ZIP code'),
    '#default_value' => $config->get('zip_code'),
  );
  $form['checkout']['billing_address'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collect billing address'),
    '#default_value' => $config->get('billing_address'),
  );
  $form['checkout']['shipping_address'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collect shipping address'),
    '#description' => t('This information will be available in the Stripe dashboard payment transaction.'),
    '#default_value' => $config->get('shipping_address'),
  );
  $form['checkout']['allow_remember_me'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include option to "Remember Me" for future purchases'),
    '#default_value' => $config->get('allow_remember_me'),
  );
  $form['checkout']['bitcoin'] = array(
    '#type' => 'checkbox',
    '#title' => t('Accept Bitcoin payments'),
    '#default_value' => $config->get('bitcoin'),
  );
  $form['checkout']['alipay'] = array(
    '#type' => 'checkbox',
    '#title' => t('Accept Alipay payments'),
    '#default_value' => $config->get('alipay'),
  );
  $form['checkout']['alipay_reusable'] = array(
    '#type' => 'checkbox',
    '#title' => t("Request reusable access to customer's Alipay account"),
    '#default_value' => $config->get('alipay_reusable'),
    '#states' => array(
      'visible' => array(
        ':input[name="webform[alipay]"]' => array('checked' => TRUE),
      ),
    ),
  );

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
  $config->set('log_webhooks' , $values['log_webhooks']);
  $config->set('name' , $values['checkout']['name']);
  $config->set('currency' , $values['checkout']['currency']);
  $config->set('locale' , $values['checkout']['locale']);
  $config->set('zip_code' , $values['checkout']['zip_code']);
  $config->set('billing_address' , $values['checkout']['billing_address']);
  $config->set('shipping_address' , $values['checkout']['shipping_address']);
  $config->set('allow_remember_me' , $values['checkout']['allow_remember_me']);
  $config->set('bitcoin' , $values['checkout']['bitcoin']);
  $config->set('alipay' , $values['checkout']['alipay']);
  $config->set('alipay_reusable' , $values['checkout']['alipay_reusable']);
  $config->save();

  $secretconfig->set('test_secret_key' , $values['test_secret_key']);
  $secretconfig->set('live_secret_key' , $values['live_secret_key']);
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
