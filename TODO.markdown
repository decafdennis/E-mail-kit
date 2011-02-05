Next
====

* Less primitive drupal_mail_wrapper()
* CSS style inlining
* CAN-SPAM compliance
* Statistics module that can be enabled per message type
	* Sent (how many, when)
	* Opened (how many, when, hook)

Icebox
===================

* Keep track of subscriber opt in/out history
* E-mail formatting
	* Template system
	* Hook system
	* Placeholder/token system
	* Add unsubscribe link
* E-mail list management
	* Bounce detection
	* Campaign statistics
		* Sent
		* Delivered
		* Opened (per recipient, total)
		* Clicks (per recipient, total)
		* Unsubscribes
	* Allow subscribers to change their e-mail address
* E-mail list subscription permissions
* List and subscriber hooks to support for member management modules
	* Manage lists and subscribers
	* Manage list access
	* Provide subscriber info (like first name, last name) for replacement in emails
* Advanced subscriber management (or do we pass this stick to some member management module?)
	* Import/export
	* Filtering
	* Mass edit/remove
