#!/bin/bash
# Shell script to run each user's customized version of smallwm
if [ -x "/etc/user.d/$USER.sh" ]; then
	/etc/user.d/$USER.sh
fi

/usr/local/bin/smallwm
