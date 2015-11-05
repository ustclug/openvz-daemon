#!/bin/bash
#usage vzcreate $CTID $OSTEMPLATE $HOSTNAME
set -e
if [ -z $1 ]; then
    echo "no CTID"
    exit 1
else
    CTID=$1
fi

if [ -z $2 ]; then
    echo "no ostemplate"
    exit 1
else
    OSTEMPLATE=$2
fi

if [ -z $3 ]; then
    HOSTNAME="vz-${CTID}"
else
    HOSTNAME=$3
fi

POOL=rbd
SIZE=10240
CONF=freeshell
if rbd status shell-${CTID} 2&>/dev/null; then
    echo shell existed ... quit
    exit 1
else
    rbd create -p ${POOL} --size ${SIZE} shell-${CTID}
    rbd map shell-${CTID}
    mkfs.ext4 /dev/rbd/$POOL/shell-${CTID} 
    ./vzmount $CTID
    ROOTPASSWD=$(openssl rand -base64 12)
    echo ${ROOTPASSWD}
    vzctl create ${CTID} --ostemplate $OSTEMPLATE --config ${CONF} --private /mnt/freeshell/${CTID}/${CTID}
    vzctl set ${CTID} --ipadd 10.70.$((${CTID}/256)).$((${CTID}%256)) --save
    vzctl set ${CTID} --nameserver 202.38.64.1 --save
    vzctl set ${CTID} --hostname $HOSTNAME --save
    vzctl set ${CTID} --userpasswd root:${ROOTPASSWD}
    ./vzumount ${CTID}
fi
exit 0

