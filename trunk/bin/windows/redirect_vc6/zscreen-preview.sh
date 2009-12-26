#!/bin/bash
mkdir -p ~/images/
mv "$1" ~/images/
cygpath -au ~/images/"$(basename "$1")"|tr -d '\n' > ~/.zscreen.history
cygstart ~/images/"$(basename "$1")"
echo -n ../images/"$(basename "$1")"|putclip
findexec -p gimp-2.6
