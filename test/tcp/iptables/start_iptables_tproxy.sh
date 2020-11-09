#!/usr/bin/env bash

#使用nat表做数据转发
iptables -t mangle -N DIVERT #在nat表上新建名为DIVERT自定义链
iptables -t mangle -A PREROUTING -p tcp -m socket --transparent -j DIVERT #已建立的socket且被tproxy标记过的数据包执行DIVERT
iptables -t mangle -A DIVERT -j MARK --set-xmark 0x10000000/0xf0000000 #进入DIVERT设置标记
iptables -t mangle -A DIVERT -j ACCEPT
iptables -t mangle -N MY_TCP
iptables -t mangle -p tcp -A MY_TCP -j TPROXY --on-port 8081 --tproxy-mark 0x10000000/0xf0000000
#MY_TCP执行TPROXY转发为8081端口并进行标记
iptables -t mangle -A MY_TCP -j ACCEPT
iptables -t mangle -N MYMANGLE
iptables -t mangle -A PREROUTING -p tcp -j MYMANGLE #MYMANGLE链加入到PREROUTING
iptables -t mangle -A MYMANGLE -p tcp -m multiport --dports 80 -j MY_TCP #80端口的包执行MY_TCP
ip rule add fwmark 0x10000000/0xf0000000 table 200 pref 200 #对标记过的数据包执行序号为200的规则
ip route add local default dev lo table 200 #200规则：数据包发送到本地回环