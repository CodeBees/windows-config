#!/bin/bash
set -e 
cd /q/gcode/scim-cs/scim
seq=$1
python q:/bin/windows/terminateModule.py ywbhj"$seq" || true
setime.pl $seq > 1.reg
regedit /s 1.reg
rm -f 1.reg

shift
test "$#" = 0 && vccompile '' '' || "$@" 
cp ../Release/scim.dll ~/.sys/ywbhj$seq.dll -v


