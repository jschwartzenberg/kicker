#!/bin/sh
#
#  DEFAULT KDE STARTUP SCRIPT ( @KDE_VERSION_STRING@ )
#

# When the X server dies we get a HUP signal from xinit. We must ignore it
# because we still need to do some cleanup.
trap 'echo GOT SIGHUP' HUP

# Check if a KDE session already is running
if kcheckrunning >/dev/null 2>&1; then
	echo "KDE seems to be already running on this display."
	xmessage -geometry 500x100 "KDE seems to be already running on this display." > /dev/null 2>/dev/null
	exit 1
fi

# Set the background to plain grey.
# The standard X background is nasty, causing moire effects and exploding
# people's heads. We use colours from the standard KDE palette for those with
# palettised displays.
if test -z "$XDM_MANAGED" || echo "$XDM_MANAGED" | grep ",auto" > /dev/null; then
  xsetroot -solid "#000000"
fi

# we have to unset this for Darwin since it will screw up KDE's dynamic-loading
unset DYLD_FORCE_FLAT_NAMESPACE

# Enable lightweight memory corruption checker
MALLOC_CHECK_=2 
export MALLOC_CHECK_

# in case we have been started with full pathname spec without being in PATH
bindir=`echo "$0" | sed -n 's,^\(/.*\)/[^/][^/]*$,\1,p'`
if [ -n "$bindir" ]; then
  case $PATH in
    $bindir|$bindir:*|*:$bindir|*:$bindir:*) ;;
    *) PATH=$bindir:$PATH; export PATH;;
  esac
fi

# Boot sequence:
#
# kdeinit is used to fork off processes which improves memory usage
# and startup time.
#
# * kdeinit starts klauncher first.
# * Then kded is started. kded is responsible for keeping the sycoca
#   database up to date. When an up to date database is present it goes
#   into the background and the startup continues.
# * Then kdeinit starts kcminit. kcminit performs initialisation of
#   certain devices according to the user's settings
#
# * Then ksmserver is started which takes control of the rest of the startup sequence

# The user's personal KDE directory is usually ~/.kde, but this setting
# may be overridden by setting KDEHOME.

kdehome=$HOME/.kde
test -n "$KDEHOME" && kdehome=`echo "$KDEHOME"|sed "s,^~/,$HOME/,"`

# see kstartupconfig source for usage
mkdir -m 700 -p $kdehome
mkdir -m 700 -p $kdehome/share
mkdir -m 700 -p $kdehome/share/config
cat >$kdehome/share/config/startupconfigkeys <<EOF
kcminputrc Mouse cursorTheme ''
kcminputrc Mouse cursorSize ''
ksplashrc KSplash Theme Default
ksplashrc KSplash Engine KSplashX
kcmrandrrc Display ApplyOnStartup false
kcmrandrrc [Screen0]
kcmrandrrc [Screen1]
kcmrandrrc [Screen2]
kcmrandrrc [Screen3]
kcmfonts General forceFontDPI 0
EOF
kstartupconfig
if test $? -ne 0; then
    xmessage -geometry 500x100 "Could not start kstartupconfig. Check your installation."
fi
. $kdehome/share/config/startupconfig

# XCursor mouse theme needs to be applied here to work even for kded or ksmserver
if test -n "$kcminputrc_mouse_cursortheme" -o -n "$kcminputrc_mouse_cursorsize" ; then
    kapplymousetheme "$kcminputrc_mouse_cursortheme" "$kcminputrc_mouse_cursorsize"
    if test $? -eq 10; then
        export XCURSOR_THEME=default
    elif test -n "$kcminputrc_mouse_cursortheme"; then
        export XCURSOR_THEME="$kcminputrc_mouse_cursortheme"
    fi
    if test -n "$kcminputrc_mouse_cursorsize"; then
        export XCURSOR_SIZE="$kcminputrc_mouse_cursorsize"
    fi
fi

if test "$kcmrandrrc_display_applyonstartup" = "true"; then
    # 4 screens is hopefully enough
    for scrn in 0 1 2 3; do
        args=
        width="\$kcmrandrrc_screen${scrn}_width" ; eval "width=$width"
        height="\$kcmrandrrc_screen${scrn}_height" ; eval "height=$height"
        if test -n "${width}" -a -n "${height}"; then
            args="$args -s ${width}x${height}"
        fi
        refresh="\$kcmrandrrc_screen${scrn}_refresh" ; eval "refresh=$refresh"
        if test -n "${refresh}"; then
            args="$args -r ${refresh}"
        fi
        rotation="\$kcmrandrrc_screen${scrn}_rotation" ; eval "rotation=$rotation"
        if test -n "${rotation}"; then
            case "${rotation}" in
                0)
                    args="$args -o 0"
                    ;;
                90)
                    args="$args -o 1"
                    ;;
                180)
                    args="$args -o 2"
                    ;;
                270)
                    args="$args -o 3"
                    ;;
            esac
        fi
        reflectx="\$kcmrandrrc_screen${scrn}_reflectx" ; eval "reflectx=$reflectx"
        if test "${refrectx}" = "true"; then
            args="$args -x"
        fi
        reflecty="\$kcmrandrrc_screen${scrn}_reflecty" ; eval "reflecty=$reflecty"
        if test "${refrecty}" = "true"; then
            args="$args -y"
        fi
        if test -n "$args"; then
            xrandr $args
        fi
    done
fi

if test "$kcmfonts_general_forcefontdpi" -eq 120; then
    xrdb -quiet -merge -nocpp <<EOF
Xft.dpi: 120
EOF
elif test "$kcmfonts_general_forcefontdpi" -eq 96; then
    xrdb -quiet -merge -nocpp <<EOF
Xft.dpi: 96
EOF
fi


dl=$DESKTOP_LOCKED
unset DESKTOP_LOCKED # Don't want it in the environment

if test -z "$dl"; then
  # the splashscreen and progress indicator
  case "$ksplashrc_ksplash_engine" in
    KSplashX)
      ksplashx ${ksplashrc_ksplash_theme}
      ;;
    None)
      ;;
    Simple)
      ksplashsimple
      ;;
    *)
      ;;
  esac
fi

# Source scripts found in <localprefix>/env/*.sh and <prefixes>/env/*.sh
# (where <localprefix> is $KDEHOME or ~/.kde, and <prefixes> is where KDE is installed)
#
# This is where you can define environment variables that will be available to
# all KDE programs, so this is where you can run agents using e.g. eval `ssh-agent`
# or eval `gpg-agent --daemon`.
# Note: if you do that, you should also put "ssh-agent -k" as a shutdown script
#
# (see end of this file).
# For anything else (that doesn't set env vars, or that needs a window manager),
# better use the Autostart folder.

exepath=`kde4-config --path exe | tr : '\n'`

for prefix in `echo "$exepath" | sed -n -e 's,/bin[^/]*/,/env/,p'`; do
  for file in "$prefix"*.sh; do
    test -r "$file" && . "$file"
  done
done

# Activate the kde font directories.
#
# There are 4 directories that may be used for supplying fonts for KDE.
#
# There are two system directories. These belong to the administrator.
# There are two user directories, where the user may add her own fonts.
#
# The 'override' versions are for fonts that should come first in the list,
# i.e. if you have a font in your 'override' directory, it will be used in
# preference to any other.
#
# The preference order looks like this:
# user override, system override, X, user, system
#
# Where X is the original font database that was set up before this script
# runs.

usr_odir=$HOME/.fonts/kde-override
usr_fdir=$HOME/.fonts

if test -n "$KDEDIRS"; then
  kdedirs_first=`echo "$KDEDIRS"|sed -e 's/:.*//'`
  sys_odir=$kdedirs_first/share/fonts/override
  sys_fdir=$kdedirs_first/share/fonts
else
  sys_odir=$KDEDIR/share/fonts/override
  sys_fdir=$KDEDIR/share/fonts
fi

# We run mkfontdir on the user's font dirs (if we have permission) to pick
# up any new fonts they may have installed. If mkfontdir fails, we still
# add the user's dirs to the font path, as they might simply have been made
# read-only by the administrator, for whatever reason.

test -d "$sys_odir" && xset +fp "$sys_odir"
test -d "$usr_odir" && (mkfontdir "$usr_odir" ; xset +fp "$usr_odir")
test -d "$usr_fdir" && (mkfontdir "$usr_fdir" ; xset fp+ "$usr_fdir")
test -d "$sys_fdir" && xset fp+ "$sys_fdir"

# Ask X11 to rebuild its font list.
xset fp rehash

# Set a left cursor instead of the standard X11 "X" cursor, since I've heard
# from some users that they're confused and don't know what to do. This is
# especially necessary on slow machines, where starting KDE takes one or two
# minutes until anything appears on the screen.
#
# If the user has overwritten fonts, the cursor font may be different now
# so don't move this up.
#
xsetroot -cursor_name left_ptr

# Get Ghostscript to look into user's KDE fonts dir for additional Fontmap
if test -n "$GS_LIB" ; then
    GS_LIB=$usr_fdir:$GS_LIB
    export GS_LIB
else
    GS_LIB=$usr_fdir
    export GS_LIB
fi

lnusertemp=`kde4-config --path exe --locate lnusertemp`
if test -z "$lnusertemp"; then
  # Startup error
  echo 'startkde: ERROR: Could not locate lnusertemp in '`kde4-config --path exe` 1>&2
fi

# Link "tmp" "socket" and "cache" resources to directory in /tmp
# Creates:
# - a directory /tmp/kde-$USER and links $KDEHOME/tmp-$HOSTNAME to it.
# - a directory /tmp/ksocket-$USER and links $KDEHOME/socket-$HOSTNAME to it.
# - a directory /var/tmp/kdecache-$USER and links $KDEHOME/cache-$HOSTNAME to it.
# Note: temporary locations can be overriden through the KDETMP and KDEVARTMP
# environment variables
for resource in tmp cache socket; do
    if ! "$lnusertemp" $resource >/dev/null; then
        echo 'startkde: Call to lnusertemp failed (temporary directories full?). Check your installation.'  1>&2
        xmessage -geometry 600x100 "Call to lnusertemp failed (temporary directories full?). Check your installation."
        exit 1
    fi
done

# In case of dcop sockets left by a previous session, cleanup
#dcopserver_shutdown

echo 'startkde: Starting up...'  1>&2

# Make sure that D-Bus is running, running qdbus will auto-launch it
# if it is not
if qdbus >/dev/null 2>/dev/null; then
    : # ok
else
    echo 'startkde: Could not start D-Bus. Check your installation.'  1>&2
    xmessage -geometry 500x100 "Could not start D-Bus. Check your installation."
fi


# Mark that full KDE session is running (e.g. Konqueror preloading works only
# with full KDE running). The KDE_FULL_SESSION property can be detected by
# any X client connected to the same X session, even if not launched
# directly from the KDE session but e.g. using "ssh -X", kdesu. $KDE_FULL_SESSION
# however guarantees that the application is launched in the same environment
# like the KDE session and that e.g. KDE utilities/libraries are available.
# KDE_FULL_SESSION property is also only available since KDE 3.5.5.
# The matching tests are:
#   For $KDE_FULL_SESSION:
#     if test -n "$KDE_FULL_SESSION"; then ... whatever
#   For KDE_FULL_SESSION property:
#     xprop -root | grep "^KDE_FULL_SESSION" >/dev/null 2>/dev/null
#     if test $? -eq 0; then ... whatever
#
# Additionally there is (since KDE 3.5.7) $KDE_SESSION_UID with the uid
# of the user running the KDE session. It should be rarely needed (e.g.
# after sudo to prevent desktop-wide functionality in the new user's kded).
#
KDE_FULL_SESSION=true
export KDE_FULL_SESSION
xprop -root -f KDE_FULL_SESSION 8t -set KDE_FULL_SESSION true

KDE_SESSION_VERSION=4
export KDE_SESSION_VERSION
xprop -root -f KDE_SESSION_VERSION 32c -set KDE_SESSION_VERSION 4

KDE_SESSION_UID=$UID
export KDE_SESSION_UID

# We set LD_BIND_NOW to increase the efficiency of kdeinit.
# kdeinit unsets this variable before loading applications.
LD_BIND_NOW=true kdeinit4 +kcminit_startup
if test $? -ne 0; then
  # Startup error
  echo 'startkde: Could not start kdeinit4. Check your installation.'  1>&2
  xmessage -geometry 500x100 "Could not start kdeinit4. Check your installation."
fi

# If the session should be locked from the start (locked autologin),
# lock now and do the rest of the KDE startup underneath the locker.
if test -n "$dl"; then
  kwrapper4 krunner_lock --forcelock &
  # Give it some time for starting up. This is somewhat unclean; some
  # notification would be better.
  sleep 1
fi

# finally, give the session control to the session manager
# see kdebase/ksmserver for the description of the rest of the startup sequence
# if the KDEWM environment variable has been set, then it will be used as KDE's
# window manager instead of kwin.
# if KDEWM is not set, ksmserver will ensure kwin is started.
# kwrapper4 is used to reduce startup time and memory usage
# kwrapper4 does not return useful error codes such as the exit code of ksmserver.
# We only check for 255 which means that the ksmserver process could not be 
# started, any problems thereafter, e.g. ksmserver failing to initialize, 
# will remain undetected.
test -n "$KDEWM" && KDEWM="--windowmanager $KDEWM"
kwrapper4 ksmserver $KDEWM 
if test $? -eq 255; then
  # Startup error
  echo 'startkde: Could not start ksmserver. Check your installation.'  1>&2
  xmessage -geometry 500x100 "Could not start ksmserver. Check your installation."
fi

# wait if there's any crashhandler shown
while qdbus | grep -q ^[^w]*org.kde.drkonqi ; do
    sleep 5
done

echo 'startkde: Shutting down...'  1>&2

# Clean up
kdeinit4_shutdown

echo 'startkde: Running shutdown scripts...'  1>&2

# Run scripts found in $KDEDIRS/shutdown
for prefix in `echo "$exepath" | sed -n -e 's,/bin[^/]*/,/shutdown/,p'`; do
  for file in `ls "$prefix" 2> /dev/null | egrep -v '(~|\.bak)$'`; do
    test -x "$prefix$file" && "$prefix$file"
  done
done

unset KDE_FULL_SESSION
xprop -root -remove KDE_FULL_SESSION
unset KDE_SESSION_VERSION
xprop -root -remove KDE_SESSION_VERSION
unset KDE_SESSION_UID

echo 'startkde: Done.'  1>&2
