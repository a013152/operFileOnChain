#cc = gcc
#prom = a.out
#source = main.c operFile.c commonFun.c cJSON.c
#lib =  -L/usr/lib/mysql -lmysqlclient -lcurl
#$(prom):$(source)
#	$(cc) -o $(prom) $(source) $(lib)

a.out:main.c commonFun.c  operFile.c cJSON.c 
	gcc -o a.out main.c  commonFun.c  operFile.c cJSON.c  -lmysqlclient -L/usr/lib/mysql  -lcurl


