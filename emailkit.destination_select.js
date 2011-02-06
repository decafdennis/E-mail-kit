// Developed by Dennis Stevense for Digital Deployment

Drupal.behaviors.emailkitDestinationSelect = function(context) {
  // Find all destination select elements
  $('dl.emailkit-destination-select-items', context).each(function() {
    new Drupal.emailkitDestinationSelect($(this));
  });
};

/**
 * Initializes a destination select field handler for the given element.
 *
 * @param items A jQuery object containing a definition list.
 */
Drupal.emailkitDestinationSelect = function(items) {
  var self = this;
  this.items = items;
  
  // Wrap the content of definitions in divs without margins and paddings to make the sliding animation look better
  $('dd', items).wrapInner('<div class="emailkit-animation" />');
  
  // Find all terms, which contain the radio fields
  $('dt', items).each(function() {
    var itemTerm = $(this);
    
    var itemClassName = self.itemClassName(itemTerm);
    if (itemClassName != null) {
      // When the radio field is clicked, update the destination select field
      $('input.form-radio', itemTerm).click(function() {
        self.updateItems(itemClassName, true);
      });
    }
  });

  // Find the currenty selected term and update the destination select field accordingly
  var selectedItemClassName = null;
  $('dt:has(input.form-radio:checked)', items).each(function() {
    var itemTerm = $(this);

    var itemClassName = self.itemClassName(itemTerm);
    if (itemClassName != null) {
      selectedItemClassName = itemClassName;;
    }
  });
  self.updateItems(selectedItemClassName, false);
};

/**
 * Returns the e-mail kit class name of the given item.
 *
 * @param item A jQuery object. 
 *
 * @return The class name, or null if it has no e-mail kit class name.
 */
Drupal.emailkitDestinationSelect.prototype.itemClassName = function(item) {
  var className = null;
  
  $.each(item.attr('class').split(/\s/), function(index, candidateClassName) {
    if (candidateClassName.match(/^emailkit-destination-select-item-/)) {
      className = candidateClassName;
      return;
    }
  });
  
  return className;
}

/**
 * Updates the destination select field.
 *
 * @param selectedItemClassName The class name of the selected item.
 * @param animated Whether to animate the changes. (Not implemented yet.)
 */
Drupal.emailkitDestinationSelect.prototype.updateItems = function(selectedItemClassName, animated) {
  var self = this;
  
  $('dd', self.items).each(function() {
    var itemDefinition = $(this);
    var itemDefinitionAnimation = $('.emailkit-animation', itemDefinition);

    if (selectedItemClassName != null && itemDefinition.hasClass(selectedItemClassName)) {
      itemDefinitionAnimation.css('opacity', 1);
      
      if (animated) {
        itemDefinitionAnimation.animate({ height: 'show' }, 'fast');
      }
      else {
        itemDefinitionAnimation.show();
      }
    }
    else {
      if (animated) {
        itemDefinitionAnimation.animate({ opacity: 0, height: 'hide' }, 'fast');
      }
      else {
        itemDefinitionAnimation.hide();
        itemDefinitionAnimation.css('opacity', 0);
      }
    }
  });
}
