#!/usr/bin/env bash

#使用nat表做数据转发
iptables -t nat -D PREROUTING -p tcp -j PROXY_INIT_OUTPUT
iptables -t nat -D OUTPUT -p tcp -j PROXY_INIT_OUTPUT
iptables -t nat -F PROXY_INIT_OUTPUT
iptables -t nat -X PROXY_INIT_OUTPUT
iptables -t nat -F PROXY_INIT_REDIREC
iptables -t nat -X PROXY_INIT_REDIREC
