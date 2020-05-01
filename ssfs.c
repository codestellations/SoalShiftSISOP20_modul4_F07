// gcc -Wall `pkg-config fuse --cflags` ssfs.c -o ssfs `pkg-config fuse --libs`

#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>

#define KEY 10

static const char *dirpath = "/home/el/tiga";
char charlist[1024] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
char *limit, *start;
int command = 0;

void enc(char* kata){
  int len, sta;

  if(!strcmp(kata, ".") || !strcmp(kata, ".."))
  return;

  start = strchr(kata, '_');
  sta = 0;
  if(strstr(kata, "encv1_")!=NULL){
    sta = start-kata;
    for(int i=sta; i<strlen(kata); i++){
      sta = strlen(kata);
      if(kata[i]== '/'){
        sta = i+1;
        break;
      }
    }
  }

  limit = strrchr(kata, '.');
  if(limit != NULL){
    len = limit-kata;
  }
  else len = strlen(kata);

  for(int i=sta; i<len; i++){
    for(int j=0; j<87; j++){
      if(kata[i] == charlist[j]){
        kata[i] = charlist[(j+87-KEY)%87];
        break;
      }
    }
  }

  printf("ENC %s\n", kata);
}

void dec(char* kata){
  int len, sta;
  if(!strcmp(kata, ".") || !strcmp(kata, ".."))
  return;

  start = strchr(kata, '_');
  sta = 0;
  if(strstr(kata, "encv1_")!=NULL){
    sta = start-kata;
    for(int i=sta; i<strlen(kata); i++){
      sta = strlen(kata);
      if(kata[i]== '/'){
        sta = i+1;
        break;
      }
    }

  }

  limit = strrchr(kata, '.');
  if(limit != NULL){
    len = limit-kata;
  }
  else len = strlen(kata);

  for(int i=sta; i<len; i++){
    for(int j=0; j<87; j++){
      if(kata[i] == charlist[j]){
        kata[i] = charlist[(j+KEY)%87];
        break;
      }
    }
  }

  printf("DEC %s\n", kata);
}

/*buat encv2_*/
void enc2(char* kata)
{
    int len = 0;
    char dest[1024];
    void * buff = (char *)malloc(1024);
    FILE *fp = fopen(kata, "rb");
    sprintf(dest,"%s.%03d", dest, len);
    while(1)
    {
        size_t size = fread(buff, 1, 1024, fp);
        if(size == 0) break;
        FILE *fp2 = fopen(dest, "w");
        fwrite(buff, 1, size, fp2);
	fclose(fp2);
        len++;
        sprintf(dest, "%s.%03d", dest, len);
    }
    free(buff);
    fclose(fp);
    remove(dest);
}

void logSys(char* command, char* argv1, char* argv2, int lev){
  char message[1000], str_time[100];
  char level[2][10] = {"INFO", "WARNING"};
  char log[100] = "/home/el/fs.log";
  time_t now = time(NULL);
  // [LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC...]

  strftime(str_time, 50, "%y%m%d-%H:%M:%S", localtime(&now));

  if(argv2 == NULL){
    if(argv1 == NULL){
      sprintf(message, "%s::%s::%s\n", level[lev], str_time, command);
    }
    else
    sprintf(message, "%s::%s::%s::%s\n", level[lev], str_time, command, argv1);
  }
  else
  sprintf(message, "%s::%s::%s::%s::%s\n", level[lev], str_time, command, argv1, argv2);

  FILE *fptr = fopen(log, "a+");
  fprintf(fptr, "%s", message);
  fclose(fptr);

  return;
}

void toDatabase(char* command, char* argv1, char* argv2, int lev){
  char message[1000], str_time[100];
  char level[2][10] = {"INFO", "WARNING"};
  char log[100] = "/home/el/database/log/encv1.log";
  time_t now = time(NULL);
  // [LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC...]

  strftime(str_time, 50, "%y%m%d-%H:%M:%S", localtime(&now));

  if(argv2 == NULL){
    if(argv1 == NULL){
      sprintf(message, "%s::%s::%s\n", level[lev], str_time, command);
    }
    else
    sprintf(message, "%s::%s::%s::%s\n", level[lev], str_time, command, argv1);
  }
  else
  sprintf(message, "%s::%s::%s::%s::%s\n", level[lev], str_time, command, argv1, argv2);

  FILE *fptr = fopen(log, "a+");
  fprintf(fptr, "%s", message);
  fclose(fptr);

  return;
}

static  int  xmp_getattr(const char *path, struct stat *stbuf) {
  int res;
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL && command==0)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);
  }

  res = lstat(fpath, stbuf);

  if (res == -1)
  return -errno;

  command = 0;

  return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
off_t offset, struct fuse_file_info *fi) {
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);
    printf("READIR temp %s dirpath %s name %s\n", fpath, dirpath, name);
  }

  int res = 0;

  DIR *dp;
  struct dirent *de;
  (void) offset;
  (void) fi;

  dp = opendir(fpath);
  if (dp == NULL)
  return -errno;

  while ((de = readdir(dp)) != NULL) {
    struct stat st;

    memset(&st, 0, sizeof(st));
    st.st_ino = de->d_ino;
    st.st_mode = de->d_type << 12;

    char fullpathname[1000];
    sprintf(fullpathname, "%s/%s", fpath, de->d_name);

    char temp[1000];
    strcpy(temp, de->d_name);
    if(strstr(fullpathname, "encv1_")!=NULL){
      dec(temp);
    }

    printf("OPENDIR fpath %s name %s\n", fullpathname, de->d_name);

    res = (filler(buf, temp, &st, 0));

    if(res!=0) break;
  }
  closedir(dp);

  command = 0;

  return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
struct fuse_file_info *fi) {
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s",dirpath,name);

    printf("READ temp %s path %s name %s\n", fpath, path, name);
  }

  int res = 0;
  int fd = 0;

  (void) fi;
  fd = open(fpath, O_RDONLY);

  if (fd == -1)
  return -errno;

  res = pread(fd, buf, size, offset);

  if (res == -1)
  res = -errno;

  // logSys("READ", name, NULL, 0);

  close(fd);

  command = 0;

  return res;
}

static int xmp_write(const char *path, const char *buf, size_t size,
off_t offset, struct fuse_file_info *fi) {
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);

    printf("WRITE temp %s path %s name %s\n", fpath, path, name);
  }

  int res,fd;

	(void) fi;
	fd=open(fpath, O_WRONLY);
	res=pwrite(fd, buf, size, offset);

  logSys("WRITE", name, NULL, 0);

	close(fd);

  command = 0;

	return res;
}

static int xmp_open(const char *path, struct fuse_file_info *fi) {
  int res;
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(dirpath, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);

    printf("OPEN temp %s path %s name %s\n", fpath, path, name);
  }

  res = open(fpath, fi->flags);

  logSys("OPEN", name, NULL, 0);

  close(res);

  command = 0;

  return 0;

}

static int xmp_mkdir(const char *path, mode_t mode){
  int res;
  char fpath[1000];
  char name[1000];
  char temp[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL){
      enc(name);
      toDatabase("MKDIR", name, NULL, 0);
    }

    sprintf(fpath, "%s%s", dirpath, name);

    printf("MKDIR temp %s path %s name %s\n", fpath, path, name);
  }

  res = mkdir(temp, 0700);

  if(res == -1) return -errno;

  logSys("MKDIR", name, NULL, 0);

  command = 1;

  return 0;
}

static int xmp_rmdir(const char *path){
  int res;
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);

    printf("RMDIR temp %s path %s name %s\n", fpath, path, name);

  }

  res = rmdir(fpath);
  if(res == -1) return -errno;

  logSys("RMDIR", name, NULL, 1);

  command = 0;

  return 0;
}

static int xmp_rename(const char *path, const char *to){
  int res;
  char fpath[1000];
  char name[1000];
  char toname[1000];
  char topath[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);
    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL){
      enc(name);
      toDatabase("MKDIR", name, NULL, 0);
    }
    sprintf(fpath, "%s%s", dirpath, name);

    sprintf(toname, "%s", to);
    sprintf(topath, "%s%s", dirpath, toname);

    printf("RENAME temp %s path %s name %s\n", fpath, path, name);
  }

  res = rename(fpath, topath);
  if(res == -1) return -errno;

  logSys("RENAME", name, toname, 0);

  command = 0;

  return 0;
}

static int xmp_unlink(const char *path){
  int res;
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);

    printf("UNLINK temp %s path %s name %s\n", fpath, path, name);
  }

  res = unlink(fpath);
  if(res == -1) return -errno;

  logSys("UNLINK", name, NULL, 1);

  command = 0;

  return 0;
}

static int xmp_create(const char* path, mode_t mode, struct fuse_file_info *fi){
  (void) fi;

  int res;
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);
    enc(name);
    sprintf(fpath, "%s%s", dirpath, name);

    printf("CREAT temp %s path %s name %s\n", fpath, path, name);

  }

  res = creat(fpath, mode);
  if(res == -1) return -errno;

  logSys("CREAT", name, NULL, 0);

  command = 0;

  return 0;
}

static int xmp_truncate(const char *path, off_t size){
  int res;

  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);

    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    enc(name);

    sprintf(fpath, "%s%s", dirpath, name);

    printf("TRUNCATE temp %s path %s name %s\n", fpath, path, name);
  }

  res = truncate(fpath, size);
  if(res == -1) return -errno;

  return 0;
}

static struct fuse_operations xmp_oper = {
  .getattr = xmp_getattr,
  .readdir = xmp_readdir,
  .read = xmp_read,
  .write = xmp_write,
  .open = xmp_open,
  .mkdir = xmp_mkdir,
  .rmdir = xmp_rmdir,
  .rename = xmp_rename,
  .unlink = xmp_unlink,
  .create = xmp_create,
  .truncate = xmp_truncate,
};

int  main(int  argc, char *argv[]) {

  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);

}
