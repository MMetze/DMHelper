#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export VLC_PLUGIN_PATH="${HERE}/usr/lib/vlc/plugins"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/DMHelper" "$@"
