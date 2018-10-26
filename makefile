cc = gcc
prom = a.out
source = main.c operFile.c commonFun.c cJSON.c
lib = -lcurl -L/usr/lib/mysql -lmysqlclient

$(prom):$(source)
	$(cc) -o $(prom) $(source) $(lib)

#a.out:function.c commonFun.c cJSON.c
#	gcc -o a.out function.c commonFun.c cJSON.c -lcurl


