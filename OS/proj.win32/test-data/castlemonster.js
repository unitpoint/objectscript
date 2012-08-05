/*
* jCryption JavaScript data encryption v1.2
* http://www.jcryption.org/
*
* Copyright (c) 2010 Daniel Griesser
* Dual licensed under the MIT and GPL licenses.
* http://www.opensource.org/licenses/mit-license.php
* http://www.opensource.org/licenses/gpl-2.0.php
*
* If you need any further information about this plugin please
* visit my homepage or contact me under daniel.griesser@jcryption.org
*/
(function($) {
	$.jCryption = function(el, options) {
		var base = this;

		base.$el = $(el);
		base.el = el;

		base.$el.data("jCryption", base);
		base.init = function() {

			base.options = $.extend({},$.jCryption.defaultOptions, options);

			$encryptedElement = $("<input />",{
				type:'hidden',
				name:base.options.postVariable
			});

				var $submitElement = base.options.submitElement;

			$submitElement.bind(base.options.submitEvent,function() {
				$(this).attr("disabled",true);
					$.jCryption.getKeys(base.options.getKeysURL, function(keys) {
						$.jCryption.encrypt(base.$el.serialize(), keys,function(encrypted) {
							$encryptedElement.val(encrypted);
							$(base.$el).find(base.options.formFieldSelector).attr("disabled",true).end().append($encryptedElement).submit();
						});
					});
				return false;
			});

		};
		base.init();
	};

})();
