#!/bin/sh
for i in $1/*
do
	a=`echo $i| awk -F "_" '{ print $5}'`
	cat $i |grep -B 2 "/_objs" |sed -e "s/<item>//"|sed -e "s/<\/item>//"|tr '\r\n\-\-' ' '|tr '<\/_objs>' '\r\n'|sed "/^$/d"|sed "/^ $/d"|sed "s/^/$a/" |sort
done	
