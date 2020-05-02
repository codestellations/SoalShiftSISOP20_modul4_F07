# SoalShiftSISOP20_modul4_F07
Praktikum Modul 4 mata kuliah Sistem Operasi kelompok F07

- Erlinda Argyanti Nugraha  05111840000017
- Lii'zza Aisyah Putri Sulistio   05111840000073

---

# Soal 1  : Enkripsi Versi 1
apabila sebuah folder diberi nama dengan awalan "encv1_" maka direktori tersebut akan terenkripsi.
berikut adalah key yang digunakan adalah 
~~~ 
9(ku@AW1[Lmvgax6q`5Y2Ry?+sF!^HKQiBXCUSe&0M.b%rI'7d)o4~VfZ*{#:}ETt$3J-zpc]lnh8,GwP_ND|jO 
~~~
dan key yang dipakai adalah 10.

## Fungsi Enkripsi
karena yang dienkripsi adalah file dan folder di dalam folder `encv1_`, maka folder `encv1_` sendiri tidak ikut terenkripsi. untuk menghandle hal tersebut, digunakan code di bawah.
~~~
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
~~~

untuk file yang dienkripsi, extension file tidak ikut terenkripsi. untuk menghandle hal tersebut, digunakan code di bawah.
~~~
  limit = strrchr(kata, '.');
  if(limit != NULL){
    len = limit-kata;
  }
  else len = strlen(kata);
~~~

berikut adalah code untuk shift enkripsi caesar cipher sesuai dengan key yang digunakan.
~~~
  for(int i=sta; i<len; i++){
    for(int j=0; j<87; j++){
      if(kata[i] == charlist[j]){
        kata[i] = charlist[(j+KEY)%87];
        break;
      }
    }
  }
~~~
keterangan dari fungsi enkripsi :
- `kata` yaitu passing parameter, yaitu kata yang akan dienkripsi.
- `charlist` berisi string key untuk enkripsi yang digunakan.
- `kata[i] = charlist[(j+KEY)%87]` pengenkripsian menggunakan shift sebanyak `KEY` yaitu 10 dimodulo 87 karena 87 merupakan banyak huruf dalam key yang digunakan.


## Fungsi Dekripsi
fungsi yang digunakan kurang lebih sama dengan fungsi enkripsi, pada bagian pemisahan nama file dengan folder `encv1_` dan pemisahan ekstensi file.

berikut adalah code untuk shift dekripsi caesar cipher sesuai dengan key yang digunakan.
~~~
  for(int i=sta; i<len; i++){
    for(int j=0; j<87; j++){
      if(kata[i] == charlist[j]){
        kata[i] = charlist[(j+87-KEY)%87];
        break;
      }
    }
  }
~~~
keterangan dari fungsi dekripsi :
- `kata[i] = charlist[(j+87-KEY)%87]` pendekripsian menggunakan shift sebanyak 87, yaitu banyak huruf dalam key, dikurangi 10, yaitu key yang digunakan.

## Pemanggilan fungsi
fungsi *enkripsi* dipanggil dalam implementasi `readdir` apabila file terdapat di dalam direktori `encv1_`.
~~~
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
      enc(temp);
    }

    res = (filler(buf, temp, &st, 0));

    if(res!=0) break;
  }
~~~

sedangkan untuk fungsi *enkripsi* dipanggil pada semua implementasi fungsi-fungsi lain yang digunakan apabila file terdapat di dalam direktori `encv1_`. berikut adalah contoh pemanggilan fungsi *enkripsi* pada implementasi `read`.
~~~
    char temp[1000];
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL)
    dec(name);
~~~

## Pencatatan ke database
pencatatan direktori yang dibuat dengan nama `encv1_` atau pengubahan nama direktori menjadi atau dari `encv1_` akan ditulis di dalam log file di dalam direktori database/log. berikut adalah fungsi untuk menulis ke dalam file log.
~~~
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
~~~
fungsi `toDatabase()` ini isinya sama dengan sistem log nomor 4. yang membedakan hanyalah pemanggilan fungsi dan tempat file dibuat. file log ini dibuat di `/home/el/database/log/encv1.log`.

fungsi ini dipanggil pada implementasi `mkdir` dan `rename` seperti berikut. berikut adalah pemanggilan fungsi tersebut pada implementasi `mkdir`.
~~~
    sprintf(temp, "%s%s", dirpath, name);
    if(strstr(temp, "encv1_")!=NULL){
      dec(name);
      toDatabase("MKDIR", name, NULL, 0);
    }
~~~


# Soal 4 : Log System

## Fungsi logSys
berkas `fs.log` yang terletak pada `/home/[user]` digunakan untuk menyimpan daftar perintah system call yang telah dijalankan. berikut adalah code yang digunakan.
~~~
void logSys(char* command, char* argv1, char* argv2, int lev){
  char message[1000], str_time[100];
  char level[2][10] = {"INFO", "WARNING"};
  char log[100] = "/home/el/fs.log";
  time_t now = time(NULL);
  
  strftime(str_time, 50, "%y%m%d-%H:%M:%S", localtime(&now));
  
  .
  .
  .
}
~~~
berikut keterangannya :
- `char level[2][10] = {"INFO", "WARNING"}` variabel ini digunakan untuk membedakan level yang akan ditulis di dalam message. *level 0* menunjukkan messsage **INFO** dan *level 1* menunjukkan message **WARNING**.
- `char log[100] = "/home/el/fs.log"` tempat file *fs.log* akan dibuat.
- `strftime(str_time, 50, "%y%m%d-%H:%M:%S", localtime(&now))` membuat timestamp berdasarkan waktu sekarang.

sedangkan untuk pencatatannya, ada variasi dalam penulisan deskripsi message.
~~~
  if(argv2 == NULL){
    if(argv1 == NULL){
      sprintf(message, "%s::%s::%s\n", level[lev], str_time, command);
    }
    else
    sprintf(message, "%s::%s::%s::%s\n", level[lev], str_time, command, argv1);
  }
  else
  sprintf(message, "%s::%s::%s::%s::%s\n", level[lev], str_time, command, argv1, argv2);
~~~
berikut keterangannya :
- `sprintf(message, "%s::%s::%s\n", level[lev], str_time, command)` untuk perintah yang tidak memiliki argumen.
- `sprintf(message, "%s::%s::%s::%s\n", level[lev], str_time, command, argv1)` untuk perintah yang memiliki satu argumen.
- `sprintf(message, "%s::%s::%s::%s::%s\n", level[lev], str_time, command, argv1, argv2)` untuk perintah yang memiliki dua argumen.

kemudian message tersebut akan ditulis ke dalam file `fs.log` dengan code sebagai berikut.
~~~
  FILE *fptr = fopen(log, "a+");
  fprintf(fptr, "%s", message);
  fclose(fptr);
~~~
berikut keterangannya :
- `FILE *fptr = fopen(log, "a+")` perintah yang digunakan adalah *a+* yaitu menuliskan pada baris akhir file.
- `fprintf(fptr, "%s", message)` message yang ditulis dalam file `fs.log`.
- `fclose(fptr)` menutup file setelah ditulis.

## Pemanggilan fungsi
fungsi `logSys` dipanggil pada fungsi-fungsi sebagai berikut :
- `write`
- `open`
- `mkdir`
- `rmdir`
- `rename`
- `unlink`
- `creat`

berikut adalah pemanggilan fungsi dengan *level 0* atau INFO, yaitu pada implementasi `write`.
~~~
  logSys("WRITE", name, NULL, 0)
~~~

berikut adalah pemanggilan fungsi dengan *level 1* atau WARNING, yaitu pada implementasi `rmdir`.
~~~
  logSys("RMDIR", name, NULL, 1);
~~~

