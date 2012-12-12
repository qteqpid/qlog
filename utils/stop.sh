#!/bin/bash - 
#===============================================================================
#
#          FILE: stop.sh
# 
#         USAGE: ./stop.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (), 
#  ORGANIZATION: 
#       CREATED: 2012年12月07日 14时32分29秒 CST
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error
pidfile=/etc/qlog/qlog.pid

function killserver()
{
    pid=$(cat $pidfile)
    echo "kill process $pid"
    kill -2 $pid
    rm -f /etc/qlog/qlog.pid &> /dev/null

}
[[ -e $pidfile ]] && killserver


