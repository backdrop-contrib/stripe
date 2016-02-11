<?php

/**
 * Implements _webform_defaults_component().
 */
function _webform_defaults_webform_stripe() {
  return array(
    'name' => '',
    'form_key' => NULL,
    'pid' => 0,
    'weight' => 0,
    'value' => '',
    'required' => 0,
    'extra' => array(
      'webform_stripe' => array(
        'amount' => 0,
        'description' => '',
        'panel-label' => t('Pay {{amount}}'),
        'email' => TRUE,
      ),
    ),
  );
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_webform_stripe($component) {
  // Show notification if keys aren't configured.
  if (!variable_get('webform_stripe_secret_key', '') || !variable_get('webform_stripe_publishable_key', '')) {
    drupal_set_message(t('You need to <a href="!url">configure your Stripe settings</a>.', array(
      '!url' => url('admin/config/services/webform-stripe'),
    )), 'warning');
  }

  $form['webform_stripe'] = array(
    '#type' => 'fieldset',
    '#title' => t('Stripe'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#parents' => array('extra', 'webform_stripe'),
  );
  $form['webform_stripe']['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount'),
    '#description' => t("The amount (in cents) that's shown to the user."),
    '#default_value' => $component['extra']['webform_stripe']['amount'],
    '#required' => TRUE,
  );
  $form['webform_stripe']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('A description of the product or service being purchased.'),
    '#default_value' => $component['extra']['webform_stripe']['description'],
  );
  $form['webform_stripe']['panel-label'] = array(
    '#type' => 'textfield',
    '#title' => t('Panel label'),
    '#description' => t('The label of the payment button in the payment form (e.g. “Subscribe”, “Pay {{amount}}”, etc.). If you include {{amount}}, it will be replaced by the provided amount. Otherwise, the amount will be appended to the end of your label.'),
    '#default_value' => $component['extra']['webform_stripe']['panel-label'],
  );
  $form['webform_stripe']['email'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use email address of logged-in user'),
    '#default_value' => $component['extra']['webform_stripe']['email'],
  );

  $form['#validate'][] = '_webform_edit_webform_stripe_validate';
  return $form;
}

/**
 * Edit validate callback.
 */
function _webform_edit_webform_stripe_validate($form, &$form_state) {
  // Ensure positive amount.
  if ($form_state['values']['extra']['webform_stripe']['amount'] <= 0) {
    form_set_error('extra][webform_stripe][amount', t('Enter a positive amount (in cents).'));
  }
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_webform_stripe($component, $value = NULL, $filter = TRUE, $submission = NULL) {
  // If a payment is already saved, don't attach anything to the form, and
  // preserve the stored value.
  if (isset($value[0]) && webform_stripe_is_charge($value[0])) {
    return array(
      '#type' => 'value',
      '#default_value' => $value,
      '#stripe_webform' => FALSE,
    );
  }

  // At this point, this is a hidden field intended to store the Stripe token
  // and email. After saving, it will store the Stripe charge ID.
  $element = array(
    '#type' => 'hidden',
    '#attributes' => array('class' => 'webform-stripe-token'),
    '#theme_wrappers' => array('webform_element'),
    '#weight' => $component['weight'],
    '#attached' => array(
      'js' => array(
        'https://checkout.stripe.com/checkout.js',
        drupal_get_path('module', 'webform_stripe') . '/webform_stripe.js',
      ),
    ),
  );
  return $element;
}

/**
 * Implements _webform_submit_component().
 */
function _webform_submit_webform_stripe($component, $value) {
  // Nothing to process if we don't have a token.
  if (!is_string($value) || substr($value, 0, 4) !== 'tok_') {
    return $value;
  }

  // Parse Stripe token and email.
  list($token, $email) = explode(':', $value);

  // Include Stripe library.
  libraries_load('stripe-php');

  // Set secret key.
  \Stripe\Stripe::setApiKey(variable_get('webform_stripe_secret_key', ''));

  // Charge card.
  try {
    $checkout_settings = webform_stripe_checkout_settings();
    $charge = \Stripe\Charge::create(array(
      'source' => $token,
      'amount' => $component['extra']['webform_stripe']['amount'],
      'description' => $component['extra']['webform_stripe']['description'],
      'currency' => $checkout_settings['currency'],
      'receipt_email' => $email,
    ));

    // Store charge ID.
    $value = $charge->id;
  }

  // Card was declined.
  catch(\Stripe\Error\Card $e) {
    drupal_set_message(t('The payment was unsuccessful.'), 'error');

    // Remove token, so we don't try to charge this card again.
    $value = '';
  }

  return $value;
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_webform_stripe($component, $value, $format = 'html') {
  if (isset($value[0]) && webform_stripe_is_charge($value[0])) {
    return array(
      '#type' => 'markup',
      '#title' => $component['name'],
      '#markup' => l(t('view payment'), 'https://dashboard.stripe.com/test/payments/' . $value[0]),
      '#weight' => $component['weight'],
      '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
      '#translatable' => array('title'),
    );
  }
}

/**
 * Implements _webform_table_component().
 */
function _webform_table_webform_stripe($component, $value) {
  return webform_stripe_is_charge($value[0]) ? l(t('view payment'), 'https://dashboard.stripe.com/test/payments/' . $value[0]) : '';
}

/**
 * Implements _webform_csv_headers_component().
 */
function _webform_csv_headers_webform_stripe($component, $export_options) {
  $header = array();
  $header[0] = '';
  $header[1] = '';
  $header[2] = $export_options['header_keys'] ? $component['form_key'] : $component['name'];
  return $header;
}

/**
 * Implements _webform_csv_data_component().
 */
function _webform_csv_data_webform_stripe($component, $export_options, $value) {
  return webform_stripe_is_charge($value[0]) ? t('yes') : t('no');
}
