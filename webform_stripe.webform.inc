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
      'test_mode' => FALSE,
      'amount_type' => 'value',
      'amount_value' => '',
      'amount_value_component' => '',
      'amount_value_mapping' => array(),
      'amount_value_multiplier' => 100,
      'amount_plan' => '',
      'amount_plan_component' => '',
      'amount_plan_mapping' => array(),
      'description_type' => 'value',
      'description_value' => '',
      'description_value_component' => '',
      'description_value_mapping' => array(),
      'panel_label' => t('Pay {{amount}}'),
      'email_type' => 'user',
      'email_component' => '',
      'image_type' => 'default',
      'image_file' => '',
      'image_url' => '',
      'continue_text' => '',
      'pay_text' => '',
      'cancel_text' => '',
    ),
  );
}

/**
 * Gets Stripe plans.
 */
function _webform_stripe_get_plans($test_mode) {
  $plans = array();
  $secret_key = $test_mode ? variable_get('webform_stripe_test_secret_key', '') : variable_get('webform_stripe_live_secret_key', '');

  // Load list of Stripe plans.
  if (($library = webform_stripe_load_library()) && $library['loaded'] && !empty($secret_key)) {
    \Stripe\Stripe::setApiKey($secret_key);
    try {
      foreach (\Stripe\Plan::all()->data as $plan) {
        $plans[$plan->id] = $plan->name;
      }
    }
    catch(\Stripe\Error\Base $e) {

    }
  }

  return $plans;
}

/**
 * Implements _webform_edit_component().
 */
function _webform_edit_webform_stripe($component) {
  // Show notification if keys aren't configured.
  if (!variable_get('webform_stripe_test_secret_key', '') || !variable_get('webform_stripe_test_publishable_key', '') || !variable_get('webform_stripe_live_secret_key', '') || !variable_get('webform_stripe_live_publishable_key', '')) {
    drupal_set_message(t('You need to <a href="!url">configure your Stripe API keys</a>.', array(
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

  // Test mode.
  $form['test_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Test mode'),
    '#description' => t('When enabled, the test API keys are used and the payment will not actually be charged. You can use <a href="!url">these test credit cards</a>.', array(
      '!url' => url('https://stripe.com/docs/testing#cards'),
    )),
    '#default_value' => $component['extra']['test_mode'],
    '#parents' => array('extra', 'test_mode'),
  );

  // Amount type.
  $form['amount'] = array(
    '#type' => 'fieldset',
    '#title' => t('Amount'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="webform-stripe-amount">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#parents' => array('extra'),
  );
  $form['amount']['amount_type'] = array(
    '#type' => 'radios',
    '#title' => t('Amount type'),
    '#description' => t('Choose "amount" for a one-time charge, or "subscription" for a recurring charge. Choose "fixed" to manually set the amount/plan, or "dynamic" to determine it from another component on this form.'),
    '#options' => array(
      'value' => t('Fixed amount'),
      'value_component' => t('Dynamic amount'),
      'plan' => t('Fixed subscription'),
      'plan_component' => t('Dynamic subscription'),
    ),
    '#default_value' => $component['extra']['amount_type'],
    '#required' => TRUE,
  );

  // Fixed amount.
  $form['amount']['amount_value'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed amount'),
    '#description' => t('The amount that will be charged. Stripe expects this value in cents, but you can enter a whole value here which will be multiplied by the multiplier below.'),
    '#default_value' => $component['extra']['amount_value'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'value'),
      ),
    ),
  );

  // Dynamic amount, which can be determined by a textfield, number, or select
  // component.
  $amount_components = array();
  foreach ($node->webform['components'] as $other_component) {
    if (in_array($other_component['type'], array('textfield', 'number', 'select'))) {
      if ($other_component['type'] === 'select' && $other_component['extra']['multiple']) {
        continue;
      }
      $amount_components[$other_component['cid']] = $other_component['name'];
    }
  }
  $form['amount']['amount_value_component'] = array(
    '#type' => 'select',
    '#title' => t('Dynamic amount'),
    '#description' => t("Sets the component which will determine the amount charged. Eligible components include: Textfield, Number, Select options (not multiple)."),
    '#options' => $amount_components,
    '#empty_option' => '- ' . t('Select a component') . ' -',
    '#default_value' => $component['extra']['amount_value_component'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'value_component'),
      ),
    ),
  );

  // Map select components to fixed amounts. The ajax handler is added in
  // webform_stripe_form_webform_component_edit_form_alter().
  $form['amount']['amount_value_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Amount options'),
    '#description' => t('The selected amount component has multiple options. Enter an amount for each choice.'),
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'value_component'),
      ),
    ),
  );

  // Amount multiplier.
  $form['amount']['amount_value_multiplier'] = array(
    '#type' => 'textfield',
    '#title' => t('Amount multiplier'),
    '#description' => t("Optionally multiply the amount by this value. This is useful if you don't want to deal with cents here, or require your users to enter values in cents on the form."),
    '#default_value' => $component['extra']['amount_value_multiplier'],
    '#size' => 10,
    '#states' => array(
      'visible' => array(
        array(':input[name="extra[amount_type]"]' => array('value' => 'value')),
        array(':input[name="extra[amount_type]"]' => array('value' => 'value_component')),
      ),
    ),
  );

  // Fixed plan.
  $form['amount']['amount_plan'] = array(
    '#type' => 'select',
    '#title' => t('Subscription plan'),
    '#description' => t('The Stripe subscription plan to which the user will be subscribed.'),
    '#options' => array(),
    '#empty_option' => '- ' . t('Select a plan') . ' -',
    '#default_value' => $component['extra']['amount_plan'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'plan'),
      ),
    ),
  );

  // Dynamic plan, which can be determined by a select component.
  $plan_components = array();
  foreach ($node->webform['components'] as $other_component) {
    if ($other_component['type'] === 'select' && !$other_component['extra']['multiple']) {
      $plan_components[$other_component['cid']] = $other_component['name'];
    }
  }
  $form['amount']['amount_plan_component'] = array(
    '#type' => 'select',
    '#title' => t('Dynamic subscription plan'),
    '#description' => t("Sets the component which will determine the subscription plan. Eligible components include: Select options (not multiple)."),
    '#options' => $plan_components,
    '#empty_option' => '- ' . t('Select a component') . ' -',
    '#default_value' => $component['extra']['amount_plan_component'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'plan_component'),
      ),
    ),
  );

  // Map select components to fixed plans. The ajax handler is added in
  // webform_stripe_form_webform_component_edit_form_alter().
  $form['amount']['amount_plan_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Plan options'),
    '#description' => t('Select the subscription plan for each option.'),
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="extra[amount_type]"]' => array('value' => 'plan_component'),
      ),
    ),
  );

  // Description type.
  $form['description'] = array(
    '#type' => 'fieldset',
    '#title' => t('Description'),
    '#collapsible' => TRUE,
    '#prefix' => '<div id="webform-stripe-description">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#parents' => array('extra'),
  );
  $form['description']['description_type'] = array(
    '#type' => 'radios',
    '#title' => t('Description type'),
    '#description' => t('Choose "fixed" to manually set the description, "dynamic" to determine it from another component on this form, or "subscription plan" to use the description of the selected plan.'),
    '#options' => array(
      'value' => t('Fixed description'),
      'value_component' => t('Dynamic description'),
      'plan' => t('Subscription plan description'),
    ),
    '#default_value' => $component['extra']['description_type'],
    '#required' => TRUE,
  );

  // Fixed description.
  $form['description']['description_value'] = array(
    '#type' => 'textfield',
    '#title' => t('Fixed description'),
    '#description' => t('A description of the product or service being purchased.'),
    '#default_value' => $component['extra']['description_value'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[description_type]"]' => array('value' => 'value'),
      ),
    ),
  );

  // Dynamic description, which can be determined by a textfield or select
  // component.
  $description_components = array();
  foreach ($node->webform['components'] as $other_component) {
    if (in_array($other_component['type'], array('textfield', 'select'))) {
      if ($other_component['type'] === 'select' && $other_component['extra']['multiple']) {
        continue;
      }
      $description_components[$other_component['cid']] = $other_component['name'];
    }
  }
  $form['description']['description_value_component'] = array(
    '#type' => 'select',
    '#title' => t('Dynamic description'),
    '#description' => t("Sets the component which will determine the description. Eligible components include: Textfield, Select options (not multiple)."),
    '#options' => $description_components,
    '#empty_option' => '- ' . t('Select a component') . ' -',
    '#default_value' => $component['extra']['description_value_component'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[description_type]"]' => array('value' => 'value_component'),
      ),
    ),
  );

  // Map select components to fixed descriptions. The ajax handler is added in
  // webform_stripe_form_webform_component_edit_form_alter().
  $form['description']['description_value_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('Description options'),
    '#description' => t('The selected description component has multiple options. Enter a description for each choice. If left blank, the selected option will be used as the description.'),
    '#tree' => TRUE,
    '#states' => array(
      'visible' => array(
        ':input[name="extra[description_type]"]' => array('value' => 'value_component'),
      ),
    ),
  );

  // Email type.
  $form['email'] = array(
    '#type' => 'fieldset',
    '#title' => t('Email'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#parents' => array('extra'),
  );
  $form['email']['email_type'] = array(
    '#type' => 'radios',
    '#title' => t('Email type'),
    '#options' => array(
      'user' => t('Use email address of logged-in user'),
      'component' => t('Dynamic email address'),
      'checkout' => t('Collect email address in Stripe Checkout form'),
    ),
    '#default_value' => $component['extra']['email_type'],
    '#required' => TRUE,
  );

  // Dynamic email, which can be determined by an email component.
  $email_components = array();
  foreach ($node->webform['components'] as $other_component) {
    if (in_array($other_component['type'], array('email'))) {
      $email_components[$other_component['cid']] = $other_component['name'];
    }
  }
  $form['email']['email_component'] = array(
    '#type' => 'select',
    '#title' => t('Dynamic email address'),
    '#description' => t("Sets the component which will determine the email address. Eligible components include: Email."),
    '#options' => $email_components,
    '#empty_option' => '- ' . t('Select a component') . ' -',
    '#default_value' => $component['extra']['email_component'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[email_type]"]' => array('value' => 'component'),
      ),
    ),
  );

  // Image type.
  $form['image'] = array(
    '#type' => 'fieldset',
    '#title' => t('Image'),
    '#collapsible' => TRUE,
    '#tree' => TRUE,
    '#parents' => array('extra'),
  );
  $form['image']['image_type'] = array(
    '#type' => 'radios',
    '#title' => t('Image type'),
    '#options' => array(
      'default' => t('Default image'),
      'file' => t('Custom image upload'),
      'url' => t('Custom image URL'),
      'none' => t('No image'),
    ),
    '#default_value' => $component['extra']['image_type'],
    '#required' => TRUE,
  );

  // Image upload.
  $form['image']['image_file'] = array(
    '#type' => 'managed_file',
    '#title' => t('Image upload'),
    '#description' => t('Select an png, jpg, or gif image that with a minimum size of 128x128px.'),
    '#default_value' => $component['extra']['image_file'],
    '#upload_validators' => array(
      'file_validate_image_resolution' => array(0, '128x128'),
      'file_validate_extensions' => array('png jpg gif'),
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="extra[image_type]"]' => array('value' => 'file'),
      ),
    ),
  );
  if (module_exists('media')) {
    $form['image']['image_file']['#type'] = 'media';
  }

  // Image URL.
  $form['image']['image_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Image URL'),
    '#description' => t('Enter an external URL or an internal path to a square image of your brand or product. It should be a png, jpg, or gif image with a minimum size of 128x128px.'),
    '#default_value' => $component['extra']['image_url'],
    '#states' => array(
      'visible' => array(
        ':input[name="extra[image_type]"]' => array('value' => 'url'),
      ),
    ),
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
  $form['display']['panel_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Payment button label'),
    '#description' => t('The label of the payment button in the payment form (e.g. "Pay {{amount}}", "Subscribe", etc.). If you include {{amount}}, it will be replaced by the provided amount. Otherwise, the amount will be appended to the end of your label.'),
    '#default_value' => $component['extra']['panel_label'],
    '#parents' => array('extra', 'panel_label'),
  );

  $form['#validate'][] = '_webform_edit_webform_stripe_validate';
  return $form;
}

/**
 * Alters component edit form, needed for ajax handling.
 */
function _webform_edit_webform_stripe_alter(&$form, &$form_state) {
  $component = $form_state['build_info']['args'][1];

  // Add ajax handler to amount component field.
  $form['amount']['amount_value_component']['#ajax'] = array(
    'callback' => '_webform_edit_webform_stripe_amount_ajax_callback',
    'wrapper' => 'webform-stripe-amount',
  );

  // Get the selected amount component. If it's a select field, add its
  // options to the mapping container and display the fieldset. Otherwise,
  // hide the mapping fieldset by converting it to a container.
  $form['amount']['amount_value_mapping']['#type'] = 'container';
  $cid = isset($form_state['values']['extra']['amount_value_component']) ? $form_state['values']['extra']['amount_value_component'] : $component['extra']['amount_value_component'];
  if (!empty($cid) && isset($form['#node']->webform['components'][$cid])) {
    $dynamic_component = $form['#node']->webform['components'][$cid];
    if ($dynamic_component['type'] === 'select') {
      $form['amount']['amount_value_mapping']['#type'] = 'fieldset';
      $options = _webform_select_options($dynamic_component);
      foreach ($options as $key => $value) {
        $form['amount']['amount_value_mapping'][$key] = array(
          '#type' => 'textfield',
          '#field_suffix' => check_plain($value),
          '#size' => 10,
          '#default_value' => isset($component['extra']['amount_value_mapping'][$key]) ? $component['extra']['amount_value_mapping'][$key] : '',
        );
      }
    }
  }

  // Add test mode ajax callback.
  $form['test_mode']['#ajax'] = array(
    'callback' => '_webform_edit_webform_stripe_amount_ajax_callback',
    'wrapper' => 'webform-stripe-amount',
  );

  // Determine if using test mode, and craft a message with an appropriate
  // link to the Stripe dashboard.
  $test_mode = isset($form_state['values']['extra']['test_mode']) ? $form_state['values']['extra']['test_mode'] : $component['extra']['test_mode'];
  $plan_message = t('You can <a href="!url">manage your !option subscription plans here</a>.', array(
    '!option' => $test_mode ? t('test') : t('live'),
    '!url' => $test_mode ? 'https://dashboard.stripe.com/test/plans' : 'https://dashboard.stripe.com/plans',
  ));

  // Set plan options and update description.
  $form['amount']['amount_plan']['#options'] = _webform_stripe_get_plans($test_mode);
  $form['amount']['amount_plan']['#description'] .= ' ' . $plan_message;

  // Add ajax handler to plan component field.
  $form['amount']['amount_plan_component']['#ajax'] = array(
    'callback' => '_webform_edit_webform_stripe_amount_ajax_callback',
    'wrapper' => 'webform-stripe-amount',
  );

  // Get the selected plan component and add its options to the mapping
  // fieldset. Otherwise, hide the mapping fieldset by converting it to a
  // container.
  $form['amount']['amount_plan_mapping']['#type'] = 'container';
  $cid = isset($form_state['values']['extra']['amount_plan_component']) ? $form_state['values']['extra']['amount_plan_component'] : $component['extra']['amount_plan_component'];
  if (!empty($cid) && isset($form['#node']->webform['components'][$cid])) {
    $form['amount']['amount_plan_mapping']['#type'] = 'fieldset';
    $form['amount']['amount_plan_mapping']['#description'] .= ' ' . $plan_message;
    $options = _webform_select_options($form['#node']->webform['components'][$cid]);
    foreach ($options as $key => $value) {
      $form['amount']['amount_plan_mapping'][$key] = array(
        '#type' => 'select',
        '#options' => _webform_stripe_get_plans($test_mode),
        '#empty_option' => '- ' . t('Select a plan') . ' -',
        '#field_suffix' => check_plain($value),
        '#default_value' => isset($component['extra']['amount_plan_mapping'][$key]) ? $component['extra']['amount_plan_mapping'][$key] : '',
      );
    }
  }

  // Add ajax handler to description component field.
  $form['description']['description_value_component']['#ajax'] = array(
    'callback' => '_webform_edit_webform_stripe_description_ajax_callback',
    'wrapper' => 'webform-stripe-description',
  );

  // Get the selected description component. If it's a select field, add its
  // options to the mapping container and display the fieldset. Otherwise,
  // hide the mapping fieldset by converting it to a container.
  $form['description']['description_value_mapping']['#type'] = 'container';
  $cid = isset($form_state['values']['extra']['description_value_component']) ? $form_state['values']['extra']['description_value_component'] : $component['extra']['description_value_component'];
  if (!empty($cid) && isset($form['#node']->webform['components'][$cid])) {
    $dynamic_component = $form['#node']->webform['components'][$cid];
    if ($dynamic_component['type'] === 'select') {
      $form['description']['description_value_mapping']['#type'] = 'fieldset';
      $options = _webform_select_options($dynamic_component);
      foreach ($options as $key => $value) {
        $form['description']['description_value_mapping'][$key] = array(
          '#type' => 'textfield',
          '#field_suffix' => check_plain($value),
          '#size' => 40,
          '#default_value' => isset($component['extra']['description_value_mapping'][$key]) ? $component['extra']['description_value_mapping'][$key] : $value,
        );
      }
    }
  }
}

/**
 * Amount ajax callback.
 */
function _webform_edit_webform_stripe_amount_ajax_callback($form, $form_state) {
  return $form['amount'];
}

/**
 * Description ajax callback.
 */
function _webform_edit_webform_stripe_description_ajax_callback($form, $form_state) {
  return $form['description'];
}

/**
 * Edit validate callback.
 */
function _webform_edit_webform_stripe_validate($form, &$form_state) {
  // Fixed amount.
  if ($form_state['values']['extra']['amount_type'] === 'value') {
    // Ensure positive amount.
    if (empty($form_state['values']['extra']['amount_value']) || $form_state['values']['extra']['amount_value'] <= 0) {
      form_set_error('extra][amount_value', t('Enter a positive amount (in cents).'));
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
    if (array_key_exists($settings['currency'], $minimum) && ($form_state['values']['extra']['amount_value'] * $form_state['values']['extra']['amount_value_multiplier']) < $minimum[$settings['currency']]) {
      form_set_error('extra][amount_value', t('The minimum amount for %currency is %minimum (cents).', array(
        '%currency' => $settings['currency'],
        '%minimum' => $minimum[$settings['currency']],
      )));
    }
  }

  // Component amount.
  if ($form_state['values']['extra']['amount_type'] === 'value_component') {
    // Component field is required.
    if (empty($form_state['values']['extra']['amount_value_component'])) {
      form_set_error('extra][amount_value_component', t('Select a component to use for the amount.'));
    }

    // All amount mappings must be set.
    if (!empty($form_state['values']['extra']['amount_value_mapping'])) {
      foreach ($form_state['values']['extra']['amount_value_mapping'] as $key => $value) {
        if (empty($value)) {
          form_set_error('extra][amount_value_mapping', t('Enter an amount for each component option.'));
          break;
        }
      }
    }
  }

  // Amount multiplier must be numeric.
  if (in_array($form_state['values']['extra']['amount_type'], array('value', 'value_component')) && !empty($form_state['values']['extra']['amount_value_multiplier']) && !is_numeric($form_state['values']['extra']['amount_value_multiplier'])) {
    form_set_error('extra][amount_value_multiplier', t('Enter a numeric amount multiplier.'));
  }

  // Plan is required if plan amount type is chosen.
  if ($form_state['values']['extra']['amount_type'] === 'plan' && empty($form_state['values']['extra']['amount_plan'])) {
    form_set_error('extra][amount_plan', t('Select a subscription plan.'));
  }

  // Component plan.
  if ($form_state['values']['extra']['amount_type'] === 'plan_component') {
    // Component field is required.
    if (empty($form_state['values']['extra']['amount_plan_component'])) {
      form_set_error('extra][amount_plan_component', t('Select a component to use for the subscription plan.'));
    }

    // All amount mappings must be set.
    if (!empty($form_state['values']['extra']['amount_plan_mapping'])) {
      foreach ($form_state['values']['extra']['amount_plan_mapping'] as $key => $value) {
        if (empty($value)) {
          form_set_error('extra][amount_plan_mapping', t('Select a subscription plan for each component option.'));
          break;
        }
      }
    }
  }

  // Component description.
  if ($form_state['values']['extra']['description_type'] === 'value_component' && empty($form_state['values']['extra']['description_value_component'])) {
    form_set_error('extra][description_value_component', t('Select a component to use for the description.'));
  }

  // Validate eligibility of subscription plan description.
  if ($form_state['values']['extra']['description_type'] === 'plan' && !in_array($form_state['values']['extra']['amount_type'], array('plan', 'plan_component'))) {
    form_set_error('extra][description_type', t('The description can only be taken from the subscription plan if the amount is also using a subscription plan.'));
  }

  // Email component is require is email type is component.
  if ($form_state['values']['extra']['email_type'] === 'component' && empty($form_state['values']['extra']['email_component'])) {
    form_set_error('extra][email_component', t('Select a component to use for the email address.'));
  }

  // Image file is require is image type is file.
  if ($form_state['values']['extra']['image_type'] === 'file' && empty($form_state['values']['extra']['image_file'])) {
    form_set_error('extra][image_file', t('Select a custom image.'));
  }

  // Image URL is require is image type is URL.
  if ($form_state['values']['extra']['image_type'] === 'url' && empty($form_state['values']['extra']['image_url'])) {
    form_set_error('extra][image_url', t('Enter a custom image URL.'));
  }
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_webform_stripe($component, $value = NULL, $filter = TRUE, $submission = NULL) {
  // If a payment is already saved, don't attach anything to the form, and
  // preserve the stored value.
  if (isset($value[0]) && !empty($value[0]) && substr($value[0], 0, 4) !== 'tok_') {
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
    '#attributes' => array('class' => array('webform-stripe-token')),
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
 * Alters a webform.
 */
function _webform_stripe_form_webform_client_form_alter(&$form, &$form_state) {
  // Look for a Stripe payment component on a form that has a Submit button.
  foreach ($form['#node']->webform['components'] as $cid => $component) {
    if ($component['type'] === 'webform_stripe' && isset($form['actions']['submit'])) {
      // Store component for easy access in our submit callback.
      $form['#webform_stripe'] = $component;

      // New submission.
      if (empty($form['#submission'])) {
        // Wrap form with ID so we can replace it via ajax.
        $form['#webform_stripe_html_id'] = drupal_html_id('webform-stripe-form');
        $form['#prefix'] = '<div id="' . $form['#webform_stripe_html_id'] . '">';
        $form['#suffix'] = '</div>';

        // Hide Submit button and add our Continue button.
        $continue_text = !empty($component['extra']['continue_text']) ? $component['extra']['continue_text'] : WEBFORM_STRIPE_CONTINUE_TEXT;
        if (!isset($form_state['triggering_element']) || !($form_state['triggering_element']['#name'] === 'op' && $form_state['triggering_element']['#value'] === $continue_text)) {
          $form['actions']['submit']['#access'] = FALSE;
          $form['actions']['webform_stripe_continue'] = array(
            '#type' => 'button',
            '#value' => $continue_text,
            '#weight' => 15,
            '#ajax' => array(
              'callback' => '_webform_stripe_form_webform_client_form_ajax_callback',
              'wrapper' => $form['#webform_stripe_html_id'],
            ),
          );
        }

        // Continue button was clicked.
        else {
          // Hide webform fields and submit button in a way that keeps them in
          // the HTML so they can be submitted after successful payment.
          $form['submitted']['#type'] = 'container';
          $form['submitted']['#attributes']['style'] = 'display: none';
          $form['actions']['submit']['#attributes']['style'] = 'display: none';

          // If Previous button is available, hide it.
          if (isset($form['actions']['previous'])) {
            $form['actions']['previous']['#access'] = FALSE;
          }

          // Add our Pay button, which will trigger Stripe Checkout.
          $form['actions']['webform_stripe_pay'] = array(
            '#type' => 'button',
            '#value' => !empty($component['extra']['pay_text']) ? $component['extra']['pay_text'] : WEBFORM_STRIPE_PAY_TEXT,
            '#attributes' => array('class' => array('webform-stripe-pay')),
            '#attached' => array(
              'js' => array(
                array(
                  'data' => array('webform_stripe' => _webform_stripe_process_checkout_settings($component, $form_state['values']['submitted'])),
                  'type' => 'setting',
                ),
              ),
            ),
          );

          // Add our Cancel button, which will return to editing the form.
          $form['actions']['webform_stripe_cancel'] = array(
            '#type' => 'button',
            '#value' => !empty($component['extra']['cancel_text']) ? $component['extra']['cancel_text'] : WEBFORM_STRIPE_CANCEL_TEXT,
            '#attributes' => array('class' => array('webform-stripe-cancel')),
            '#ajax' => array(
              'callback' => '_webform_stripe_form_webform_client_form_ajax_callback',
              'wrapper' => $form['#webform_stripe_html_id'],
            ),
          );
        }

        // Add submit callback.
        $form['#submit'][] = '_webform_stripe_process_token';
      }

      // Editing submission not already having a payment.
      else if (empty($form['#submission']->data[$cid][0])) {
        // Add our Pay button, which will trigger Stripe Checkout.
        $form['actions']['webform_stripe_pay'] = array(
          '#type' => 'button',
          '#value' => !empty($component['extra']['pay_text']) ? $component['extra']['pay_text'] : WEBFORM_STRIPE_PAY_TEXT,
          '#attributes' => array('class' => array('webform-stripe-pay')),
          '#weight' => 15,
          '#attached' => array(
            'js' => array(
              array(
                'data' => array('webform_stripe' => _webform_stripe_process_checkout_settings($component, $form['#submission']->data)),
                'type' => 'setting',
              ),
            ),
          ),
        );

        // Add submit callback.
        $form['#submit'][] = '_webform_stripe_process_token';
      }

      // Only process one Stripe Payment component per form.
      break;
    }
  }
}

/**
 * Webform ajax callback.
 */
function _webform_stripe_form_webform_client_form_ajax_callback($form, $form_state) {
  // Instead of simply returning $form, we also want to scroll to the top of
  // the form, so the ajax commands have to be built manually.
  // See ajax_prepare_response().
  return array(
    '#type' => 'ajax',
    '#commands' => array(
      ajax_command_insert(NULL, drupal_render($form)),
      ajax_command_prepend(NULL, theme('status_messages')),
      array(
        'command' => 'webformStripeScroll',
        'selector' => '#' . $form['#webform_stripe_html_id'],
        'duration' => 500,
      ),
    ),
  );
}

/**
 * Builds final Stripe Checkout component settings.
 */
function _webform_stripe_process_checkout_settings($component, $data) {
  // Load node.
  $node = node_load($component['nid']);

  // Merge component settings, publishable key, and default settings.
  $settings = array(
    'panel-label' => $component['extra']['panel_label'],
    'key' => $component['extra']['test_mode'] ? variable_get('webform_stripe_test_publishable_key', '') : variable_get('webform_stripe_live_publishable_key', ''),
  ) + webform_stripe_checkout_settings();

  // Determine amount or plan.
  switch ($component['extra']['amount_type']) {
    // Fixed amount.
    case 'value':
      $settings['amount'] = $component['extra']['amount_value'];
      if (!empty($component['extra']['amount_value_multiplier'])) {
        $settings['amount'] *= $component['extra']['amount_value_multiplier'];
      }
      break;

    // Dynamic amount.
    case 'value_component':
      $settings['amount'] = _webform_stripe_process_dynamic_value('amount_value', $component, $data);
      if (!empty($component['extra']['amount_value_multiplier'])) {
        $settings['amount'] *= $component['extra']['amount_value_multiplier'];
      }
      break;

    // Fixed plan.
    case 'plan':
      $settings['plan'] = $component['extra']['amount_plan'];
      break;

    // Dynamic plan.
    case 'plan_component':
      $settings['plan'] = _webform_stripe_process_dynamic_value('amount_plan', $component, $data);
      break;
  }

  // Determine description.
  switch ($component['extra']['description_type']) {
    // Fixed description.
    case 'value':
      $settings['description'] = $component['extra']['description_value'];
      break;

    // Dynamic description.
    case 'value_component':
      $settings['description'] = _webform_stripe_process_dynamic_value('description_value', $component, $data);
      break;

    // Plan description.
    case 'plan':
      $plans = _webform_stripe_get_plans($component['extra']['test_mode']);
      if (!empty($settings['plan']) && array_key_exists($settings['plan'], $plans)) {
        $settings['description'] = $plans[$settings['plan']];
      }
      break;
  }

  // Determine email address.
  switch ($component['extra']['email_type']) {
    // Logged in user.
    case 'user':
      $settings['email'] = user_uid_optional_load()->mail;
      break;

    // From component.
    case 'component':
      $settings['email'] = _webform_stripe_process_dynamic_value('email', $component, $data);
      break;
  }

  // Merge image default settings.
  if ($component['extra']['image_type'] === 'default') {
    $component['extra']['image_type'] = variable_get('webform_stripe_default_image_type', 'none');
    $component['extra']['image_file'] = variable_get('webform_stripe_default_image_file', '');
    $component['extra']['image_url'] = variable_get('webform_stripe_default_image_url', '');
  }

  // Determine image.
  switch ($component['extra']['image_type']) {
    // File.
    case 'file':
      $file = file_load($component['extra']['image_file']);
      if ($file) {
        $settings['image'] = image_style_url('webform_stripe', $file->uri);
      }
      break;

    // URL.
    case 'url':
      if (!empty($component['extra']['image_url'])) {
        $settings['image'] = url($component['extra']['image_url']);
      }
      break;

    // None.
    case 'none':
      $settings['image'] = NULL;
      break;
  }

  // Prepare for Javascript.
  foreach ($settings as $key => $setting) {
    // Convert boolean values into strings.
    if (in_array($key, array('billing-address', 'zip-code', 'shipping-address', 'allow-remember-me', 'bitcoin', 'alipay', 'alipay-reusable'))) {
      $setting = $setting ? 'true' : 'false';
    }

    // Skip if option is empty.
    if ($setting === '' || $setting === NULL) {
      unset($settings[$key]);
      continue;
    }

    $settings[$key] = $setting;
  }

  return $settings;
}

/**
 * Returns value based on another component's value.
 */
function _webform_stripe_process_dynamic_value($type, $component, $data) {
  $node = node_load($component['nid']);

  // Make sure dynamic component exists.
  if (!array_key_exists($component['extra']["{$type}_component"], $node->webform['components'])) {
    return NULL;
  }
  $dynamic_component = $node->webform['components'][$component['extra']["{$type}_component"]];

  // Find value in data, either from form_key or cid.
  $value = isset($data[$dynamic_component['form_key']]) ? $data[$dynamic_component['form_key']] : (isset($data[$dynamic_component['cid']]) ? $data[$dynamic_component['cid']] : NULL);

  // For selects, grab the key and find the mapped amount.
  if ($value !== NULL && $dynamic_component['type'] === 'select') {
    $key = _webform_submit_select($dynamic_component, $value);
    if (!is_array($value) && array_key_exists($key, $component['extra']["{$type}_mapping"])) {
      $value = $component['extra']["{$type}_mapping"][$key];

      // For descriptions, if the value is still empty, grab it from the select
      // option's value.
      if (empty($value) && $type === 'description_value') {
        $options = _webform_select_options($dynamic_component);
        $value = $options[$key];
      }
    }
    else {
      $value = NULL;
    }
  }

  return $value;
}

/**
 * Webform submit callback which processes a Stripe token.
 */
function _webform_stripe_process_token(&$form, &$form_state) {
  $component = $form['#webform_stripe'];
  $value = $form_state['values']['submitted'][$component['cid']];
  $sid = $form_state['values']['details']['sid'];

  // Nothing to process if we don't have a Stripe token.
  if (substr($value, 0, 4) !== 'tok_') {
    return;
  }

  // Process settings.
  $settings = _webform_stripe_process_checkout_settings($component, $form_state['values']['submitted']);

  // Parse Stripe token and email.
  list($token, $email) = explode(':', $value);

  // Include Stripe library.
  webform_stripe_load_library();

  // Set secret key.
  \Stripe\Stripe::setApiKey($component['extra']['test_mode'] ? variable_get('webform_stripe_test_secret_key', '') : variable_get('webform_stripe_live_secret_key', ''));

  try {
    // Create a subscription, and save the customer and subscription IDs.
    if (!empty($settings['plan'])) {
      $customer = \Stripe\Customer::create(array(
        'email' => $email,
      ));
      $subscription = $customer->subscriptions->create(array(
        'plan' => $settings['plan'],
        'source' => $token,
      ));
      $value = array(
        'test' => $component['extra']['test_mode'],
        'customer' => $customer->id,
        'subscription' => $subscription->id,
        'plan' => $settings['plan'],
      );
    }

    // Charge a one-time amount, and save the charge ID.
    else {
      $charge = \Stripe\Charge::create(array(
        'source' => $token,
        'amount' => $settings['amount'],
        'description' => !empty($settings['description']) ? $settings['description'] : '',
        'currency' => $settings['currency'],
        'receipt_email' => $email,
        'metadata' => array(
          'submission_url' => url("node/{$component['nid']}/submission/$sid", array('absolute' => TRUE)),
        ),
      ));
      $value = array(
        'test' => $component['extra']['test_mode'],
        'charge' => $charge->id,
      );
    }
  }

  // Transaction failed.
  catch(\Stripe\Error\Base $e) {
    drupal_set_message(t('The payment was unsuccessful.'), 'error');
    watchdog('webform_stripe', 'Payment was unsuccessful with message: %message', array(
      '%message' => $e->getMessage(),
    ), WATCHDOG_ERROR, l(t('view'), "node/{$component['nid']}/webform"));
  }

  // Update submission data.
  db_query("UPDATE {webform_submitted_data} SET data = :value WHERE nid = :nid AND sid = :sid AND cid = :cid", array(
    ':value' => serialize($value),
    ':nid' => $component['nid'],
    ':sid' => $sid,
    ':cid' => $component['cid'],
  ));
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_webform_stripe($component, $value, $format = 'html') {
  if (isset($value[0]) && !empty($value[0]) && substr($value[0], 0, 4) !== 'tok_') {
    $data = unserialize($value[0]);
    $link = user_access('administer webform stripe') && $format === 'html';
    return array(
      '#type' => 'markup',
      '#title' => $component['name'],
      '#markup' => isset($data['charge'])
        ? ($link ? l(t('paid'), 'https://dashboard.stripe.com/' . ($data['test'] ? 'test/' : '') . 'payments/' . $data['charge']) : t('paid'))
        : ($link ? l(t('subscribed'), 'https://dashboard.stripe.com/' . ($data['test'] ? 'test/' : '') . 'customers/' . $data['customer']) : t('subscribed')),
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
  if (isset($value[0]) && !empty($value[0]) && substr($value[0], 0, 4) !== 'tok_') {
    $data = unserialize($value[0]);
    return isset($data['charge']) ? t('paid') : t('subscribed');
  }
  return t('no');
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
  return _webform_table_webform_stripe($component, $value);
  if (isset($value[0]) && !empty($value[0]) && substr($value[0], 0, 4) !== 'tok_') {
    return t('yes');
  }
  return t('no');
}
