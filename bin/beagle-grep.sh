#!/bin/bash
echo -n beagle query argument \`'[01;31m'$(get_longest_token "$@")'[0m'\' 1>&2
echo 1>&2
my-beagle "$@"|tee /tmp/$$.beagel|xargs grep -H -n -I "$@" 

#http://thomasbhj.spaces.live.com/blog/cns!FC1463FF7BEF1F15!243.entry
cat /tmp/$$.beagel|grep -I "$@"|perl -npe 's/$/:1: [01;31m****************![0m/'
rm /tmp/$$.beagel
