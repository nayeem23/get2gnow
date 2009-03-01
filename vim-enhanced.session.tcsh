#!/bin/tcsh -f
set my_editor = "`printf "\""${0}"\"" | sed 's/.*\/\([^\.]\+\).*/\1/g'`"
switch ( "${my_editor}" )
case "gedit":
	breaksw
case "vim":
default:
	set my_editor = `printf "%s -p" "vim-enhanced"`
	breaksw
endsw

${my_editor} "./src/twitux-parser.c" "./src/twitux-send-message-dialog.c"