#include "operFile.h"
#include "commonFun.h" 
#include <curl/curl.h>
#include <cJSON.h>
#include "/usr/include/mysql/mysql.h"
#pragma comment(lib, "libmysql.lib")

char receiveBuf[BUFSIZE] = { 0 }; 
char sendData[BUFSIZE] = { 0 };

//回调函数
int curl_callback(void* buffer, size_t size, size_t nmemb, char * buff)  
{  
	//printf("recei buff size:%d, nmemb:%d, size*nmemb:%d \n", (int)size, (int)nmemb, (int)(size * nmemb)); 
	unsigned long length =0;  //拷贝的长度
	if (nmemb > BUFSIZE)
	{
		printf("recei lenght too long\n");
		length = BUFSIZE -1;
	}else{
		length =size*nmemb;
	}
	strncpy(buff, buffer, length);
	return length;  
}  

//通过发送交易保存到区块链
int saveDataOnChain(char* account, char* dataBuf, char* outTXHash)
{ 
	int result_ = 0;
	// 初始化curl
	CURL *curl = curl_easy_init();
	do{
		// 设置目标url
		curl_easy_setopt(curl, CURLOPT_URL, CURL_IP_PORT);	
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Content-type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		//设置发送数据   
		int flen = strlen(dataBuf);
		if(flen < BUFSIZE - 250){		
			sprintf(sendData, "{\"method\":\"eth_sendTransaction\",\"params\":[{\"from\":\"%s\",\"gas\":\"0xfffff\",\"data\":\"0x%s\"}],\"id\":1,\"jsonrpc\":\"2.0\"}",account,dataBuf);
			//printf("sendData:%s\n", sendData); 
		} else{
			result_ =  -1;
			break;
		}
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sendData); 
		
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback); //对返回的数据进行操作的函数地址 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, receiveBuf);
		
		curl_easy_setopt(curl, CURLOPT_POST, 1);  //设置post 访问 
		
		curl_easy_setopt(curl, CURLOPT_TIMEOUT,5L);//设置超时时间 
		
		memset(receiveBuf, 0, BUFSIZE);
		int res = curl_easy_perform(curl);// 执行操作
		
		
		
		printf("res:%d, receiveBuf:%s\n",res, receiveBuf);
		if(res == CURLE_OK){
			//解析json，或者交易hex
			cJSON * json = cJSON_Parse(receiveBuf);
			 
			if (json)
			{	 
				cJSON *resultObj = NULL;
				resultObj = cJSON_GetObjectItem(json, "result");
				if (resultObj){  
					if(resultObj->type == cJSON_String)
					{
						//保存交易hash数据
						int lenght_ = strlen(resultObj->valuestring);
						if(BUFSIZE > lenght_){ 
							strncpy(outTXHash, resultObj->valuestring, lenght_);
							outTXHash[lenght_] = 0;							
						}
						else {
							printf("BUFSIZE:%d < lenght_%d\n", BUFSIZE,lenght_); result_ =  -3;
						}
					}else {
						printf("input type is not string \n"); result_ =  -3;
					}  
				}else{
					printf("result is null\n"); result_ =  -3;
				}
			}
			else{
				printf("json is  null\n"); result_ =  -3;
			}  
			
		}else{
			result_ =  -2;
			char szError[256] ={0}; printfCURLError(res, szError);
			printf("curl_error:%d, %s",res, szError);
			break;
		}
		
	}while(0); 
	
	curl_easy_cleanup(curl);
	return result_;
}














int saveHashToMysql(int forignkey, char* hash, char * fileName)
{
	MYSQL conn ,*sock;
	time_t timeout1 = CONN_TIMEOUT, timeout2 = RECO_TIMEOUT;
	mysql_init(&conn);
	if (mysql_options(&conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout1)){
		perror(mysql_error(&conn));  return -1;  //mysql_error(&conn)
	}
	if (mysql_options(&conn, MYSQL_OPT_RECONNECT, &timeout2)){
		perror(mysql_error(&conn));  return -1;  //mysql_error(&conn)
	}
	 if((sock = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PSW, DB_NAME, DB_PORT, NULL, 0)) == NULL){
		 perror("read_database_handler()");  return -1;
	 }
	// if ( (sock = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PSW, DB_NAME, DB_PORT, NULL, 0) ) == NULL ) //连接MySQL 
	// { 
		// printf("fail to connect mysql \n"); 
		// fprintf(stderr, " %s\n", mysql_error(&conn)); 
		 // return -1;
	// } 
	
	if(mysql_set_character_set(&conn,"utf8")){
		perror(mysql_error(&conn));  return -1;
	}

	char queryString[BUFSIZE];
	sprintf(queryString, "INSERT INTO file_hash (foreign_key, chain_hash, filename, remark) VALUES(%d,\'%s\',\'%s\',\'测试备注\')",forignkey, hash, fileName);
	
	//printf("queryString:%s\n",queryString);
	
	if(mysql_query(&conn, queryString)){
		perror(mysql_error(&conn));
		return -1;
	}
	
	mysql_close(&conn);
	mysql_close(sock); //关闭连接 
	printf("Mysql save hash: %s\n", hash);
	return 0;
}


