#!/usr/bin/env bash

#使用nat表做数据转发
iptables -t nat -N PROXY_INIT_REDIREC #在nat表上新建名为PROXY_INIT_REDIREC自定义链
iptables -t nat -p tcp -A PROXY_INIT_REDIREC -j REDIRECT --to-ports 5001 #将进入PROXY_INIT_REDIREC链的数据包端口重定向到8081上
iptables -t nat -N PROXY_INIT_OUTPUT #在nat表上新建名为PROXY_INIT_OUTPUT自定义链
iptables -t nat -A PREROUTING -p tcp -j PROXY_INIT_OUTPUT #将PROXY_INIT_OUTPUT加入到PREROUTING链后
iptables -t nat -A OUTPUT -p tcp -j PROXY_INIT_OUTPUT #将PROXY_INIT_OUTPUT加入到PREROUTING链后
iptables -t nat -A PROXY_INIT_OUTPUT -p tcp -m multiport --dports 6000 -j PROXY_INIT_REDIREC #在端口为80时执行PROXY_INIT_REDIREC链