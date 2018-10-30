#ifndef _OPERFINE_H
#define _OPERFINE_H



//函数名称: saveFileOnChain
//功能描述: 保存数据到区块链
//参数： 参数1 账户， 
//		 参数2 原数据，   （注意长度，建议<=4k）
//		 参数3 输出交易Hash
//返回值： 0 成功   
int saveDataOnChain(char* account, char* dataBuf, char* outTXHash);


//函数名称: saveHashToMysql
//功能描述: 保存hash到mysql数据库
//参数： 参数1 外键， 
//		 参数2 交易hash
//		 参数3 文件名称
//返回值： 0 成功   
int saveHashToMysql(int forignkey, char* hash, char * fileName);



//函数名称: readHashFromMysql
//功能描述: 从mysql读取hash
//参数： 参数1 外键， 
//		 参数2 输出交易hash
//		 
//返回值： 0 成功   
int readHashFromMysql(int forignkey,  char* outHash);


//函数名称: readDataFromChain
//功能描述: 从区块链上读取hash
//参数： 参数1 交易hash， 
//		 参数2 输出 交易中的input数据  （注意malloc足够长度）
//		 
//返回值： 0 成功   
int readDataFromChain(char * hash, char* data);

#endif