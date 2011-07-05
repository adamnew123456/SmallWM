#!/bin/bash
#
# Builds a Slackware compatable package to either install or
# convert

VERSION="0.2"
ARCH="i686"

if [ -e "working" ]; then
	rm -r working
fi

mkdir working
mkdir working/usr
mkdir working/usr/share
mkdir working/usr/local

CDIR="working/usr/local"
mkdir $CDIR/bin
mkdir $CDIR/doc
mkdir $CDIR/src

./build.sh clean
# Cannot deploy a failed build
./build.sh smallwm || exit

./install.sh $CDIR

mkdir working/install

: > working/install/slack-desc
echo "       |-----handy-ruler--------------------------------------|" | tee -a working/install/slack-desc
for line in "SmallWM (A Hacked for Readability version of TinyWM)" \
			"" \
	        "SmallWM is a refactored hack of TinyWM that has new " \
	        "features such as window bordering, Xterm-at-a-click, " \
	        "window layering, and focus-on-click." \
	        "" \
		  	"Despite all these improvements, the binary remains" \
			"under 50K, under 1000 LOC, and rock solid." \
			"" \
			"" \
            "Packaged and written by Adam Marchetti"; do
	echo "smallwm: $line" | tee -a working/install/slack-desc
done

mkdir working/usr/share/xsessions
: > working/usr/share/xsessions/smallwm.desktop
for line in "[Desktop Entry]" "Name=SmallWM" "Icon=" \
	    "Exec=/usr/local/bin/smallwm.sh" "Type=XSession"; do
	echo $line >> working/usr/share/xsessions/smallwm.desktop
done

# Feel free to change this to sudo, if it makes you feel better.
cd working
su -c "makepkg --linkadd y --chown y  ../smallwm-$VERSION-$ARCH-1.tgz &> /dev/null"

cd ..
su -c "rm -r working"
