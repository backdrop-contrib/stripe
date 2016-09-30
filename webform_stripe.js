(function($) {

  /**
   * Overrides Ajax beforeSubmit callback to make request synchronous, which
   * preserves the click event after the ajax response is received. This
   * prevents popup blockers from "breaking" our checkout experience.
   * @see https://www.drupal.org/node/2661330
   * @see https://www.drupal.org/node/2782783
   */
  var beforeSubmit = Drupal.ajax.prototype.beforeSubmit;
  Drupal.ajax.prototype.beforeSubmit = function(form_values, element, options) {
    if (element.hasClass('webform-stripe-form')) {
      options.async = false;
    }
    beforeSubmit();
  };

  /**
   * Ajax command to open Stripe Checkout, store token and email in hidden
   * token field, and submit form.
   */
  Drupal.ajax.prototype.commands.webformStripeCheckout = function(ajax, data, status) {
    StripeCheckout.open($.extend(data.params, {
      token: function(token) {
        $('.webform-stripe-token', ajax.form.context).val(token.id + ':' + token.email);
        ajax.form[0].submit();
      }
    }));
  };

}(jQuery));
