#include <stdio.h> 
#include <stdlib.h>
#include <stdbool.h>
#include <curl/curl.h>
#include <cJSON.h>
#include "commonFun.h" 
#include "operFile.h"

#include "/usr/include/mysql/mysql.h"
#pragma comment(lib, "libmysql.lib")

#define FORIGNKEY  8
#define READFILENAME "01.zip"
#define WRITEFILENAME "02.zip"
 
int test()
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
}	

int FUN1()
{
	//----------保存-----------------------------------------------
	//1、 读取本地文件模拟socket接收到要存储的数据。
	char szFile[] = READFILENAME, preHex[67] = {0} , szAccount[]="0xf58c93ceaeffbb91f45c83022ade9cfe5ef19339"; 
	int lenghtUnit = UNITSIZE ;//单位长度   
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
	memset(preHex, '0', 66); 
	//2、 以每10K大小从后向前读取，转化为20K十六进制后， 拼接上条交易hex保存到区块链，
	if(flen >= lenghtUnit)
	{
		readbuf = malloc(lenghtUnit +1);  //只申请单位长度内存 
		writebuf = malloc(tempMultLen + 65); buffTemp = malloc(tempMultLen);
		long iReadLen =0, iCurenPos = flen, count_= 1, moveReverseLen = 0; 
		
		while(iCurenPos > 0)
		{			
			memset(readbuf, 0 ,lenghtUnit+1);	memset(writebuf, 0 ,tempMultLen + 65); memset(buffTemp, 0 ,tempMultLen); 
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
			iCurenPos += moveReverseLen;	     //记录当前位置 （越来越小，等于0时退出循环）
			printf("times %ld | have read %ld bytes ,current ftell:%ld\n",count_++ ,  iReadLen, iCurenPos); 
			  
			BytesToHexStr(readbuf, buffTemp, iReadLen);//转换为十六进制
			
			/*注意data 拼接格式 preHex+dataPlayload(状态+上一条交易Hash + 数据负载）
			preHex： 上一条交易hex （64字节）
			dataPlayload： 数据负载 （<= 单位长度字节）*/
			memcpy(writebuf, (void*)&preHex[2], 64);
			memcpy(writebuf + 64, buffTemp, iReadLen * 2);
			//printf("writebuf lenght:%lu, %s\n", strlen(writebuf), writebuf);
			saveDataOnChain(szAccount, writebuf, preHex); 
			preHex[66] =0;
			 		
			printf("preHex:%s\n",preHex); 
		}
	}
	else
	{		
		readbuf = malloc(flen + 1);
		tempMultLen = flen *2; 
		writebuf = malloc(tempMultLen + 65); buffTemp = malloc(tempMultLen); memset(writebuf, 0 ,tempMultLen + 65);
		fseek(pf, 0l, SEEK_SET); //移动到文件头
		fread(readbuf, flen, 1, pf); //读取全部内容
		BytesToHexStr(readbuf, buffTemp, flen);//转换为十六进制
		printf("times 1 | have read %ld bytes ,current ftell:%ld\n", flen, ftell(pf)); 	
		memcpy(writebuf,(void*)&preHex[2], 64); 
		memcpy(writebuf + 64, buffTemp, tempMultLen);
		//printf("writebuf lenght:%lu, %s\n", strlen(writebuf), writebuf);
		saveDataOnChain(szAccount, writebuf, preHex);   //存到区块链
		printf("preHex:%s\n",preHex);
	}
	//3、  保存最后一条交易hex， 到mysql数据库 
	saveHashToMysql(FORIGNKEY, preHex, szFile);
	
	//4、 清理	
	if(readbuf)		free(readbuf);
	if(writebuf)	free(writebuf);
	if(buffTemp)	free(buffTemp); 	 
	fclose(pf);
	
	return 0;
} 

void FUN2()
{ 
	//------------读取---------------------------------------------
	//1、从数据库中读取唯一id 对应的文件名和交易hex
	char szHash[67] = {0};
	if(readHashFromMysql(FORIGNKEY, szHash) != 0)
		;
	printf("szHash:%s\n",szHash);
	 
	//2、查询区块链，获取input数据，根据 preHex+dataPlayload 格式解析，如果上一条交易hex存在则循环查找
 
	char szData[BUFSIZE] = {0}, szData2[BUFSIZE] = {0}, unit_w = 1; 
	bool nextFlag = false;
	int lenData = 0;
	 
	
	FILE * pf = NULL;  int flen = 0;
	char *fileData  = (char*)malloc(BUFSIZE );
	char *fileName = WRITEFILENAME;
	char pBuff[BUFSIZE] ={0};
	if ((pf = fopen(fileName, "wb+")) == NULL)
	{
		printf("can not open %s!\n",fileName);
		return ;//exit( 0);
	} 
	
	do
	{ 
		if(readDataFromChain(szHash,szData) != 0) //查询区块链
			break; 
		lenData = strlen(szData);
		//printf("lenData:%d szData:%s\n", lenData, szData);
		/*注意data 格式 preHex+dataPlayload(状态+上一条交易hash + 数据负载）
			preHex： 上一条交易hex （64字节）
			dataPlayload： 数据负载 （<= 单位长度字节）*/
		memcpy(szHash, szData, 66); szHash[66] = 0;
		memcpy(szData2, &szData[66], lenData - 66); szData2[lenData - 66] = 0;
		
		//3、保存到本地 
		flen = strlen(szData2); 
		HexStrToBytes(szData2,fileData,flen); 	
		int reslut_ =(int)fwrite(fileData, flen/2, unit_w, pf); 
		if(reslut_ != unit_w)
		{
			printf("%s write fail error! fwrite return:%d, flen/2:%d\n", fileName, reslut_, flen/2);
			break;
		} 
		
		//判断下一条hash 
		nextFlag = (strcmp(szHash , "0x0000000000000000000000000000000000000000000000000000000000000000") != 0);
		printf("nextFlag:%d  szHash:%s\n",nextFlag, szHash);
	}while(nextFlag);
	
	 
	fclose(pf); 
	free(fileData); 
	
} 

int main(int argc,char *argv[])
{
	 
	//test(); 
	
	//FUN1();  //存到区块链
	
	FUN2();   //从区块链上读取
	
	return 0;
}