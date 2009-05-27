#!/bin/bash


cd "$HOMEDRIVE/$HOMEPATH"
#cygrunsrv.exe -I startup -p /c/WINDOWS/system32/subst.exe -a 'q: "c:\documents and settings\bhj"'

HOME2="`pwd`"
ln -sf "$HOME2" /qq
#psexec -s subst q: 'c:\Documents and Settings\bhj'
subst q: "$HOMEDRIVE"\\"$HOMEPATH"
export HOME=/q



cd ~/doc
regedit /s ImeNoToggle.reg
. ~/.bashrc-windows
cd ~/Local\ Settings/Temp && rm tramp* -rf

cygpath -alwm `which bash` > /c/.bash-loc
export CYGDIR=`cygpath -alwm /`


cd ~/bin/windows/startup


for x in *; do 
    cygstart "$x"
done

ln -sf "$HOME2"/bin/windows/startup.sh ~/start\ menu/programs/startup
