#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ctype.h>
#include<stdint.h>
#include <netinet/in.h>
#include <math.h>
#include <arpa/inet.h>
#include <time.h>

#define NAME_LENGTH 100
#define MODE_LENGTH 8
#define UID_LENGTH 8
#define GID_LENGTH 8
#define SIZE_LENGTH 12
#define MTIME_LENGTH 12
#define CHKSUM_LENGTH 8
#define TYPEFLAG_LENGTH 1
#define LINKNAME_LENGTH 100
#define MAGIC_LENGTH 6
#define VERSION_LENGTH 2
#define UNAME_LENGTH 32
#define GNAME_LENGTH 32
#define DEVMAJOR_LENGTH 8
#define DEVMINOR_LENGTH 8
#define PREFIX_LENGTH 155
#define NAME_OFFSET 0
#define MODE_OFFSET 100
#define UID_OFFSET 108
#define GID_OFFSET 116
#define SIZE_OFFSET 124
#define MTIME_OFFSET 136
#define CHKSUM_OFFSET 148
#define TYPEFLAG_OFFSET 156
#define LINKNAME_OFFSET 157
#define MAGIC_OFFSET 257
#define VERSION_OFFSET 263
#define UNAME_OFFSET 265
#define GNAME_OFFSET 297
#define DEVMAJOR_OFFSET 329
#define DEVMINOR_OFFSET 337
#define PREFIX_OFFSET 345
#define REG_TYPEFLAG "0"
#define SYM_TYPEFLAG "2"
#define DIR_TYPEFLAG "5"
#define USTAR "ustar\0"
#define OCTAL 8
#define BLOCK_SIZE 512
#define MAGIC_VALUE 559
#define VERSION_VALUE 96
#define EOF_OFFSET 11
#define CHK_VALUE 256

void read_file(char *name, char *path, int namecount, int *arcfd);
void write_header(int *arcfd, struct stat *sb, char *name);
void write_contents(int *arcfd, char *rfile);
void print_usage_and_exit();
void isinclude(int *c, int *t, int *x, int *v, int *s, int *f, char *code);
void create_archive(int *arcfd, char *name, int namecount, char *lastname);
void write_ascii(int num, int length, int *arcfd);
void write_word(char *word, int length, int *arcfd);
char *decimal_2_octal(int num, int length);
void end_archive(int *arcfd, int block);
int str_2_int(char *word);
void printpermissions(char readbuf[]);
void printowner(char readbuf[]);
void printsize(char readbuf[]);
void printtime(char readbuf[]);
void printname(char readbuf[]);

static int blockcount = 0;
int main(int argc, char *argv[])
{
    int arcfd, f=0, c=0, t=0, x=0, v=0, s=0, i=3;
    int namecount = 0;
    char name[256] = {0};
    char *path; 
    if(argc == 0)
        print_usage_and_exit();
    if(argc < 3)
        print_usage_and_exit();
    isinclude(&c, &t, &x, &v, &s, &f, argv[1]);
    if(c == 1)
    {
        arcfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if(argv[3] == NULL)
            print_usage_and_exit();
        while(argv[i] != NULL)
        {
            path = argv[i];
            if(c == 1)
            {
                read_file(name, path, namecount, &arcfd);
            }
            i++;
        }
        end_archive(&arcfd, BLOCK_SIZE);
        close(arcfd);
    }
    if(t == 1)
    {
        int in = open(argv[2], O_RDONLY);
        int check;
        char readbuf[512];
        int ind;
        char* ustar = "ustar";
        while((check = read(in, readbuf, 512) ) > 0 ){
            
            ind = 0;
            
            char checku[6];
            while(ind != 5){
                checku[ind] = readbuf[257 + ind];
                ind++;
            }
            checku[ind]= '\0';
            if(strcmp(checku, ustar) == 0){
                if(argc > 3){
                    char* path; 
                    ind = 0;
                    char name[100];
                    char prefix[156];

                    if( readbuf[345] != '\0'){  
                        while(readbuf[345 + ind] != '\0' && ind < 155){
                            prefix[ind] = readbuf[345 + ind];
                            ind++;
                        }
                        prefix[ind] = '/';
                        ind++;
                        prefix[ind] = '\0';
                        
                    }
                    ind =0;
                    while(readbuf[ind] != '\0' && ind < 100){
                        name[ind] = readbuf[ind];
                        
                        ind++;
                    }
                    name[ind]= '\0';

                    char *total = name;
                    if( readbuf[345] != '\0'){
                       total = strcat(prefix, name);
                       
                    }
                    
                    int iter = 3;
                    int skip = 0;
                    while(iter != argc){
                        path = argv[iter];
                        char *ptr = strstr(total, path);
                        if( ptr != NULL){
                            skip = 0;
                            break;
                        }else{
                            skip = 1;
                        }
                        iter++;

                    }
                    if(skip == 1){
                        continue;
                    }
                }
                if(v != 0){
                    printpermissions(readbuf);
                    printowner(readbuf);
                    printsize(readbuf);
                    printtime(readbuf);
                }
                printname(readbuf);
            }  
        }
        close(in);
    }
    
    close(arcfd);
    return 0;
}

void read_file(char *name, char *path, int namecount, int *arcfd)
{
    int i;
    struct stat sb;
    int len = strlen(path);
    char newname[256] = {0};
    printf("path: %s\n", path);
    if(stat(path, &sb) == 0)
    {
        for(i = 0; i < len; i++)
        {
            name[namecount] = path[i];
            namecount++;
        }
        if(S_ISDIR(sb.st_mode))
        {
            name[namecount] = '/';
            namecount++;
        }
        for(i = 0; i < namecount; i++)
        {
            newname[i] = name[i];
        }
        printf("newname: %s\n", newname);
        write_header(arcfd, &sb, newname);
        if(S_ISDIR(sb.st_mode))
        {
            struct dirent *pdirent;
            DIR *pdir;
            chdir(path);
            pdir = opendir(".");
            while((pdirent = readdir(pdir)) != NULL)
            {
                chdir(path);
                if((strcmp(pdirent->d_name, ".") != 0) && 
                            (strcmp(pdirent->d_name, "..") != 0))
                {
                    //struct stat sb2;
                    //printf("d_name: %s\n", pdirent->d_name);
                    // if(stat(pdirent->d_name, &sb2) == 0)
                    // {
                        read_file(newname, pdirent->d_name, namecount, arcfd);
                    //}
                }
            }
            free(pdirent);
        }
        else if(S_ISREG(sb.st_mode))
        {
            write_contents(arcfd, path);
        }
    }
}

void write_header(int *arcfd, struct stat *sb, char *name)
{
    // struct stat sb;
    // if(stat(name, &sb) == 0)
    // {

        struct passwd *pwd;
        struct group *pgrp;
        unsigned int sum = 0, i, len = strlen(name), len2;
        if(len < NAME_LENGTH)
            write(*arcfd, name, len);
        else
            write(*arcfd, name, NAME_LENGTH);
        for(i = 0; i < len; i++)
        {
            sum += name[i];
        }
        lseek(*arcfd, NAME_LENGTH-len, SEEK_CUR);
        sum+= str_2_int(decimal_2_octal(sb->st_mode&0xFFF, MODE_LENGTH));
        write_ascii(sb->st_mode&0xFFF, MODE_LENGTH, arcfd);
        sum+= str_2_int(decimal_2_octal(sb->st_uid, UID_LENGTH));
        write_ascii(sb->st_uid, UID_LENGTH, arcfd);
        sum+= str_2_int(decimal_2_octal(sb->st_gid, GID_LENGTH));
        write_ascii(sb->st_gid, GID_LENGTH, arcfd);
        if(S_ISDIR(sb->st_mode) || S_ISLNK(sb->st_mode))
        {
            sum+= str_2_int(decimal_2_octal(0, SIZE_LENGTH));
            write_ascii(0, SIZE_LENGTH, arcfd);
        }
        else{
            sum+= str_2_int(decimal_2_octal(sb->st_size, SIZE_LENGTH));
            write_ascii(sb->st_size, SIZE_LENGTH, arcfd);
        }
        sum+= str_2_int(decimal_2_octal(sb->st_mtime, MTIME_LENGTH));
        write_ascii(sb->st_mtime, MTIME_LENGTH, arcfd);
        lseek(*arcfd, CHKSUM_LENGTH, SEEK_CUR);
        /*need chksum*/
        if(S_ISREG(sb->st_mode))
        {
            write(*arcfd, REG_TYPEFLAG, TYPEFLAG_LENGTH);
            sum+=REG_TYPEFLAG[0];
            lseek(*arcfd, LINKNAME_LENGTH, SEEK_CUR);
        }
        else if(S_ISDIR(sb->st_mode))
        { 
            write(*arcfd, DIR_TYPEFLAG, TYPEFLAG_LENGTH);
            sum+=DIR_TYPEFLAG[0];
            lseek(*arcfd, LINKNAME_LENGTH, SEEK_CUR);
        }
        else if(S_ISLNK(sb->st_mode))
        {
            int num;
            char *linkn = (char *)malloc(sizeof(char) * LINKNAME_LENGTH);
            write(*arcfd, SYM_TYPEFLAG, TYPEFLAG_LENGTH);
            sum+=SYM_TYPEFLAG[0];
            num = readlink(name, linkn, LINKNAME_LENGTH);
            /*check if num is -1*/
            linkn = (char *)realloc(linkn, sizeof(char)*num);
            for(i = 0; i < num; i++)
            {
                sum+=linkn[i];
            }
            write(*arcfd, linkn, num);
            lseek(*arcfd, LINKNAME_LENGTH-num, SEEK_CUR);
            free(linkn);
        }
        else{
            close(*arcfd);
            perror("Invalid file type");
            exit(-1);
        }
        write(*arcfd, USTAR, MAGIC_LENGTH);
        sum+=MAGIC_VALUE;
        write(*arcfd, "00", VERSION_LENGTH);
        sum+=VERSION_VALUE;
        pwd = getpwuid(sb->st_uid);
        pgrp = getgrgid(sb->st_gid);
        len2 = strlen(pwd->pw_name);
        for(i=0; i < len2; i++)
        {
            sum+=pwd->pw_name[i];
        }
        write_word(pwd->pw_name, UNAME_LENGTH, arcfd);
        lseek(*arcfd, -1, SEEK_CUR);
        write(*arcfd, "\0", 1);
        len2 = strlen(pgrp->gr_name);
        for(i=0; i < len2; i++)
        {
            sum+=pgrp->gr_name[i];
        }
        write_word(pgrp->gr_name, GNAME_LENGTH, arcfd);
        lseek(*arcfd, -1, SEEK_CUR);
        write(*arcfd, "\0", 1);
        /*devmajor and devminor*/
        lseek(*arcfd, DEVMAJOR_LENGTH, SEEK_CUR);
        lseek(*arcfd, DEVMINOR_LENGTH, SEEK_CUR);
        /*prefix*/
        if(len > NAME_LENGTH)
        {
            len = len - NAME_LENGTH;
            write(*arcfd, name+NAME_LENGTH, len);
        }
        lseek(*arcfd, CHKSUM_OFFSET+(blockcount*BLOCK_SIZE), SEEK_SET);
        sum+=CHK_VALUE;
        write_ascii(sum, CHKSUM_LENGTH, arcfd);
        lseek(*arcfd, 
            PREFIX_OFFSET+PREFIX_LENGTH+EOF_OFFSET+(blockcount*BLOCK_SIZE), 
            SEEK_SET);
        write(*arcfd, "\0", 1);
        blockcount++;
    //}
    //printf("newname: %s\n", newname);
}

void write_contents(int *arcfd, char *rfile)
{
    //printf("newname: %s\n", newname);
    int num, fd;
    unsigned char *buff = (unsigned char *)malloc(
            sizeof(unsigned char) * BLOCK_SIZE);
    fd = open(rfile, O_RDONLY);
    while((num = read(fd, buff, BLOCK_SIZE)) == BLOCK_SIZE)
    {
        write(*arcfd, buff, num);
        blockcount++;
    }
    write(*arcfd, buff, num);
    if(num != 0)
    {
        num = BLOCK_SIZE-num;
        lseek(*arcfd, num-1, SEEK_CUR);
        write(*arcfd, "\0", 1);
        blockcount++;
    }
    free(buff);
}

void print_usage_and_exit()
{
    fprintf(stderr, "usage: mytar [ctxvS]f tarfile [ path [ ... ] ] \n");
    exit(-1);
}

void isinclude(int *c, int *t, int *x, int *v, int *s, int *f, char *code)
{
    int sum = 0;
    if(strchr(code ,'f') != NULL)
       *f = 1;
    else
        print_usage_and_exit();
    if(strchr(code ,'c') != NULL)
        *c = 1;
    if(strchr(code ,'t') != NULL)
        *t = 1;
    if(strchr(code ,'x') != NULL)
        *x = 1;
    if(strchr(code ,'v') != NULL)
        *v = 1;
    if(strchr(code ,'S') != NULL)
        *s = 1;
    sum = *c + *t + *x;
    if(sum > 1)
        print_usage_and_exit();
    if(sum == 0)
        print_usage_and_exit();
}

void write_ascii(int num, int length, int *arcfd)
{
    char *ascii = decimal_2_octal(num, length);
    write(*arcfd, ascii, length);
    free(ascii);
}

void write_word(char *word, int length, int *arcfd)
{
    int len;
    len = strlen(word);
    if(len < length){
        write(*arcfd, word, len);
        lseek(*arcfd, length-len, SEEK_CUR);
    }
    else
        write(*arcfd, word, length);
}

char *decimal_2_octal(int num, int length)
{
    int numbin = 0, remain, i;
    char *binstr = (char *)malloc(sizeof(char) * length);
    char *rbinstr = (char *)malloc(sizeof(char) * length);
    while(num > 0)
    {
        remain = num % OCTAL;
        num = num/OCTAL;
        binstr[numbin] = remain + 48;
        numbin++;
        if(length == numbin)
        {
            break;
        }
    }
    if(numbin < length)
    {
        for(i = numbin; i < length-1; i++)
        {
            binstr[i] = '0';
            numbin++;
        }
    }
    for(i = 0; i < numbin; i++)
    {
        rbinstr[i] = binstr[numbin-1-i];
    }
    rbinstr[numbin] = '\0';
    free(binstr);
    return rbinstr;
}

void end_archive(int *arcfd, int block)
{
    lseek(*arcfd, block-1, SEEK_CUR);
    write(*arcfd, "\0", 1);
    lseek(*arcfd, block-1, SEEK_CUR);
    write(*arcfd, "\0", 1);
}

int str_2_int(char *word)
{
    int len = strlen(word), i, total = 0;
    for(i = 0; i < len; i++)
    {
        total += word[i];
    }
    return total;
}

void printpermissions(char readbuf[]){
    char mode[9];
    int ind = 0;
    long int num; 
    int uid = 0;
    int gid = 0;
    int sticky = 0;
    
    char type = readbuf[156];
    if( type == '5'){
        printf("d");
    }
    if(type == '2'){
        printf("l");
    }
    if(type != '5' && type != '2'){
        printf("-");
    }
    while(ind != 8){
        mode[ind] = readbuf[100 + ind];
        ind++;
    }
    mode[ind] = '\0';
    num = strtol(mode, NULL, 8);
    
    if(num & S_ISUID){
        uid = 1;
    }
    if(num & S_ISGID){
        gid = 1;
    }
    if(num & S_ISVTX){
        sticky = 1;
    }
    printf( (num & S_IRUSR) ? "r" : "-");
    printf( (num & S_IWUSR) ? "w" : "-");
    if(uid == 1){
        printf("s");
    }else{
        printf( (num & S_IXUSR) ? "x" : "-");
    }
    printf( (num & S_IRGRP) ? "r" : "-");
    printf( (num & S_IWGRP) ? "w" : "-");
    if(gid == 1){
        printf("s");
    }else{
        printf( (num & S_IXGRP) ? "x" : "-");
    }
    printf( (num & S_IROTH) ? "r" : "-");
    printf( (num & S_IWOTH) ? "w" : "-");
    if(sticky == 1){
        printf("t");
    }else{
        printf( (num & S_IXOTH) ? "x" : "-");
    }
    printf(" ");
}
void printowner(char readbuf[]){
    char uid[32];
    char gid[32];
    int ind = 0;
    while(ind != 32){
        gid[ind] = readbuf[297 + ind];
        uid[ind] = readbuf[265 + ind];
        ind++;
    }
    printf("%s/",uid);
    printf("%s ",gid);
}
void printsize(char readbuf[]){
    char size[12];
    
    //struct group *gidst;
    int ind = 0;
    while(ind != 12){
        size[ind] = readbuf[124 + ind];
        ind++;
    }
    size[ind] = '\0';
    
    long int number = strtol(size, NULL, 8);
    int count10= 0;
    int multof10 = number;
    while(multof10 != 0){
        multof10 /=10;
        ++count10;
    }
    if(count10 == 0){
        count10 = 1;
    }
    int realcount10 = 8 - count10;
    
    while(realcount10 != 0){
        printf(" ");
        
        realcount10--;
    }
    
    printf("%ld ",number);
}
void printtime(char readbuf[]){
    char mtime[12];
    int ind = 0;
    while(readbuf[136 + ind] != '\0'){ 
        mtime[ind] = readbuf[136 + ind];
        ind++;
    }
    mtime[ind] = '\0';
 
    long int number = strtol(mtime, NULL, 8);

    time_t date;
    date = (time_t)number;
    struct tm time;
    char buf[80];

    time = *localtime(&date);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &time);
    printf("%s ", buf);

}
void printname(char readbuf[]){
    int ind = 0;
    char word[100];
    char prefix[155];
    if( readbuf[345] != '\0'){//if prefix is needed
        //printf("YEAAAAAAA");
        while(readbuf[345 + ind] != '\0' && ind < 155){
            prefix[ind] = readbuf[345 + ind];
            ind++;
        }
        prefix[ind] = '\0';
        printf("%s/", prefix);
    }
    ind = 0;
    while( readbuf[ind] != '\0' && ind < 100){
        word[ind] = readbuf[ind];
        //printf("%c", readbuf[ind]);
        ind++;
    }
    word[ind]='\0';
    printf("%s", word);
    printf("\n");
}