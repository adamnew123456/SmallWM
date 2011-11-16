#!/bin/bash
# Shell script to run each user's customized version of smallwm
if [ -x "$HOME/.smallwm.sh" ]; then
	$HOME/.smallwm.sh
fi

/usr/local/bin/smallwm &> /tmp/smallwm.log
