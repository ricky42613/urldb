#define MAX_HASH 1<<13
#define URL_LEN 1<<9
#define IP_LEN 1<<4
#define MAX_DEEP 5
#define MAX_BUF_SIZE 20
#define BUFFER_SIZE 8192
#define MAX_FILE_NAME 1<<5
typedef struct url_db url_db;

struct url_db{
    unsigned int file_lines[MAX_HASH];
    unsigned int handled_idx[MAX_HASH];
    unsigned int maxhash;
    char file_prefix[MAX_FILE_NAME];
};
void creat_gzip_file(char *filename);
void build_file_name(char *prefix,char *postfix,unsigned int num, char *fname);
void init_db(struct url_db *db);
unsigned int hash33(unsigned char *key);
void get_ip_md5(char *ip,char *hash_code);
unsigned int insert_url(struct url_db *db,char *url,char *ip,char *fprefix);
void get_from_db(struct url_db *db,char *buf,int bufsize,unsigned int file_num);
