(function($) {

  /**
   * Ajax command to open Stripe Checkout, store result, and submit form.
   */
  Drupal.ajax.prototype.commands.stripeCheckout = function(ajax, data, status) {
    // Open Stripe Checkout.
    StripeCheckout.open($.extend(data.params, {
      // Payment was successful.
      token: function(token) {
        // Set token and email in token field.
        $(data.selector, ajax.form.context).val(token.id + ':' + token.email);

        // Submit form.
        ajax.form[0].submit();
      }
    }));
  };

}(jQuery));
