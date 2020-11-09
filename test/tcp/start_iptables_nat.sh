iptables -t nat -N MY_TCP #在nat表上新建名为MY_TCP自定义链
iptables -t nat -p tcp -A MY_TCP -j REDIRECT --to-ports 8081 #将进入MY_TCP链的数据包端口重定向到8081上
iptables -t nat -N MYNAT #在nat表上新建名为MYNAT自定义链
iptables -t nat -A PREROUTING -p tcp -j MYNAT #将MYNAT加入到PREROUTING链后
iptables -t nat -A MYNAT -p tcp -m multiport --dports 80 -j MY_TCP #在端口为80时执行MY_TCP链