#PATHPROG=/usr/local/bin/ganesha.nfsd 
PATHPROG=MainNFSD/ganesha.nfsd

LOGFILE=/var/log/ganesha.log
CONFFILE=/etc/ganesha/ganesha.conf

prog=ganesha.nfsd
PID_FILE=${PID_FILE:=/var/run/${prog}.pid}
LOCK_FILE=${LOCK_FILE:=/var/lock/subsys/${prog}}

#OPTIONS="-d -L $LOGFILE -f $CONFFILE -N NIV_EVENT -p $PID_FILE "
OPTIONS="-L $LOGFILE -f $CONFFILE -N NIV_FULL_DEBUG "

# $PATHPROG -f /etc/ganesha/ganesha.conf -N NIV_EVENT -L /var/log/ganesha.log
$PATHPROG -L /var/log/proxy.ganesha.log -f /etc/ganesha/proxy.ganesha.conf -N NIV_DEBUG
