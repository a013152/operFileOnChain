#ifndef _OPERFINE_H
#define _OPERFINE_H



//函数名称: saveFileOnChain
//功能描述: 保存数据到区块链
//参数： 参数1 账户， 
//		 参数2 原数据，   （注意长度，建议<=4k）
//		 参数3 输出交易Hash
//返回值： 0 成功  -1 长度超过BUFFSIZE
int saveDataOnChain(char* account, char* dataBuf, char* outTXHash);


//函数名称: saveHashToMysql
//功能描述: 保存hash到mysql数据库
//参数： 参数1 外键， 
//		 参数2 交易hash
//		 参数3 文件名称
//返回值： 0 成功   
int saveHashToMysql(int forignkey, char* hash, char * fileName);


#endif