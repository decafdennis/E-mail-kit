require 'rubygems'
require 'premailer'

# Read the original HTML from the standard input
premailer = Premailer.new(STDIN)

# Write the HTML with inlined CSS to the standard output
STDOUT.puts premailer.to_inline_css
