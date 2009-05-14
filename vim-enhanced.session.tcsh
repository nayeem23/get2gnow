#!/bin/tcsh -f
set my_editor = "`printf "\""${0}"\"" | sed 's/.*\/\([^\.]\+\).*/\1/g'`"
switch ( "${my_editor}" )
case "connectED":
case "gedit":
	breaksw
case "vi":
case "vim":
case "vim-enhanced":
default:
	set my_editor = `printf "%s -p" "vim-enhanced"`
	breaksw
endsw

${my_editor} "./TODO" "./src/online-services.c" "./src/tweets.c" "./src/users.c" "./src/network.c" "./src/popup-dialog.c" "./src/tweet-view.c" "./src/profile-viewer.c" "./src/friends-manager.c"
