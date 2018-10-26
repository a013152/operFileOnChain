
#include "commonFun.h"
#include <curl/curl.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>

static const char *ALPHA_BASE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
 //base64编码
char *encodeBase64(const char *originChar, long originCharSize, char *base64Char) {
    int a = 0;
    int i = 0;
    while (i < originCharSize) {
        char b0 = originChar[i++];
        char b1 = (i < originCharSize) ? originChar[i++] : 0;
        char b2 = (i < originCharSize) ? originChar[i++] : 0;
         
        int int63 = 0x3F; //  00111111
        int int255 = 0xFF; // 11111111
        base64Char[a++] = ALPHA_BASE[(b0 >> 2) & int63];
        base64Char[a++] = ALPHA_BASE[((b0 << 4) | ((b1 & int255) >> 4)) & int63];
        base64Char[a++] = ALPHA_BASE[((b1 << 2) | ((b2 & int255) >> 6)) & int63];
        base64Char[a++] = ALPHA_BASE[b2 & int63];
    }
    switch (originCharSize % 3) {
        case 1:
            base64Char[--a] = '=';
        case 2:
            base64Char[--a] = '=';
    }
    return base64Char;
}
 //base64解码
char *decodeBase64(const char *base64Char, long base64CharSize, char *originChar, long originCharSize){
    int toInt[128] = {-1};
    for (int i = 0; i < 64; i++) {
        toInt[ALPHA_BASE[i]] = i;
    }
    int int255 = 0xFF;
    int index = 0;
    for (int i = 0; i < base64CharSize; i += 4) {
        int c0 = toInt[base64Char[i]];
        int c1 = toInt[base64Char[i + 1]];
        originChar[index++] = (((c0 << 2) | (c1 >> 4)) & int255);
        if (index >= originCharSize) {
            return originChar;
        }
        int c2 = toInt[base64Char[i + 2]];
        originChar[index++] = (((c1 << 4) | (c2 >> 2)) & int255);
        if (index >= originCharSize) {
            return originChar;
        }
        int c3 = toInt[base64Char[i + 3]];
        originChar[index++] = (((c2 << 6) | c3) & int255);
    }
    return originChar;
}



//十六进制字符串转换为字节流
void HexStrToBytes(const char* source, unsigned char* dest, int sourceLen)
{
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i += 2)
	{
		highByte = toupper(source[i]);
		lowByte = toupper(source[i + 1]);

		if (highByte > 0x39)
			highByte -= 0x37;
		else
			highByte -= 0x30;

		if (lowByte > 0x39)
			lowByte -= 0x37;
		else
			lowByte -= 0x30;

		dest[i / 2] = (highByte << 4) | lowByte;
	}
	return;
}

//字节流转换为十六进制字符串的
void BytesToHexStr(const char *source, char *dest, int sourceLen)
{ 
	char szTmp[3];
	for (int i = 0; i < sourceLen; i++)
	{
		sprintf(szTmp, "%02X", (unsigned char)source[i]);
		memcpy(&dest[i * 2], szTmp, 2);
	}
	return;
}

//小写转大写
void upperChars(char * szStr, size_t begin, size_t len)
{ 
	for(size_t i = begin; i < len ;  i++ ){ 
		if( szStr[i]>='A' && szStr[i]<='Z')  
			szStr[i] += 32;  
	}  
}
//大写转小写
void lowerChars(char * szStr, size_t begin, size_t len)
{ 
	for(size_t i = begin; i < len ;  i++ ){ 
		if( szStr[i]>='a' && szStr[i]<='z')  
			szStr[i] -= 32;  
	}
}

void printfCURLError(int res, char* reslutStr)
{
	switch(res)
	{
		case CURLE_OK:			sprintf(reslutStr,"no error");		break;
		case CURLE_UNSUPPORTED_PROTOCOL:			sprintf(reslutStr,"unsupported protocol");		break;
		case CURLE_FAILED_INIT:			sprintf(reslutStr,"failed init ");		break;
		case CURLE_URL_MALFORMAT:			sprintf(reslutStr,"URL using bad/illegal format or missing URL ");		break;
		case CURLE_URL_MALFORMAT_USER:			sprintf(reslutStr,"unknown error ");		break;
		case CURLE_COULDNT_RESOLVE_PROXY:			sprintf(reslutStr,"couldn’t resolve proxy name");		break;
		case CURLE_COULDNT_RESOLVE_HOST:			sprintf(reslutStr,"couldn’t resolve host name");		break;
		case CURLE_COULDNT_CONNECT:			sprintf(reslutStr,"couldn’t connect to server");		break;
		case CURLE_FTP_WEIRD_SERVER_REPLY:			sprintf(reslutStr,"FTP: weird server reply");		break;
		case CURLE_FTP_ACCESS_DENIED:			sprintf(reslutStr,"FTP: access denied");		break;
		case CURLE_FTP_USER_PASSWORD_INCORRECT:			sprintf(reslutStr,"FTP: USER_PASSWORD_INCORRECT");		break;
		case CURLE_FTP_WEIRD_PASS_REPLY:			sprintf(reslutStr,"FTP: unknown PASS reply ");		break;
		case CURLE_FTP_WEIRD_USER_REPLY:		sprintf(reslutStr,"FTP: unknown USER reply ");		break; 
		case CURLE_FTP_WEIRD_PASV_REPLY:		sprintf(reslutStr,"FTP: unknown PASV reply ");		break; 
		case CURLE_FTP_WEIRD_227_FORMAT:		sprintf(reslutStr,"FTP: unknown 227 response format  ");		break; 
		case CURLE_FTP_CANT_GET_HOST:		sprintf(reslutStr,"FTP: can’t figure out the host in the PASV response  ");		break; 
		case CURLE_FTP_CANT_RECONNECT:		sprintf(reslutStr,"FTP: can’t connect to server the response code is unknown");		break;
		case CURLE_FTP_COULDNT_SET_BINARY:		sprintf(reslutStr,"FTP: couldn’t set binary mode");		break;
		case CURLE_PARTIAL_FILE:		sprintf(reslutStr,"Transferred a partial file ");		break;
		case CURLE_FTP_COULDNT_RETR_FILE:		sprintf(reslutStr,"FTP: couldn’t retrieve (RETR failed) the specified file");		break;
		case CURLE_FTP_WRITE_ERROR:		sprintf(reslutStr,"FTP: the post-transfer acknowledge response was not OK ");		break;
		case CURLE_FTP_QUOTE_ERROR:		sprintf(reslutStr,"FTP: a quote command returned error  ");		break;
		case CURLE_HTTP_RETURNED_ERROR:		sprintf(reslutStr,"HTTP response code said error");		break;
		case CURLE_WRITE_ERROR:		sprintf(reslutStr,"failed writing received data to disk/application  ");		break;
		case CURLE_MALFORMAT_USER:		sprintf(reslutStr,"unknown error");		break;
		case CURLE_UPLOAD_FAILED:		sprintf(reslutStr,"upload failed (at start/before it took off) ");		break;
		case CURLE_READ_ERROR:		sprintf(reslutStr,"failed to open/read local data from file/application ");		break;
		case CURLE_OUT_OF_MEMORY:		sprintf(reslutStr,"out of memory  ");		break;
		case CURLE_OPERATION_TIMEOUTED:		sprintf(reslutStr,"a timeout was reached ");		break;
		case CURLE_FTP_COULDNT_SET_ASCII:		sprintf(reslutStr,"FTP could not set ASCII mode (TYPE A)");		break;
		case CURLE_FTP_PORT_FAILED:		sprintf(reslutStr,"FTP command PORT failed ");		break;
		case CURLE_FTP_COULDNT_USE_REST:		sprintf(reslutStr,"FTP command REST failed  ");		break;
		case CURLE_FTP_COULDNT_GET_SIZE:		sprintf(reslutStr,"FTP command SIZE failed ");		break;
		case CURLE_HTTP_RANGE_ERROR:		sprintf(reslutStr,"a range was requested but the server did not deliver it ");		break;
		case CURLE_HTTP_POST_ERROR:		sprintf(reslutStr,"internal problem setting up the POST  ");		break;
		case CURLE_SSL_CONNECT_ERROR:		sprintf(reslutStr,"SSL connect error");		break;
		case CURLE_BAD_DOWNLOAD_RESUME:		sprintf(reslutStr,"couldn’t resume download  ");		break;
		case CURLE_FILE_COULDNT_READ_FILE:		sprintf(reslutStr,"couldn’t read a file");		break;
		case CURLE_LDAP_CANNOT_BIND:		sprintf(reslutStr,"LDAP: cannot bind  ");		break;
		case CURLE_LDAP_SEARCH_FAILED:		sprintf(reslutStr," LDAP: search failed ");		break;
		case CURLE_LIBRARY_NOT_FOUND:		sprintf(reslutStr,"a required shared library was not found ");		break;
		default :		sprintf(reslutStr,"nuknow error!");		break;
	}  
}