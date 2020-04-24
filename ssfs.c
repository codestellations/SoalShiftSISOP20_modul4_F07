// gcc -Wall `pkg-config fuse --cflags` [file.c] -o [output] `pkg-config fuse --libs`

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
#include <sys/time.h>

#define KEY 10

static const char *dirpath = "/home/el/tiga";
char charlist[1024] = "9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO";
char *limit;

void enc(char* kata){
  int len;

  if(!strcmp(kata, ".") || !strcmp(kata, ".."))
  return;

  limit = strrchr(kata, '.');
  if(limit != NULL){
    len = limit-kata;
  }
  else len = strlen(kata);

  for(int i=0; i<len; i++){
    for(int j=0; j<87; j++){
      if(kata[i] == charlist[j]){
        kata[i] = charlist[(j+87-KEY)%87];
        break;
      }
    }
  }
}

void dec(char* kata){
  int len;
  if(!strcmp(kata, ".") || !strcmp(kata, ".."))
  return;

  limit = strrchr(kata, '.');
  if(limit != NULL){
    len = limit-kata;
  }
  else len = strlen(kata);

  for(int i=0; i<len; i++){
    for(int j=0; j<87; j++){
      if(kata[i] == charlist[j]){
        kata[i] = charlist[(j+KEY)%87];
        break;
      }
    }
  }
}

void logSys(char* command, char* argv1, char* argv2, int lev){
  char message[1000], str_time[100];
  char level[2][10] = {"INFO", "WARNING"};
  char log[100] = "/home/el/fs.log";
  time_t now = time(NULL);
  // [LEVEL]::[yy][mm][dd]-[HH]:[MM]:[SS]::[CMD]::[DESC...]

  strftime(str_time, 50, "[%Y][%m][%D]-[%H]:[%M]:[%S]", localtime(&now));

  if(argv2 == NULL){
    if(argv1 == NULL){
      sprintf(message, "[%s]::%s::[%s]\n", level[lev], str_time, command);
    }
    else
    sprintf(message, "[%s]::%s::[%s]::%s\n", level[lev], str_time, command, argv1);
  }
  else
  sprintf(message, "[%s]::%s::[%s]::%s::%s\n", level[lev], str_time, command, argv1, argv2);

  FILE *fptr = fopen(log, "a+");
  fprintf(fptr, "%s", message);
  fclose(fptr);

  return;
}

static  int  xmp_getattr(const char *path, struct stat *stbuf) {
  int res;
  char fpath[1000];
  char name[1000];

  sprintf(name, "%s", path);
  if(strstr(path, "encv1_")!=NULL)
  enc(name);
  sprintf(fpath,"%s%s",dirpath,name);

  printf("GETATR fpath %s path %s name %s\n", fpath, path, name);

  res = lstat(fpath, stbuf);

  if (res == -1)
  return -errno;

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
    if(strstr(path, "encv1_")!=NULL)
    enc(name);
    sprintf(fpath, "%s%s", dirpath, name);
    // printf("READIR temp %s path %s name %s\n", fpath, path, name);

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
    sprintf(fullpathname, "%s%s", fpath, de->d_name);

    char temp[1000];
    strcpy(temp, de->d_name);
    if(strstr(path, "encv1_")!=NULL)
    dec(temp);
    // printf("OPENDIR temp %s name %s\n", temp, de->d_name);

    res = (filler(buf, temp, &st, 0));

    if(res!=0) break;
  }
  closedir(dp);
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

    if(strstr(path, "encv1_")!=NULL)
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

  logSys("READ", name, NULL, 0);

  close(fd);
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
    if(strstr(path, "encv1_")!=NULL)
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
    if(strstr(path, "encv1_")!=NULL)
    enc(name);
    sprintf(fpath, "%s%s", dirpath, name);

    printf("OPEN temp %s path %s name %s\n", fpath, path, name);

  }

  res = open(fpath, fi->flags);

  logSys("OPEN", name, NULL, 0);

  close(res);
  return 0;

}

static int xmp_mkdir(const char *path, mode_t mode){
  int res;
  char fpath[1000];
  char name[1000];

  if(strcmp(path,"/") == 0) {
    path=dirpath;
    sprintf(fpath,"%s",dirpath);
  }

  else {
    sprintf(name, "%s", path);
    // enc(name);
    sprintf(fpath, "%s%s", dirpath, name);

    printf("MKDIR temp %s path %s name %s\n", fpath, path, name);
  }

  res = mkdir(fpath, 0700);

  if(res == -1) return -errno;

  logSys("MKDIR", name, NULL, 0);

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
    enc(name);
    sprintf(fpath, "%s%s", dirpath, name);

    printf("RMDIR temp %s path %s name %s\n", fpath, path, name);

  }

  res = rmdir(fpath);
  if(res == -1) return -errno;

  logSys("RMDIR", name, NULL, 1);

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
    enc(name);
    sprintf(fpath, "%s%s", dirpath, name);

    sprintf(toname, "%s", to);
    enc(toname);
    sprintf(topath, "%s%s", dirpath, toname);

    printf("RENAME temp %s path %s name %s\n", fpath, path, name);

  }

  res = rename(fpath, topath);
  if(res == -1) return -errno;

  logSys("RENAME", name, toname, 0);

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
    enc(name);
    sprintf(fpath, "%s%s", dirpath, name);

    printf("UNLINK temp %s path %s name %s\n", fpath, path, name);
  }

  res = unlink(fpath);
  if(res == -1) return -errno;

  logSys("UNLINK", name, NULL, 1);

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

};

int  main(int  argc, char *argv[]) {

  umask(0);
  return fuse_main(argc, argv, &xmp_oper, NULL);

}
