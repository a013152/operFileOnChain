#ifndef _COMMNIFUN_H
#define _COMMNIFUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define  BUFSIZE	21504

#define CURL_IP_PORT  "127.0.0.1:8101"
#define  DB_HOST		"172.16.0.206"  
#define  DB_USER		"root"
#define  DB_PSW			"root"
#define  DB_NAME		"tongdao"
#define  DB_PORT		3306
 
	
#define  CONN_TIMEOUT   5
#define  RECO_TIMEOUT   1


//结构体：CURL读取数据时的回调输出参数
typedef struct _stReadData{
	unsigned int Offset;   //记录读取位置
	char* pReceivBuf;
}stReadData;
 
 
//    定义链表中的节点  
typedef struct node  
{  
    char* member;             //    节点中的成员  
    struct node *pNext;       //    指向下一个节点的指针  
}Node,*pNode; 

 
 
//函数名称: encodeBase64
//功能描述:base64编码
//参数： 参数1 原数据流， 
//		 参数2 原数据流长度， 
//		 参数3 存放编码后的base64字符buff  (注意:buff长度>=原数据流长度+2)
char *encodeBase64(const char *originChar, long originCharSize, char *base64Char);
 

//函数名称: decodeBase64
//功能描述:base64解码
//参数： 参数1 base64字符串， 
//		 参数2 base64字符串长度， 
//		 参数3 存放解码后原buff   
//		 参数4  原buff  长度，要求>=base64字符串长度-2
char *decodeBase64(const char *base64Char, long base64CharSize, char *originChar, long originCharSize);
  

//函数名称: BytesToHexStr
//功能描述:字节流转换为十六进制字符串
//参数： 参数1 字节流原buff，
//		 参数2 存放转化十六进制后的buff，
//		 参数3 字节流原buff的长度， 
//注意： 参数2申请的存放十六进制的buff长度需要大于或等于原buff的2倍。
void BytesToHexStr(const char *source, char *dest, int sourceLen);

//函数名称: HexStrToBytes
//功能描述:十六进制字符串转换为字节流
//参数： 参数1 十六进制字符buff，
//	 	 参数2 存放转化字符流的buff，
//		 参数3 十六进制字符buff的长度， 
//注意：参数2申请的存放转化字符流的buff长度需要大于等于十六进制字符的1/2。
void HexStrToBytes(const char* source, unsigned char* dest, int sourceLen);


//函数名称: printfCURLError
//功能描述:获取curl的错误码对应的描述。
//参数： 参数1 错误码， 
//       参数2 存放描述的字符buff   
void printfCURLError(int res, char* reslutStr);

//函数名称: upperChars
//功能描述:大写转小写
//参数： 参数1 字符， 
//       参数2 开始位置   
//       参数3 结束位置 
void upperChars(char * szStr,  size_t begin, size_t len);

//函数名称: lowerChars
//功能描述:小写转大写
//参数： 参数1 字符， 
//       参数2 开始位置   
//       参数3 结束位置 
void lowerChars(char * szStr, size_t begin, size_t len);

#endif