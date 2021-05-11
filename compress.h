#define local static
#define LGCHUNK 14
#define CHUNK (1U << LGCHUNK)
#define DSIZE 32768U

/* structure for gzip file read operations */
typedef struct {
	int fd;                     /* file descriptor */
	int size;                   /* 1 << size is bytes in buf */
	unsigned left;              /* bytes available at next */
	unsigned char *buf;         /* buffer */
	z_const unsigned char *next;    /* next byte in buffer */
	char *name;                 /* file name for error messages */
} file;

local void bye(char *msg1, char *msg2);
local unsigned gcd(unsigned a, unsigned b);
local void rotate(unsigned char *list, unsigned len, unsigned rot);
local int readin(file *in);
local int readmore(file *in);
local void skip(file *in, unsigned n);
unsigned long read4(file *in);
local void gzheader(file *in);
void gztack_str(char *target, int gd, z_stream *strm, int last);
int gzscan(char *name, z_stream *strm, int level);
void build_file_name(char *prefix,char *postfix,unsigned int num, char *fname);
void creat_gzip_file(char *filename);
