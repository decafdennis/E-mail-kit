<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Implementation of hook_dispatcher_send() for the emailkit_default dispatcher.
 */
function emailkit_default_dispatcher_send($dispatcher, $message, $destination) {
  // Create the text part
  $text_part = array(
    'headers' => array(
      'Content-Type' => 'text/plain; charset=utf-8; format=flowed; delsp=yes',
      'Content-Transfer-Encoding' => 'quoted-printable',
    ),
    'body' => _emailkit_mime_quoted_printable(emailkit_message_render($message, EMAILKIT_MESSAGE_FORMAT_TEXT)),
  );
  
  // Create the HTML part
  $html_part = array(
    'headers' => array(
      'Content-Type' => 'text/html; charset=utf-8',
      'Content-Transfer-Encoding' => 'quoted-printable',
    ),
    'body' => _emailkit_mime_quoted_printable(emailkit_message_render($message, EMAILKIT_MESSAGE_FORMAT_HTML)),
  );
  
  // Create the multipart containing the alternatives
  $alternative_boundary = _emailkit_mime_content_boundary();
  $alternative_multipart = _emailkit_mime_multipart_alternative($alternative_boundary, array($text_part, $html_part));

  // Create an array suitable for drupal_mail_send()
  $mail = array(
    'from' => variable_get('site_mail', ini_get('sendmail_from')),
    'subject' => $message['#subject'],
    'headers' => array(
      'MIME-Version' => '1.0',
      'Content-Type' => 'multipart/alternative; boundary="' . $alternative_boundary . '"',
      'X-Mailer' => 'E-mail kit (Drupal)',
    ),
    'body' => $alternative_multipart,
  );
  
  // Process the destination
  $to_array = array();
  foreach ($destination['#recipients'] as $recipient) {
    $to_array[] = $recipient['address'];
  }
  $mail['to'] = implode(", ", $to_array);

  return drupal_mail_send($mail);
}

/**
 * Returns a unique string that can be used as a content boundary in MIME mesages.
 */
function _emailkit_mime_content_boundary() {
  return 'emailkit=_' . md5(uniqid(mt_rand(), TRUE));
}

/**
 * Returns multipart/alternative content using the given boundary and consisting of the given parts.
 *
 * @param $boundary A string used as a content boundary.
 * @param $parts An array of parts, which by itself are arrays containing headers and a body, similar to what drupal_mail_send() accepts.
 */
function _emailkit_mime_multipart_alternative($boundary, $parts) {
  $output = '';
  
  foreach ($parts as $part) {
    // Add the boundary
    $output .= "--$boundary\r\n";
    
    // Add the content header
    if (isset($part['headers'])) {
      foreach ($part['headers'] as $key => $value) {
        $output .= $key . ": " . mime_header_encode($value) . "\r\n";
      }
    }
    $output .= "\r\n";
    
    // Add the content itself
    if (isset($part['body'])) {
      $output .= $part['body'] . "\r\n";
    }
  }
  
  // Add the final boundary
  if (!empty($parts)) {
    $output .= "--$boundary--\r\n";
  }
  
  return $output;
}

/**
 * Encodes the given string using quoted-printable encoding.
 *
 * @param $input The input string, which is expected to have LF line endings.
 */
function _emailkit_mime_quoted_printable($input) {
  $output = '';
  $output_line_length = 0;
  
  // Just make absolutely sure we don't have any CR line endings around
  $input = str_replace("\r", '', $input);
  $input_length = strlen($input);
  
  // Iterate each character of the input string
  for ($i = 0; $i < $input_length; $i++) {
    // Get the character and its ASCII value
    $c = $input[$i];
    $ascii = ord($c);
    
    // Encode line endings
    if ($ascii == 0x0A) {
      $c = "\r\n";
      $output_line_length = -strlen($c);
    }
    // Encode spaces at the end of a line
    else if ($ascii == 0x20 && $i + 1 < $input_length && $input[$i + 1] == "\n") {
      $c = "=20";
    }
    // Encode equal signs and non-printable ASCII characters
    else if ($ascii == 0x3D || $ascii < 0x20 || $ascii > 0x7E) {
      $c = sprintf("=%02X", $ascii);
    }
    
    // Add a soft line break, if necessary
    if ($output_line_length + strlen($c) >= 76) {
      $output .= "=\r\n";
      $output_line_length = 0;
    }
    
    $output .= $c;
    $output_line_length += strlen($c);
  }
  
  return $output;
}
