#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <ctype.h>
#include <zlib.h>
#include "db.h"

void creat_gzip_file(char *filename){
    if(access( filename, F_OK )) {
        char prefix[1024];
        memset(prefix,'\0',sizeof(char)*1024);
        strcpy(prefix,"gzip - > \0");
        strcat(prefix,filename);
        FILE *pipe = popen(prefix, "w");
        pclose(pipe);
    }
}

void build_file_name(char *prefix,char *postfix,unsigned int num, char *fname){
	sprintf(fname, "%s%u%s", prefix,num,postfix);
}

void init_db(struct url_db *db){
	db->maxhash = MAX_HASH;
	memset(db->file_prefix,'\0',sizeof(char)*32);
    for(int i=0;i<MAX_HASH;i++){
        db->file_lines[i] = 0;
        db->handled_idx[i] = 0;
    }
}

unsigned int hash33(unsigned char *key){
	unsigned char *ptr = key;
	unsigned int hashv;

	hashv = 0;
	while (*ptr){
		hashv = (hashv << 5) + hashv + *ptr++;
	}
	return hashv;
}

void md5(char *word, char *hash_code){
    memset(hash_code,'\0',sizeof(33*sizeof(char)));
	MD5_CTX md5_context;
	MD5_Init(&md5_context);
	unsigned char decrypt[17];							  //存放加密後的結果
	MD5_Update(&md5_context, word, strlen((char *)word)); //對欲加密的字符進行加密
	MD5_Final(decrypt, &md5_context);
	char *ktr = hash_code;
	for (int i = 0; i < 16; i++)
	{
		sprintf(ktr, "%02x", decrypt[i]);
		ktr += 2;
	}
	ktr = '\0';
}

void get_ip_md5(char *ip,char *hash_code){
    memset(hash_code,'\0',sizeof(33*sizeof(char)));
	MD5_CTX md5_context;
	MD5_Init(&md5_context);
	unsigned char decrypt[17];							  //存放加密後的結果
	MD5_Update(&md5_context, ip, strlen((char *)ip)); //對欲加密的字符進行加密
	MD5_Final(decrypt, &md5_context);
	char *ktr = hash_code;
	for (int i = 0; i < 16; i++)
	{
		sprintf(ktr, "%02x", decrypt[i]);
		ktr += 2;
	}
	ktr = '\0';
}

unsigned int insert_url(struct url_db *db,char *url,char *ip,char *fprefix){
    char hashcode[33];
    char insert_line[BUFFER_SIZE];
    memset(insert_line,0,sizeof(char)*BUFFER_SIZE);
    memset(hashcode,0,sizeof(char)*33);
    get_ip_md5(ip,hashcode);
    unsigned int hv = hash33(hashcode);
    hv = hv % db->maxhash;
    strcat(insert_line,url);
    strcat(insert_line," ");
    strcat(insert_line,ip);
    strcat(insert_line,"\n");
    char postfix[] = ".gz";
    char filename[1024];
    memset(filename,0,sizeof(char)*1024);
    build_file_name(fprefix,postfix,hv,filename);
	gzFile fp = gzopen(filename,"a");
    gzwrite(fp,insert_line,strlen(insert_line));
    gzclose(fp);
    db->file_lines[hv] += 1;
	return hv;
}

void get_from_db(struct url_db *db,char *buf,int bufsize,unsigned int file_num){
	char postfix[] = ".gz";
    char filename[1024];
    memset(filename,0,sizeof(char)*1024);
	memset(buf,0,sizeof(char)*bufsize);
    build_file_name(db->file_prefix,postfix,file_num,filename);
	gzFile fp = gzopen(filename,"r");
	if(fp){
		off_t seek_pos = gzseek(fp,db->handled_idx[file_num],SEEK_SET);
		int len = gzread(fp,buf,bufsize-1);
		if(len){
			char *ptr = buf;
			ptr += len;
			while (*(ptr-1)!='\n'){
				*(ptr-1) = '\0';
				len--;
				if (ptr-1 == buf){
					break;
				}
				ptr--;
			}
			printf("file len:%d\n",len);
			db->handled_idx[file_num] += len;
		}
	}
	gzclose(fp);
}