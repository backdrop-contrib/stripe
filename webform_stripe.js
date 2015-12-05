(function($) {

  // Opens Strip Checkout for enabled webforms.
  Drupal.behaviors.webformStripe = {
    attach: function(context, settings) {
      // Get our DOM elements.
      var $tokenField = $('.webform-stripe-token', context);
      var $form = $tokenField.closest('form');
      var $trigger = $form.find(Drupal.settings.webform_stripe.checkout_trigger);

      // Clicking the trigger.
      $trigger.click(function(event) {
        // Prevent the default action.
        event.preventDefault();

        // Disable trigger.
        $trigger.attr('disabled', 'disabled');

        // Open Stripe checkout.
        StripeCheckout.open($.extend(Drupal.settings.webform_stripe.checkout_params, {
          // Success!
          token: function(token) {
            // Set token in token field.
            $tokenField.val(token.id);

            // If trigger is the submit button, submit the form.
            if ($trigger.is('.webform-submit')) {
              $form.submit();
            }

            // If trigger is the "make payment" link, change the text to help
            // user know they should submit the form to complete payment.
            else if ($trigger.is('.webform-stripe-make-payment')) {
              $trigger.text(Drupal.t('Save to complete payment'));
            }
          },

          // After closing Stripe form, unless we have a successful Stripe
          // token, enable the trigger.
          closed: function() {
            if (!$tokenField.val()) {
              $trigger.removeAttr('disabled');
            }
          }
        }));
      });

    }
  };

}(jQuery));
