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
			sprintf(sendData, "{\"method\":\"eth_sendTransaction\",\"params\":[{\"from\":\"%s\",\"gas\":\"0xfffff\",\"data\":\"%s\"}],\"id\":1,\"jsonrpc\":\"2.0\"}",account,dataBuf);
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


int curl_callback_pack(void* buffer, size_t size, size_t nmemb, void * pStRead_)  
{  
	stReadData* pStRead = (stReadData*)pStRead_;
	//rintf("curl callback recei buff size:%lu, nmemb:%lu, size*nmemb:%lu \n", size, nmemb, size * nmemb); 
	unsigned long length =0;  //拷贝的长度
	if (nmemb+pStRead->Offset > BUFSIZE)
	{ 
		length = BUFSIZE -1;
	}else{
		length =size*nmemb;
	}
	strncpy(&pStRead->pReceivBuf[pStRead->Offset], buffer, length);
	pStRead->Offset += length;
	return length;  
} 


int readDataFromChain(char * hash, char* data)
{
	CURL *curl;
	curl = curl_easy_init();
	// 设置目标url
	curl_easy_setopt(curl, CURLOPT_URL, CURL_IP_PORT);	
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-type: application/json");
	headers = curl_slist_append(headers, "Accept-Encoding: gzip, deflate");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	//设置发送数据   
	sprintf(sendData, "{\"method\":\"eth_getTransactionByHash\",\"params\":[\"%s\"],\"id\":1,\"jsonrpc\":\"2.0\"}",hash);	 
	//printf("sendData:%s\n", sendData);  
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sendData);
	
	//对返回的数据进行操作的函数地址  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback_pack);
	char* pBuff = (char*)malloc(BUFSIZE); memset(pBuff,0,BUFSIZE);
	stReadData obj ;obj.Offset = 0;obj.pReceivBuf=pBuff;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&obj);

	//设置问非0表示本次操作为post  
	curl_easy_setopt(curl, CURLOPT_POST, 1); 
	
	//设置超时时间
	curl_easy_setopt(curl, CURLOPT_TIMEOUT,5L); 
	
	int res = curl_easy_perform(curl);
	
	//printf("curl_easy_perform res:%d\n",res);
	if(res != 0){
		char szTemp[256] = {0};
		printfCURLError(res , szTemp);
		printf("%s\n",szTemp);
		return -1;
	}
	curl_easy_cleanup(curl); 
	
	
	//解析json 里的 input 数据
	cJSON * json = cJSON_Parse(obj.pReceivBuf);
	char * json_data = NULL; 
	if (json)
	{	 
		cJSON *resultObj = NULL;
		resultObj = cJSON_GetObjectItem(json, "result");
		if (resultObj){
			
			cJSON *inputObj = cJSON_GetObjectItem(resultObj, "input");
			if(inputObj)
			{
				json_data = cJSON_Print(inputObj);
				if(inputObj->type == cJSON_String)
				{
					//保存数据
					int lenght_ = strlen(inputObj->valuestring);
					if(BUFSIZE > lenght_){ 
						strcpy(data, inputObj->valuestring);
						 data[lenght_] = 0;
						// char * pdata = &buff[2];  //截取
						// cJSON_ReplaceItemInObject(resultObj,"input",cJSON_CreateString(pdata));  //用于代替resultObj对象中input元组的值 
						// inputObj = cJSON_GetObjectItem(resultObj, "input");
						// strcpy(buff, inputObj->valuestring);
						//printf("getInputData2:%s\n", buff);
					}
					else {
						printf("BUFSIZE:%d < lenght_%d\n", BUFSIZE,lenght_);
						return -1;
					}
				}else {
					printf("input type is not string \n");
					return -1;
				}
			}else{
				printf("input is null\n");
				return -1;
			}
			
		}else{
			printf("result is null\n");
			return -1;
		}
	}
	else{
		printf("json is  null\n");
	} 
	free(pBuff);
	return 0;
} 



int saveHashToMysql(int forignkey, char* fileHash, char * fileName)
{
	MYSQL conn ,*sock;
	time_t timeout1 = CONN_TIMEOUT, timeout2 = RECO_TIMEOUT;
	mysql_init(&conn);
	do
	{
		if (mysql_options(&conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout1)){
			perror(mysql_error(&conn));  return -1;break;  //mysql_error(&conn)
		}
		if (mysql_options(&conn, MYSQL_OPT_RECONNECT, &timeout2)){
			perror(mysql_error(&conn)); return -1; break;  //mysql_error(&conn)
		}
		 if((sock = mysql_real_connect(&conn, DB_HOST, DB_USER, DB_PSW, DB_NAME, DB_PORT, NULL, 0)) == NULL){
			 perror("read_database_handler()"); return -1; break;
		 }
	 
		if(mysql_set_character_set(&conn,"utf8")){
			perror(mysql_error(&conn)); return -1; break;
		}

		char queryString[BUFSIZE];
		sprintf(queryString, "INSERT INTO file_hash (foreign_key, data_chain_hash, annex_chain_hash, filename, remark) VALUES(%d,\'%s\',\'%s\',\'%s\',\'测试备注\')",forignkey, fileHash,fileHash, fileName);
		
		//printf("queryString:%s\n",queryString);
		
		if(mysql_query(&conn, queryString)){
			perror(mysql_error(&conn));return -1;
			break;
		} 
	}  while(0);
	mysql_close(&conn);
	mysql_close(sock); //关闭连接 
	
	return 0;
}


int readHashFromMysql(int forignkey, char* outHash)
{
	int res = 0 ;
	 
	//读取mysql
	 // static int l= 0;
	MYSQL mysql; //声明MySQL的句柄 
	MYSQL_RES * result; //保存结果集的
	MYSQL_ROW row; //代表的是结果集中的一行  
	
	time_t timeout1 = CONN_TIMEOUT, timeout2 = RECO_TIMEOUT;
	char szSql[256] = {0} ; sprintf(szSql,"select * from file_hash where foreign_key=%d", forignkey); //查询语句
	//printf("%d\n", ++l);
	mysql_init(&mysql); 
	do
	{ 
		if (mysql_options(&mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout1)){
			perror(mysql_error(&mysql));res = -1;break;   
		} 
		if (mysql_options(&mysql, MYSQL_OPT_RECONNECT, &timeout2)){
			perror(mysql_error(&mysql));res = -1; break;  
		} 
		printf("try to connect msyql ->");
		if (mysql_real_connect(&mysql, DB_HOST, DB_USER, DB_PSW, DB_NAME, DB_PORT, NULL, 0) == NULL) //连接MySQL 
		{
			perror("fail to connect mysql:");res = -1;break;
		} else {
			printf(" connect ok.\n");
		}
		do{
			if(mysql_set_character_set(&mysql,"utf8")){
				perror(mysql_error(&mysql)); res = -1; break;
			} 
			
			if ( mysql_query(&mysql, szSql) != 0 ) //如果连接成功，则开始查询 .成功返回0
			{ 
				printf("fail to query!\n");res = -1;	break; 
			} 
			
			if ( (result = mysql_store_result(&mysql)) == NULL ) //保存查询的结果 
			{ 
				printf("fail to store result!\n");  break; 
			} 
			else 
			{ 
				while ( (row = mysql_fetch_row(result)) != NULL ) //读取结果集中的数据，返回的是下一行。 】 
				{   
					memcpy(outHash, row[3], 66);  // row[2] datahash   ,row[3] 文件hash
				} 
			} 
		 } 
		 while(0);
		 mysql_free_result(result); //释放结果集 
	}while(0); 
	mysql_close(&mysql); //关闭连接  
	return res;
}


