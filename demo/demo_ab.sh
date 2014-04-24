#!/bin/bash
PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:~/bin
export PATH

ab_cmd="ab -n 10000 -c 100 http://192.168.10.30/demokafka.php"  

maturity=$(date +%s -d '2014-04-23 17:08:15')
# echo "End time:::" $maturity
numb=0
rps_sum=0


# Concurrency Level:      10
# Time taken for tests:   0.205 seconds
# Complete requests:      10
# Failed requests:        0
# Total transferred:      3110 bytes
# Requests per second:    48.71 [#/sec] (mean)
# Time per request:       205.313 [ms] (mean)
# Time per request:       20.531 [ms] (mean, across all concurrent requests)
# Transfer rate:          14.79 [Kbytes/sec] received

ABNUM=0
CR=0
FR=0
RPS=0
TPRA=0
TPRB=0
TR=0
MAXRPS=0
MAXRPS_info=0
MINRPS=99999999
MINRPS_info=0
MAXTR=0
MAXTR_info=0
MINTR=99999999
MINTR_info=0
while [ "1" = "1" ]
do
	ABNUM=`expr $ABNUM + 1`	
	begintime=`date "+%Y-%m-%d %H:%M:%S"`
	echo "[begin time]:::" $begintime >>aaaa.log
	
	echo "[run number]:::" $numb >> aaaa.log
	result=`$ab_cmd | grep 'Concurrency Level:
	\|Time taken for tests:
	\|Complete requests:
	\|Failed requests:
	\|Total transferred:
	\|Requests per second:
	\|Time per request:
	\|Time per request:
	\|Transfer rate:' > bbbb.log`  
	cat bbbb.log >> aaaa.log
	
	TFILE="./bbbb.log"

	declare -i cr=`cat $TFILE | grep 'Complete requests:' | awk -F ' ' '{ print $3 }'`
	CR=`expr $CR + $cr`
	
	declare -i fr=`cat $TFILE | grep 'Failed requests:' | awk -F ' ' '{ print $3 }'`
	FR=`expr $FR + $fr`
	
	declare -i rps=`cat $TFILE | grep 'Requests per second:' | awk -F ' ' '{ print $4 }' | awk -F '.' '{ print $1 }'`
	RPS=`expr $RPS + $rps`
	if [ $rps -gt $MAXRPS ]; then  
		MAXRPS=$rps
        MAXRPS_info=`cat $TFILE | grep 'Requests per second:' | awk -F ' ' '{ print $4 }'`
    fi 
    if [ $rps -lt $MINRPS ]; then  
    	MINRPS=$rps
        MINRPS_info=`cat $TFILE | grep 'Requests per second:' | awk -F ' ' '{ print $4 }'`
    fi   

	tpr=`cat $TFILE | grep 'Time per request:' | awk -F ' ' '{ print $4 }'`
	
	declare -i tpra=`echo $tpr |awk -F ' ' '{ print $1 }' | awk -F '.' '{ print $1 }'`
	declare -i tprb=`echo $tpr |awk -F ' ' '{ print $2 }' | awk -F '.' '{ print $1 }'`
	TPRA=`expr $TPRA + $tpra`
	TPRB=`expr $TPRB + $tprb`
	
	declare -i tr=`cat $TFILE | grep 'Transfer rate:' | awk -F ' ' '{ print $3 }' | awk -F '.' '{ print $1 }'`
	TR=`expr $TR + $tr`
	if [ $tr -gt $MAXTR ]; then  
		MAXTR=$tr
        MAXTR_info=`cat $TFILE | grep 'Transfer rate:' | awk -F ' ' '{ print $3 }'`
    fi 
    if [ $tr -lt $MINTR ]; then  
    	MINTR=$tr
        MINTR_info=`cat $TFILE | grep 'Transfer rate:' | awk -F ' ' '{ print $3 }'`
    fi   

	# cat $TFILE | while read line 
	# do
	# 	if [ $k -eq 2 ];then
	# 		declare -i cr=`echo $line | grep 'Complete requests:' | awk -F ' ' '{ print $3 }'`
	# 		echo "cr::"$cr
	# 		echo "CR::"$CR
	# 		CRS=`expr $CR + $cr`
	# 		echo $CRS>ccc.log
	# 		# export complete_requests=$complete_requests
	# 		# echo $complete_requests
	# 	fi

	# 	# ttft=`echo $line | grep 'Time taken for tests:' | awk -F ' ' '{ print $5 }'`
	# 	# echo $ttft
	# 	# r_p_s=`echo $line | grep 'Requests per second:' | awk -F ' ' '{ print $4 }'`
	# 	# rps_sum=`expr $rps_sum + $r_p_s`
	# 	k=`expr $k + 1`
	# done<$TFILE

	# echo "fff"$k
	# echo $CR

 	time2=$(($(date +%s) - $maturity));
	if [ $time2 -ge 1 ];then	
		stoptime=`date "+%Y-%m-%d %H:%M:%S"`	
		echo  >> cccc.log
		echo "[Begin time]:::" $begintime "=========================">> cccc.log		
		echo "[run Total]:::" $numb >> cccc.log
		echo "[Complete requests]:::" $CR >> cccc.log
		echo "[Failed requests]:::" $FR >> cccc.log
		echo "[Requests per second]:::" $RPS >> cccc.log
		echo "[Time per request]:::" $TPRA >> cccc.log
		echo "[Time per request]:::" $TPRB >> cccc.log
		echo "[Transfer rate]:::" $TR >> cccc.log
		echo "[AB num]:::" $ABNUM >> cccc.log
		echo "[end time]:::" $stoptime"=============================">> cccc.log
		echo "======================================================">> cccc.log
		echo "======================================================">> cccc.log		
		echo "[max rps]:::" $MAXRPS_info " [#/sec] (mean)">> cccc.log
		echo "[min rps]:::" $MINRPS_info " [#/sec] (mean)">> cccc.log

		echo "[max Transfer rate]:::" $MAXTR_info " [Kbytes/sec] received">> cccc.log
		echo "[min Transfer rate]:::" $MINTR_info " [Kbytes/sec] received">> cccc.log

		echo "======================================================">> cccc.log
		echo "======================================================">> cccc.log
		echo "[avg rps]:::[total rps] / [ab num] =" `expr $RPS / $ABNUM` >> cccc.log
		break;
	fi
done

