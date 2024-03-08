#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<fcntl.h> 
#include<errno.h> 
#include<unistd.h>
#include<stdint.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <pwd.h>
#include <sys/types.h>
#include <math.h>
#include <grp.h>
#include <arpa/inet.h>
#include <time.h>


#define SIZE 2048
void isinclude(int *c, int *t, int *x, int *v, int *s, int *f, char *code);
void print_usage_and_exit();
void printpermissions(char readbuf[]);
void printowner(char readbuf[]);
void printsize(char readbuf[]);
void printtime(char readbuf[]);
void printname(char readbuf[]);

void print_usage_and_exit(){
    fprintf(stderr, "usage: mytar [ctxvS]f tarfile [ path [ ... ] ] \n");
    exit(-1);
}
//ctxvS]f
void isinclude(int *c, int *t, int *x, int *v, int *s, int *f, char *code){

    if(strchr(code ,'f') != NULL){
       *f = 1;
    }else{
        print_usage_and_exit();
    }
    if(strchr(code ,'c') != NULL){
        *c = 1;
    }

    if(strchr(code ,'t') != NULL){
        *t = 1;
    }

    if(strchr(code ,'x') != NULL){
        *x = 1;
    }

    if(strchr(code ,'v') != NULL){
        *v = 1;
    }

    if(strchr(code ,'S') != NULL){
        *s = 1;
    }
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
        printf("YEAAAAAAA");
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
int main(int argc, char *argv[]){
    
    int c = 0; 
    int t = 0;
    int x= 0; 
    int v = 0; 
    int s =0; 
    int f= 0; 
    
    isinclude(&c, &t, &x, &v, &s, &f, argv[1]);
    // if(argc > 3){
    //     printf("%s", argv[3]);
    // }
    

    if(t != 0){
        int in = open(argv[2], O_RDONLY);
        int check;
        char readbuf[512];
        int ind;
        char* ustar = "ustar";
        while((check = read(in, readbuf, 512) ) > 0 ){
            //printf("\n%c", readbuf[0]);
            ind = 0;
            char checku[6];
            while(ind != 5){
                checku[ind] = readbuf[257 + ind];
                ind++;
            }//checks puts the ustar off set into a buffer
            checku[ind]= '\0';
            if(strcmp(checku, ustar) == 0){
                if(argc > 3){
                    char* path = argv[3];
                    // char *last = path[strlen(path) - 1];
                    // while( *last != '/'){
                    //     *last = '\0';
                    //     last--;
                    // }
                    // int back = 1;
                    // while(path[strlen(path) - back] != '/'){
                    //     path[strlen(path) - back] = '\0';
                    //     back++;
                    // }
                    ind = 0;
                    char name[100];
                    
                    while(readbuf[ind] != '\0'){
                        name[ind] = readbuf[ind];
                        //printf("%c", readbuf[ind]);
                        ind++;
                    }
                    name[ind]= '\0';
                    // printf("%s\n", name);
                     //printf("%s\n", path);
                    char *ptr = strstr(name, path);
                    if( ptr == NULL){
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
    
     return 0; 
}
