#!/bin/bash
#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 2000 - 2022, EdelWeb for EdelKey and OpenEvidence
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# SPDX-License-Identifier: curl
#
###########################################################################

OPENSSL=openssl
if [ -f /usr/local/ssl/bin/openssl ] ; then
OPENSSL=/usr/local/ssl/bin/openssl
fi

USAGE="echo Usage is genroot.sh \<name\>"

HOME=`pwd`
cd $HOME

KEYSIZE=2048
DURATION=6000
# The -sha256 option was introduced in OpenSSL 1.0.1
DIGESTALGO=-sha256

PREFIX=$1
if [ ".$PREFIX" = . ] ; then
   echo No configuration prefix
   NOTOK=1
else
   if [ ! -f $PREFIX-ca.prm ] ; then
      echo No configuration file $PREFIX-ca.prm
      NOTOK=1
   fi
fi

if [ ".$NOTOK" != . ] ; then
   echo "Sorry, I can't do that for you."
   $USAGE
   exit
fi

GETSERIAL="\$t = time ;\$d =  \$t . substr(\$t+$$ ,-4,4)-1;print \$d"
SERIAL=`/usr/bin/env perl -e "$GETSERIAL"`

# exit on first fail
set -e

echo SERIAL=$SERIAL PREFIX=$PREFIX DURATION=$DURATION KEYSIZE=$KEYSIZE

echo "openssl genrsa -out $PREFIX-ca.key -passout XXX $KEYSIZE"
openssl genrsa -out $PREFIX-ca.key -passout pass:secret $KEYSIZE

echo "openssl req -config $PREFIX-ca.prm -new -key $PREFIX-ca.key -out $PREFIX-ca.csr"
$OPENSSL req -config $PREFIX-ca.prm -new -key $PREFIX-ca.key -out $PREFIX-ca.csr -passin pass:secret

echo "openssl x509 -set_serial $SERIAL -extfile $PREFIX-ca.prm -days $DURATION -req -signkey $PREFIX-ca.key -in $PREFIX-ca.csr -out $PREFIX-$SERIAL.ca-cacert $DIGESTALGO "

$OPENSSL x509  -set_serial $SERIAL -extfile $PREFIX-ca.prm -days $DURATION -req -signkey $PREFIX-ca.key -in $PREFIX-ca.csr -out $PREFIX-$SERIAL-ca.cacert $DIGESTALGO

echo "openssl x509 -text -in $PREFIX-$SERIAL-ca.cacert -nameopt multiline > $PREFIX-ca.cacert "
$OPENSSL x509 -text -in $PREFIX-$SERIAL-ca.cacert -nameopt multiline > $PREFIX-ca.cacert

echo "openssl x509 -in $PREFIX-ca.cacert -outform der -out $PREFIX-ca.der "
$OPENSSL x509 -in $PREFIX-ca.cacert -outform der -out $PREFIX-ca.der

echo "openssl x509 -in $PREFIX-ca.cacert -text -nameopt multiline > $PREFIX-ca.crt "

$OPENSSL x509 -in $PREFIX-ca.cacert -text -nameopt multiline > $PREFIX-ca.crt

echo "openssl x509 -noout -text -in $PREFIX-ca.cacert -nameopt multiline"
$OPENSSL x509 -noout -text -in $PREFIX-ca.cacert -nameopt multiline

#$OPENSSL rsa -in ../keys/$PREFIX-ca.key -text -noout -pubout
