(function($) {

  /**
   * Opens Stripe Checkout when Webform Stripe 'pay' button is clicked.
   */
  Drupal.behaviors.webformStripe = {
    attach: function(context, settings) {
      $('.webform-stripe-pay', context).click(function() {
        StripeCheckout.open($.extend(settings.webform_stripe, {
          // Payment was successful.
          token: function(token) {
            // Set token and email in token field.
            $('.webform-stripe-token', context).val(token.id + ':' + token.email);

            // Submit form.
            $('.webform-submit', context).click();
          }
        }));
        return false;
      });
    }
  };

}(jQuery));
