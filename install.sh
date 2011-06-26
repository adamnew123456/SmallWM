#!/bin/bash
#
# Installs smallwm in $1
BASE="/usr/local"
TEST="no"

if [ ! "$1" = "" ]; then
	BASE="$1"
	TEST="yes"
fi

noop(){
	echo -n ""
}

if [ ! -e "smallwm" ]; then
	./build.sh release
fi

find -name "*.sh" | xargs chmod +x

cp smallwm "$BASE/bin"
cp smallwm.sh "$BASE/bin"

mkdir "$BASE/src/smallwm-0.1" &> /dev/null || noop
cp client.c client.h event.c event.h smallwm.c smallwm.h global.h smallwm.sh build.sh install.sh "$BASE/src/smallwm-0.1"

mkdir "$BASE/doc/smallwm-0.1" &> /dev/null || noop
cp README "$BASE/doc/smallwm-0.1"

# Ignore slackpkg.sh installs
if [ "$TEST" = "no" ]; then
echo "
[Desktop Entry]
Name=SmallWM
Icon=
Exec=$BASE/bin/smallwm.sh
Type=XSession
" > /usr/share/xsessions/smallwm.desktop
fi
