#!/bin/bash
#
# Builds a Slackware compatable package to either install or
# convert

VERSION="0.1.2"

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
for line in "SmallWM - A Hacked for Readability version of TinyWM" \
	    "" "SmallWM is a refactored TinyWM with a couple extra" \
	    "features such as window bordering, Xterm-at-a-click, " \
	    "and a couple extra keyboard shortcuts" \
	    "" "" "" "" "" "Packaged by Adam Marchetti"; do
	echo "smallwm: $line" | tee -a working/install/slack-desc
done

mkdir working/usr/share/xsessions
: > working/usr/share/xsessions/smallwm.desktop
for line in "[Desktop Entry]" "Name=SmallWM" "Icon=" \
	    "Exec=/usr/local/bin/smallwm.sh" "Type=XSession"; do
	echo $line >> working/usr/share/xsessions/smallwm.desktop
done

cd working
su -c "makepkg --linkadd y --chown y  ../smallwm-$VERSION.tgz &> /dev/null"
