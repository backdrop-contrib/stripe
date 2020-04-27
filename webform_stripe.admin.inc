<?php

/**
 * Settings form.
 */
function webform_stripe_settings_form() {
  // Keys.
  $form['keys'] = array(
    '#type' => 'fieldset',
    '#title' => t('Stripe API Keys'),
    '#description' => t('The API keys from <a href="!url">your Stripe dashboard</a>.', array(
      '!url' => 'https://dashboard.stripe.com/account/apikeys',
    )),
  );
  $form['keys']['webform_stripe_test_publishable_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Test Publishable Key'),
    '#default_value' => variable_get('webform_stripe_test_publishable_key', ''),
    '#required' => TRUE,
  );
  $form['keys']['webform_stripe_test_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Test Secret Key'),
    '#default_value' => variable_get('webform_stripe_test_secret_key', ''),
    '#required' => TRUE,
  );
  $form['keys']['webform_stripe_live_publishable_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Publishable Key'),
    '#default_value' => variable_get('webform_stripe_live_publishable_key', ''),
    '#required' => TRUE,
  );
  $form['keys']['webform_stripe_live_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Live Secret Key'),
    '#default_value' => variable_get('webform_stripe_live_secret_key', ''),
    '#required' => TRUE,
  );

  // Image type.
  $form['image'] = array(
    '#type' => 'fieldset',
    '#title' => t('Default image'),
  );
  $form['image']['webform_stripe_default_image_type'] = array(
    '#type' => 'radios',
    '#title' => t('Image type'),
    '#options' => array(
      'none' => t('No image'),
      'file' => t('Custom image upload'),
      'url' => t('Custom image URL'),
    ),
    '#default_value' => variable_get('webform_stripe_default_image_type', 'none'),
    '#required' => TRUE,
  );

  // Image upload.
  $form['image']['webform_stripe_default_image_file'] = array(
    '#type' => 'managed_file',
    '#title' => t('Image upload'),
    '#description' => t('Select an png, jpg, or gif image that with a minimum size of 128x128px.'),
    '#default_value' => variable_get('webform_stripe_default_image_file', ''),
    '#upload_validators' => array(
      'file_validate_image_resolution' => array(0, '128x128'),
      'file_validate_extensions' => array('png jpg gif'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="webform_stripe_default_image_type"]' => array('value' => 'file'),
      ),
    ),
  );
  if (module_exists('media')) {
    $form['image']['webform_stripe_default_image_file']['#type'] = 'media';
  }

  // Image URL.
  $form['image']['webform_stripe_default_image_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Image URL'),
    '#description' => t('Enter an external URL or an internal path to a square image of your brand or product. It should be a png, jpg, or gif image with a minimum size of 128x128px.'),
    '#default_value' => variable_get('webform_stripe_default_image_url', ''),
    '#states' => array(
      'visible' => array(
        ':input[name="webform_stripe_default_image_type"]' => array('value' => 'url'),
      ),
    ),
  );

  // Checkout settings.
  $checkout_settings = webform_stripe_checkout_settings();
  $form['webform_stripe_checkout_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Checkout settings'),
    '#tree' => TRUE,
  );
  $form['webform_stripe_checkout_settings']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('The name of your company or website.'),
    '#default_value' => $checkout_settings['name'],
  );
  $form['webform_stripe_checkout_settings']['currency'] = array(
    '#type' => 'textfield',
    '#title' => t('Currency'),
    '#description' => t('The currency of the amount (3-letter <a href="!url">ISO code</a>).', array(
      '!url' => 'https://support.stripe.com/questions/which-currencies-does-stripe-support',
    )),
    '#default_value' => $checkout_settings['currency'],
    '#required' => TRUE,
  );
  $form['webform_stripe_checkout_settings']['locale'] = array(
    '#type' => 'select',
    '#title' => t('Locale'),
    '#description' => t('Specify "auto" to display Checkout in the <a href="!url">user\'s preferred language</a>, if available.', array(
      '!url' => 'https://support.stripe.com/questions/what-languages-does-stripe-checkout-support',
    )),
    '#default_value' => $checkout_settings['locale'],
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
  $form['webform_stripe_checkout_settings']['zip-code'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate the billing ZIP code'),
    '#default_value' => $checkout_settings['zip-code'],
  );
  $form['webform_stripe_checkout_settings']['billing-address'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collect billing address'),
    '#default_value' => $checkout_settings['billing-address'],
  );
  $form['webform_stripe_checkout_settings']['shipping-address'] = array(
    '#type' => 'checkbox',
    '#title' => t('Collect shipping address'),
    '#description' => t('This information will be available in the Stripe dashboard payment transaction.'),
    '#default_value' => $checkout_settings['shipping-address'],
  );
  $form['webform_stripe_checkout_settings']['allow-remember-me'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include option to "Remember Me" for future purchases'),
    '#default_value' => $checkout_settings['allow-remember-me'],
  );
  $form['webform_stripe_checkout_settings']['bitcoin'] = array(
    '#type' => 'checkbox',
    '#title' => t('Accept Bitcoin payments'),
    '#default_value' => $checkout_settings['bitcoin'],
  );
  $form['webform_stripe_checkout_settings']['alipay'] = array(
    '#type' => 'checkbox',
    '#title' => t('Accept Alipay payments'),
    '#default_value' => $checkout_settings['alipay'],
  );
  $form['webform_stripe_checkout_settings']['alipay-reusable'] = array(
    '#type' => 'checkbox',
    '#title' => t("Request reusable access to customer's Alipay account"),
    '#default_value' => $checkout_settings['alipay-reusable'],
    '#states' => array(
      'visible' => array(
        ':input[name="webform_stripe_checkout_settings[alipay]"]' => array('checked' => TRUE),
      ),
    ),
  );
  return system_settings_form($form);
}
