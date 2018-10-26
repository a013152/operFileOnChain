#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <cJSON.h>
#include "commonFun.h" 
#include "operFile.h"
 

int main(int argc,char *argv[])
{
	// char *sourcedata = "123456";
    // int t1 = strlen(sourcedata), t2 = 0;
    // int tempMultLen_ =  (t1/3 + (((t1 % 3) > 0) ? 1: 0)) * 4;   //存放base64的代码
    // char *base64  = (char*) malloc(tempMultLen_); memset(base64, 0 , tempMultLen_);
    // encodeBase64(sourcedata, strlen(sourcedata), base64);  
	// printf("Source:%s, len:%lu \nbase64:%s, len:%lu\n", sourcedata, strlen(sourcedata), base64, strlen(base64) );
	// char * pOrigna = (char*)malloc(tempMultLen_); memset(pOrigna, 0, tempMultLen_); 
	// decodeBase64(base64, strlen(base64), pOrigna , strlen(base64));  
	// printf("base64:%s, len:%lu \nOrigna:%s, len:%lu\n", base64, strlen(base64), pOrigna, strlen(pOrigna) );
	// return 0;
	// saveHashToMysql(1, "0x1626b2c99eedb59c8199ae5d40c6a6a0fa67ea6a5abcd9dbe382054b550e2267", "01.jpg");
	// return 0;
	
	
	//----------保存-----------------------------------------------
	//1、 读取本地文件模拟socket接收到要存储的数据。
	char szFile[] = "01.jpg", preHex[67] = {0} , szAccount[]="0xf58c93ceaeffbb91f45c83022ade9cfe5ef19339"; 
	int lenghtUnit = 2048 ;//单位长度
	int tempMultLen = lenghtUnit * 2; 
	FILE * pf = NULL; char* readbuf = NULL ,* writebuf = NULL , * buffTemp = NULL ;
	if ((pf = fopen(szFile, "rb")) == NULL)
	{
		printf("can not open %s , exit!\n",szFile);
		return 0;//exit( 0);
	}
	fseek(pf, 0l, SEEK_END); //移动到文件末尾
	long flen = ftell(pf);	//计算文件的长度
	
	printf("File:%s .size:%ld \n", szFile, flen);
	if(flen >= lenghtUnit)
	{
		readbuf = malloc(lenghtUnit +1);  //只申请单位长度内存 
		writebuf = malloc(tempMultLen + 67); buffTemp = malloc(tempMultLen);
		long iReadLen =0, iCurenPos = flen, count_= 1, moveReverseLen = 0; 
		memset(preHex, '0', 66); 
		while(iCurenPos > 0){			
			memset(readbuf, 0 ,lenghtUnit+1);	memset(writebuf, 0 ,tempMultLen + 67); memset(buffTemp, 0 ,tempMultLen); 
			iReadLen = 0;
			if((iCurenPos - lenghtUnit) >=0 ){  //是否满足 读单位长度  
				iReadLen = lenghtUnit;
			}
			else{
				iReadLen = flen % lenghtUnit;
			}
			moveReverseLen += (0 - iReadLen); 	 //计算移动长度 （是个负数）
			fseek(pf, moveReverseLen, SEEK_END); //从文件末尾向前移动		
			fread(readbuf, iReadLen, 1, pf); 	 //读取1次长度的内容, 注意当前的位置 
			iCurenPos += moveReverseLen;	     //记录当前位置 （越来越小，等于0时推出循环）
			printf("times %ld | have read %ld bytes ,current ftell:%ld\n",count_++ ,  iReadLen, iCurenPos); 
			  
			BytesToHexStr(readbuf, buffTemp, iReadLen);//转换为十六进制
			memcpy(writebuf, (void*)&preHex[2], 64);
			memcpy(writebuf + 64, buffTemp, iReadLen * 2);
			//printf("writebuf lenght:%lu, %s\n", strlen(writebuf), writebuf);
			saveDataOnChain(szAccount, writebuf, preHex); 
			preHex[66] =0;
			 		
			printf("preHex:%s\n",preHex); 
		}
	}else{		
		readbuf = malloc(flen + 1);
		tempMultLen = flen *2; 
		writebuf = malloc(tempMultLen + 65); buffTemp = malloc(tempMultLen);
		fseek(pf, 0l, SEEK_SET); //移动到文件头
		fread(readbuf, flen, 1, pf); //读取全部内容
		BytesToHexStr(readbuf, buffTemp, flen);//转换为十六进制
			
		memcpy(writebuf, preHex, 64);
		memcpy(writebuf + 64, buffTemp, tempMultLen);
		saveDataOnChain(szAccount, writebuf, preHex);   //存到区块链
		printf("preHex:%s\n",preHex);
	}
	//保存交易hex到mysql 
	saveHashToMysql(1, preHex, szFile);
	
	//清理	
	if(readbuf)		free(readbuf);
	if(writebuf)	free(writebuf);
	if(buffTemp)	free(buffTemp); 
	 
	fclose(pf);
	
	
	//2、 以每4K大小从后向前读取，转化为4Kbase64十六进制后， 拼接上条交易hex保存到区块链，
		/*注意data 拼接格式 preHex+dataPlayload(状态+上一条42位交易hex + 数据负载）
		preHex： 上一条交易hex （42字节）+ 0补齐64位
		dataPlayload： 数据负载 （<= 4096字节）*/
	
	// 3、 判断最后一条交易hex，需要保存到mysql数据库
	
	
	
	
	
	//------------读取---------------------------------------------
	//1、从数据库中读取唯一id 对应的文件名和交易hex
	
	//2、查询区块链，获取input数据，根据 type+preHex+dataPlayload 格式解析，如果上一条交易hex存在则循环查找
	
	//3、按顺序拼接数据后
	
	
	
	
	return 0;
}