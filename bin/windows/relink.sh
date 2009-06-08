#!/bin/bash
src="`/bin/readlink -f \"$1\"`"
if [[ -z "$src" ]]; then
    src="`/bin/readlink \"$1\"`"
fi

if [[ $INPLACE == true ]]; then
    tgt="$1"
else
    tgt=~/bin/windows/lnks/"`basename \"$1\"`"
fi

function re_match()
{
    if echo "$1" | grep "$2" >/dev/null; then
        return 0;
    else 
        return 1
    fi
}

function relink_func()
{
    if ! [[ -e "$src" ]]; then
        if re_match "$src" "^/./"; then
            for x in {a..z}; do 
                new_src=/cygdrive/$x/"${src:3}"
                if [[ -e "$new_src" ]]; then
                    echo "$1" source found at "$new_src"
                    rm "$tgt"
                    ln -sf "$new_src" "$tgt"
                    return
                fi
            done
            echo "$1" is not valid
        elif re_match "$src" "^/cygdrive/./"; then
            for x in {a..z}; do
                new_src=/cygdrive/$x/"${src:12}"
                if [[ -e "$new_src" ]]; then
                    echo "$1" source found at "$new_src"
                    rm "$tgt"
                    ln -sf "$new_src" "$tgt"
                    return
                fi
            done
            echo "$1" is not valid
        fi
    else
        echo "$1" source found at "$src"
        rm "$tgt"
        ln -sf "$src" "$tgt"
    fi
}
relink_func "$1"
