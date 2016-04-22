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
      'amount_type' => 'value',
      'amount_component' => '',
      'amount_mapping' => array(),
      'amount_multiplier' => 100,
      'description_type' => 'value',
      'description_component' => '',
      'description_mapping' => array(),
      'continue_text' => '',
      'pay_text' => '',
      'cancel_text' => '',
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

  $node = node_load($component['nid']);

  // Prevent multiple Stripe payment components per webform.
  if (empty($component['cid'])) {
    foreach ($node->webform['components'] as $other_component) {
      if ($other_component['type'] === 'webform_stripe') {
        drupal_set_message(t('Only one Stripe payment component is allowed.'), 'error');
        drupal_goto("node/$node->nid/webform/components");
      }
    }
  }

  // Amount.
  $form['amount'] = array(
    '#type' => 'fieldset',
    '#title' => t('Amount'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#parents' => array('extra'),
  );
  $form['amount']['amount_type'] = array(
    '#type' => 'radios',
    '#title' => t('Amount'),
    '#options' => array(
      'value' => t('Fixed'),
      'component' => t('Dynamic'),
    ),
    '#default_value' => $component['extra']['amount_type'],
    '#required' => TRUE,
  );
  $form['amount']['amount'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed amount'),
    '#description' => t("The amount (in cents) that will be charged."),
    '#default_value' => $component['extra']['webform_stripe']['amount'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'value'),
      ),
    ),
    '#parents' => array('extra', 'webform_stripe', 'amount'),
  );
  $amount_components = array();
  foreach ($node->webform['components'] as $other_component) {
    if (in_array($other_component['type'], array('textfield', 'number', 'select'))) {
      if ($other_component['type'] === 'select' && $other_component['extra']['multiple']) {
        continue;
      }
      $amount_components[$other_component['cid']] = check_plain($other_component['name']);
    }
  }
  $form['amount']['amount_component'] = array(
    '#type' => 'select',
    '#title' => t('Dynamic amount'),
    '#description' => t("Sets the component which will determine the amount charged. Eligible components include: Textfield, Number, Select options (not multiple)."),
    '#options' => $amount_components,
    '#empty_option' => '- ' . t('Select a component') . ' -',
    '#default_value' => $component['extra']['amount_component'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'component'),
      ),
    ),
  );
  $form['amount']['amount_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Amount options'),
    '#description' => t('The selected amount component has multiple options. You may enter an amount for each choice.'),
    '#prefix' => '<div id="webform-stripe-amount-mapping">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'component'),
      ),
    ),
  );
  $form['amount']['amount_multiplier'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount multiplier'),
    '#description' => t("Optionally multiply the amount by this value. This is useful if you don't want to deal with cents here, or require your users to enter values in cents on the form."),
    '#default_value' => $component['extra']['amount_multiplier'],
    '#size' => 10,
  );

  // Description.
  $form['description'] = array(
    '#type' => 'fieldset',
    '#title' => t('Description'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#parents' => array('extra'),
  );
  $form['description']['description_type'] = array(
    '#type' => 'radios',
    '#title' => t('Description'),
    '#options' => array(
      'value' => t('Fixed'),
      'component' => t('Dynamic'),
    ),
    '#default_value' => $component['extra']['description_type'],
    '#required' => TRUE,
    '#parents' => array('extra', 'description_type'),
  );
  $form['description']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed description'),
    '#description' => t('A description of the product or service being purchased.'),
    '#default_value' => $component['extra']['webform_stripe']['description'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[description_type]"]' => array('value' => 'value'),
      ),
    ),
    '#parents' => array('extra', 'webform_stripe', 'description'),
  );
  $description_components = array();
  foreach ($node->webform['components'] as $other_component) {
    if (in_array($other_component['type'], array('textfield', 'select'))) {
      if ($other_component['type'] === 'select' && $other_component['extra']['multiple']) {
        continue;
      }
      $description_components[$other_component['cid']] = check_plain($other_component['name']);
    }
  }
  $form['description']['description_component'] = array(
    '#type' => 'select',
    '#title' => t('Dynamic description'),
    '#description' => t("Sets the component which will determine the description. Eligible components include: Textfield, Select options (not multiple)."),
    '#options' => $description_components,
    '#empty_option' => '- ' . t('Select a component') . ' -',
    '#default_value' => $component['extra']['description_component'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[description_type]"]' => array('value' => 'component'),
      ),
    ),
    '#parents' => array('extra', 'description_component'),
  );
  $form['description']['description_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Description options'),
    '#description' => t('The selected description component has multiple options. You may enter a description for each choice. If a field is left blank, the selected option will be used as the description.'),
    '#prefix' => '<div id="webform-stripe-description-mapping">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="extra[description_type]"]' => array('value' => 'component'),
      ),
    ),
  );

  // Additional settings.
  $form['webform_stripe'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#parents' => array('extra', 'webform_stripe'),
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

  // Button texts.
  $form['display']['continue_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Continue button label'),
    '#description' => t('This is used for the button which validates the form and presents the <em>Pay</em> and <em>Cancel</em> buttons. Default: %default', array(
      '%default' => WEBFORM_STRIPE_CONTINUE_TEXT,
    )),
    '#default_value' => $component['extra']['continue_text'],
    '#parents' => array('extra', 'continue_text'),
  );
  $form['display']['pay_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Pay button label'),
    '#description' => t('This is used for the button which opens the Stripe Checkout payment form. Default: <em>%default</em>', array(
      '%default' => WEBFORM_STRIPE_PAY_TEXT,
    )),
    '#default_value' => $component['extra']['pay_text'],
    '#parents' => array('extra', 'pay_text'),
  );
  $form['display']['cancel_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Cancel button label'),
    '#description' => t('This is used for the button which cancels payment and returns to editing the form. Default: <em>%default</em>', array(
      '%default' => WEBFORM_STRIPE_CANCEL_TEXT,
    )),
    '#default_value' => $component['extra']['cancel_text'],
    '#parents' => array('extra', 'cancel_text'),
  );

  $form['#validate'][] = '_webform_edit_webform_stripe_validate';
  return $form;
}

/**
 * Edit validate callback.
 */
function _webform_edit_webform_stripe_validate($form, &$form_state) {
  // Manual amount.
  if ($form_state['values']['extra']['amount_type'] === 'value') {
    // Ensure positive amount.
    if (empty($form_state['values']['extra']['webform_stripe']['amount']) || $form_state['values']['extra']['webform_stripe']['amount'] <= 0) {
      form_set_error('extra][webform_stripe][amount', t('Enter a positive amount (in cents).'));
    }

    // Ensure amount is greater than Stripe's minimum charge amount.
    // https://support.stripe.com/questions/what-is-the-minimum-amount-i-can-charge-with-stripe
    $settings = webform_stripe_checkout_settings();
    $minimum = array(
      'USD' => 50,
      'CAD' => 50,
      'GBP' => 30,
      'EUR' => 50,
      'DKK' => 2.5,
      'NOK' => 3,
      'SEK' => 3,
      'CHF' => 50,
      'AUD' => 50,
      'JPY' => 50,
      'MXN' => 10,
      'SGD' => 50,
    );
    if (array_key_exists($settings['currency'], $minimum) && ($form_state['values']['extra']['webform_stripe']['amount'] * $form_state['values']['extra']['amount_multiplier']) < $minimum[$settings['currency']]) {
      form_set_error('extra][webform_stripe][amount', t('The minimum amount for %currency is %minimum (cents).', array(
        '%currency' => $settings['currency'],
        '%minimum' => $minimum[$settings['currency']],
      )));
    }
  }

  // Component amount.
  if ($form_state['values']['extra']['amount_type'] === 'component') {
    // Component field is required.
    if (empty($form_state['values']['extra']['amount_component'])) {
      form_set_error('extra][amount_component', t('Choose a component to use for the amount.'));
    }

    // All amount mappings must be set.
    if (!empty($form_state['values']['extra']['amount_mapping'])) {
      foreach ($form_state['values']['extra']['amount_mapping'] as $key => $value) {
        if (empty($value)) {
          form_set_error('extra][amount_mapping', t('Enter an amount for each component option.'));
        }
      }
    }
  }

  // Component multiplier must be numeric.
  if (!empty($form_state['values']['extra']['amount_multiplier']) && !is_numeric($form_state['values']['extra']['amount_multiplier'])) {
    form_set_error('extra][amount_multiplier', t('Enter a numeric amount multiplier.'));
  }

  // Component description.
  if ($form_state['values']['extra']['description_type'] === 'component' && empty($form_state['values']['extra']['description_component'])) {
    form_set_error('extra][description_component', t('Choose a component to use for the description.'));
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
        'https://checkout.stripe.com/checkout.js' => array('type' => 'external'),
        drupal_get_path('module', 'webform_stripe') . '/webform_stripe.js',
      ),
    ),
  );
  return $element;
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
