#!/bin/bash
File=$(cygpath -au "${1:-$(getclip)}")
cygpath -au "$(readlink -f "$File")"|tr -d '\r\n'|xargs -0 cygpath -alw|tr -d '\r\n'|putclip
getclip