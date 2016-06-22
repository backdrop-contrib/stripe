#Stripe API Backdrop module

This module provides a simple abstraction to use the Stripe PHP SDK. This 
module is designed to be required by other contrib/custom modules.

##Dependencies

- [Libraries](https://github.com/backdrop-contrib/libraries)
- [Stripe's PHP bindings](https://github.com/stripe/stripe-php) should be 
  copied into the `/libraries` directory in a folder named `stripe`.

##Installation

- Once the dependencies are in place, install this module using the [official 
  Backdrop CMS instructions](https://backdropcms.org/guide/modules).
- Configure the module at *Configuration > Web Services > Stripe API Settings*
  (`admin/config/services/stripe_api`), adding your Stripe API keys and 
  setting preferences.
  
##Usage

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
two hooks for you to implement in your custom module.

```php
/**
 * Interact with all incoming Stripe webhooks.
 *
 * @param string $type
 *   Webhook type, such as customer.created, charge.captured, etc.
 *
 * @param object $data
 *   Incoming data object.
 *
 * @param Stripe\Event $event
 *   The verified Stripe Event that is being sent.
 *   Only available in live mode with real events (not test events).
 *
 * @throws \Exception
 */
function hook_stripe_api_webhook($type, $data, Stripe\Event $event = NULL) { ... }

/**
 * Interact with a specific incoming webhook type.
 *
 * @param object $data
 *   Incoming data object.
 *
 * @param Stripe\Event $event
 *   The verified Stripe Event that is being sent.
 *   Only available in live mode with real events (not test events).
 *
 * @throws \Exception
 */
function hook_stripe_api_webhook_EVENT_TYPE($data, Stripe\Event $event = NULL) { ... }
```

This is an *unofficial* module and is no way associated with Stripe.

##Issues

Bugs and Feature requests should be reported in this module's 
[Issue Queue](https://github.com/backdrop-contrib/stripe_api/issues).

##License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

##Current Maintainers

- [Laryn Kragt Bakker](https://github.com/laryn) - [CEDC.org](https://cedc.org)

##Credits

 - This module was ported to Backdrop by 
   [Laryn Kragt Bakker](https://github.com/laryn) -
   [CEDC.org](https://cedc.org).
 - This module was originally created and is maintained for Drupal by 
   [Dan Pepin](https://github.com/donutdan4114) and 
   [Bonify, LLC](http://bonify.io)
