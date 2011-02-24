<?php 
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This template contains the default markup of a message sent by emailkit.
 *
 * @param $message The structured message array. The message (or pieces of it) can be rendered using drupal_render(). (This is inspired by Drupal 7, where it would be rendered using the render() and hide() functions.)
 */

?><html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  </head>
  <body>
    <div id="header">
      <?php print drupal_render($message['header']) ?>
    </div>
    <div id="content">
      <?php print drupal_render($message['content']) ?>
    </div>
    <div id="footer">
      <?php print drupal_render($message['footer']) ?>
    </div>
    <?php
      // This will render out any remaining elements that have not yet been rendered by the above calls
      print drupal_render($message);
    ?>
  </body>
</html>
