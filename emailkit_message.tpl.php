<?php 
// Developed by Dennis Stevense for Digital Deployment

/**
 * @file This template contains the default markup of a message sent by emailkit.
 */

?><html>
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  </head>
  <body>
    <?php print $message['#children'] ?>
  </body>
</html>
