<?php
// Developed by Dennis Stevense for Digital Deployment

/**
 * Inlines the CSS in the given HTML document and returns the resulting HTML document.
 *
 * This function uses the emailkit_premailer.rb script, which in turn uses the premailer gem.
 */
function emailkit_premailer_inline_css($input) {
  $output = $input;
  
  // Determine the command to run
  $script_path = drupal_get_path('module', 'emailkit_premailer') . '/emailkit_premailer.rb';
  $command = 'ruby ' . escapeshellarg($script_path);
  
  // Determine the file descriptors to set up
  $descriptors = array(
    0 => array('pipe', 'r'),
    1 => array('pipe', 'w'),
    2 => array('pipe', 'w'),
  );

  // Run the command
  $process = proc_open($command, $descriptors, $pipes);

  // Did we successfully start the process?
  if (is_resource($process)) {
    // Write the message content to the standard input of the process
    fwrite($pipes[0], $output);
    fclose($pipes[0]);
    
    // Read the result from the standard output of the process
    $process_output = stream_get_contents($pipes[1]);
    fclose($pipes[1]);
    
    // Read the standard error of the process
    $process_error = stream_get_contents($pipes[2]);
    fclose($pipes[2]);
    
    // Stop the process
    $return = proc_close($process);

    // Only use the result if the return code is 0 (i.e. no errors occured)
    if ($return == 0) {
      $output = $process_output;
    }
    else {
      trigger_error($process_error, E_USER_WARNING);
    }
  }
  else {
    trigger_error("Could not run emailkit_premailer.rb", E_USER_WARNING);
  }
  
  return $output;
}
