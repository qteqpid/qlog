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

pid=$(cat /etc/qlog/qlog.pid)
echo "kill process $pid"
kill -2 $pid
rm -f /etc/qlog/qlog.pid &> /dev/null


