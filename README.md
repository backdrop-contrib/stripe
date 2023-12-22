# Stripe

This module provides a simple abstraction to use the Stripe PHP SDK. This
module is designed to be required by other contrib/custom modules.

## Features

- Features from the `webform_stripe` module are included in the 2.x version.

## Installation

- Once the dependencies are in place, install this module using the [official
  Backdrop CMS instructions](https://backdropcms.org/guide/modules).
- Configure the module at *Configuration > Web Services > Stripe*
  (`admin/config/services/stripe`), adding your Stripe API keys and
  setting preferences.

## Usage

The main way to make Stripe API calls is using the `stripe_api_call` function.

```php
/**
 * Makes a call to the Stripe API.
 *
 * @param string $obj
 *   Stripe object. Can be a Charge, Refund, Customer, Subscription, Card, Plan,
 *   Coupon, Discount, Invoice, InvoiceItem, Dispute, Transfer, TransferReversal,
 *   Recipient, BankAccount, ApplicationFee, FeeRefund, Account, Balance, Event,
 *   Token, BitcoinReceiver, FileUpload.
 *
 * @param string $method
 *   Stripe object method. Common operations include retrieve, all, create,
 *
 * @param mixed $params
 *   Additional params to pass to the method. Can be an array, string.
 *
 * @return Stripe\ApiResource
 *   Returns the ApiResource or NULL on error.
 */
stripe_api_call($obj, $method = NULL, $params = NULL) { ... }

// Examples.

// Get a customer object.
$customer = stripe_api_call('customer', 'retrieve', 'cus_id123123123');
$customer->email = NEW_EMAIL;
$customer->save();

// Create a customer object.
$customer = stripe_api_call('customer', 'create', array(
  'email' => $user->mail,
));

// Delete a customer.
$customer = stripe_api_call('customer', 'retrieve', 'cus_id123123123');
$customer->delete();

// List customers.
$list = stripe_api_call('customer', 'all', array('limit' => 5));
```

This module provides a secure Stripe webhook (events are validated) and provides
two hooks for you to implement in your custom module. See stripe.api.php for API
documentation.

This is an *unofficial* Stripe module and is not associated with Stripe.

## Issues

Bugs and Feature requests should be reported in this module's
[Issue Queue](https://github.com/backdrop-contrib/stripe_api/issues).

## Current Maintainers

- [Laryn Kragt Bakker](https://github.com/laryn)
- [Jen Lampton](https://github.com/jenlampton)

## Credits

 - This module was ported to Backdrop by
   [Laryn Kragt Bakker](https://github.com/laryn)
 - The Backdrop port was initially sponsored by [CEDC.org](https://cedc.org)
 - This module was originally created and is maintained for Drupal by
   [Dan Pepin](https://github.com/donutdan4114) and
   [Bonify, LLC](http://bonify.io)

## Stripe Bindings

- Note: [Stripe's PHP bindings](https://github.com/stripe/stripe-php) have been
  packaged and incuded in this module. (Version 13.7.0). The Stripe bindings
  are released under the MIT license.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
