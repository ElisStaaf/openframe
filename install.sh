#!/bin/sh

# Keep in mind that this is *very*
# shaky, and might just destroy your
# system. Be careful out there!

if [ $1 == "-Fs" ] && [ $# == 2 ]; then
    INCLUDE="$2"
else
    INCLUDE="/usr/include"
fi

sudo cd .. && \
sudo cp $(cd -) $(basename $(cd -))
