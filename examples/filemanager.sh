#!/bin/ksh

while true
do
	FILENAME=$(ls -a | xlistsel -i -b Open -b Delete -b Exit -t $PWD)
	case $? in
		0 | 3) break ;; # User closes dialog
		# Either chdir into the selected dir or edit the file
		1) [ -d $FILENAME ] && cd $FILENAME || xedit "$FILENAME" ;;
		2) echo rm "$FILENAME" ;; # well... we better avoid this here ;-)
	esac
done

