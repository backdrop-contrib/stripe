(function($) {

  /**
   * Ajax command to add Stripe Checkout buttons to a Webform.
   */
  Drupal.ajax.prototype.commands.webformStripeCheckout = function(ajax, data, status) {
    // Wrap and hide form.
    var $form = $(ajax.form[0]);
    $wrapper = $('<div class="webform-stripe-wrapper"></div>').hide();
    $form.wrapInner($wrapper);

    // Append action buttons.
    $form.append(data.actions);

    // When pay button is clicked, open Stripe Checkout.
    var $payButton = $('.webform-stripe-pay-button', $form);
    $payButton.click(function() {
      StripeCheckout.open($.extend(data.params, {
        // Payment was successful.
        token: function(token) {
          // Set token and email in token field.
          $(data.selector, ajax.form.context).val(token.id + ':' + token.email);

          // Submit form.
          ajax.form[0].submit();
        }
      }));
      return false;
    });

    // When edit button is clicked, remove action buttons and unwrap form.
    $('.webform-stripe-edit-button', $form).click(function() {
      $('.webform-stripe-actions').remove();
      $form.children().children().unwrap();
      return false;
    });
  };

}(jQuery));
