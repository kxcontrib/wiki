#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<math.h>
#include"k.h"

// obtain k.h from http://kx.com/q/c/c/k.h
// linux:
//  compile with gcc -m64 -DKXVER=3 csv.c c.o
//  obtain c.o from http://kx.com/q/l64/c.o for linux
// windows:
//  start the x86 or 64 bit version of build environment, then: cl -DKXVER=3 /MD csv.c c.obj ws2_32.lib
//  obtain c.obj from http://kx.com/q/w32/ or w64/

// times are typically stored in GMT on a server, default is not to apply our local timezone to display.
#define USEGMT 1

// TODO: handle infinities and nested types.

int main(int argc,char*argv[]){
  K flip,result,columnNames,columnData;
  struct tm*(*lgmtime)(const time_t*timep)=USEGMT?gmtime:localtime;
  J row,col,nCols,nRows;
  I handle=khpu("localhost",12001,"myusername:mypassword");
  if(handle<0)
    printf("Cannot connect\n"),exit(1);
  else if(!handle)
    printf("Wrong credentials\n"),exit(1);
  // create a table of various types to test decode
  result=k(handle,"([]B:01010b;X:0x0001020304;H:0 1 2 3 4h;I:0 1 2 3 4i;J:0 1 2 3 4j;\
                      E:0 1 2 3 4e;F:0 1 2 3 4.;C:\"abcde\";S:`a`b`c`d`e;P:.z.p+til 5;\
                      M:2001.01m+til 5;D:2001.01.01+til 5;Z:.z.z+til 5;N:.z.n+til 5;\
                      U:10:00:00+til 5;T:10:00:00.000+til 5)",
           (K)0); // last arg to variadic function k(I,...) MUST be 0, since this terminates argument list.
  if(!result)
    printf("Network Error\n"),perror("Network"),exit(1);
  kclose(handle); // all data received, so can close socket here
  if(result->t==-128)
    printf("Server Error: %s\n",result->s),r0(result),exit(1);
  if(result->t!=99&&result->t!=98) // accept table or dict only
    printf("type %d\n",result->t),r0(result),exit(1);
  flip=ktd(result); // if keyed table, unkey it. ktd decrements ref count of arg.
  // table (flip) is column names!list of columns (data)
  columnNames=kK(flip->k)[0];
  columnData=kK(flip->k)[1];
  nCols=columnNames->n;
  nRows=kK(columnData)[0]->n;
  for(row=0;row<nRows;row++){
    if(0==row){
      for(col=0;col<nCols;col++){  
        if(col>0)printf(",");
          printf("%s",kS(columnNames)[col]);
      }
      printf("\n");
    }
    for(col=0;col<nCols;col++){
      K obj=kK(columnData)[col];
      if(col>0)
        printf(",");
      switch(obj->t){
        case(0):{ // handle a list of char vectors
          K x=kK(obj)[row];
          if(10==x->t){
            int i;
            for(i=0;i<xn;i++)
              printf("%c",kC(x)[i]);
          }
          else
            printf("type %d not supported by this client",obj->t);
        }break;
        case(KB):{printf("%d",kG(obj)[row]);}break;
        case(KG):{printf("%d",kG(obj)[row]);}break;
        case(KH):{printf("%d",kH(obj)[row]);}break;
        case(KI):{I i=kI(obj)[row];if(ni!=i)printf("%d",i);}break;
        case(KJ):{J j=kJ(obj)[row];if(nj!=j)printf("%lld",j);}break;
        case(KE):{E e=kE(obj)[row];if(!isnan(e))printf("%f",e);}break;
        case(KF):{F f=kF(obj)[row];if(!isnan(f))printf("%f",f);}break;
        case(KC):{printf("%c",kC(obj)[row]);}break;
        case(KS):{printf("%s",kS(obj)[row]);}break;
        case(KP):{
          J j=kJ(obj)[row];
          if(nj!=j){ // checks if value is not 0Nj
            time_t time=j/8.64e13+10957*8.64e4;
            struct tm*timinfo=lgmtime(&time);
            printf("%04d.%02d.%02dD%02d:%02d:%02d.%09lld",
                   timinfo->tm_year+1900,
                   timinfo->tm_mon+1,
                   timinfo->tm_mday,
                   timinfo->tm_hour,
                   timinfo->tm_min,
                   timinfo->tm_sec,
                   j%1000000000);
          }
        }break;
        case(KM):{
          I i=kI(obj)[row],year=i/12+2000,month=i%12+1;
          if(ni!=i) // checks if value is not 0Ni
            printf("%04d.%02d",year,month);
        }break;
        case(KD):{
          I i=kI(obj)[row];
          if(ni!=i){
            time_t time=(i+10957)*8.64e4;
            struct tm*timinfo=lgmtime(&time);
            printf("%04d.%02d.%02d",
                   timinfo->tm_year+1900,
                   timinfo->tm_mon+1,
                   timinfo->tm_mday);
          }
        }break;
        case(KZ):{
          F f=kF(obj)[row];
          if(!isnan(f)){
            time_t time=(f+10957)*8.64e4;
            struct tm*timinfo=lgmtime(&time);
            printf("%04d.%02d.%02dT%02d:%02d:%02d.%03lld",
                   timinfo->tm_year+1900,
                   timinfo->tm_mon+1,
                   timinfo->tm_mday,
                   timinfo->tm_hour,
                   timinfo->tm_min,
                   timinfo->tm_sec,
                   (long long)(round(f*8.64e7))%1000);
          }
        }break; 
        case(KN):{
          J j=kJ(obj)[row];
          if(ni!=j){
            time_t time=j/1000000000;
            struct tm*timinfo=localtime(&time);
            printf("%dD%02d:%02d:%02d.%09lld",
                   timinfo->tm_yday,
                   timinfo->tm_hour-1,
                   timinfo->tm_min,
                   timinfo->tm_sec,
                   j%1000000000);
          }
        }break;
        case(KU):{
          I i=kI(obj)[row];
          if(ni!=i){ 
            time_t time=i*60;
            struct tm*timinfo=localtime(&time);
            printf("%02d:%02d",timinfo->tm_hour-1,timinfo->tm_min);
          }
        }break;
        case(KV):{
          I i=kI(obj)[row];
          if(ni!=i){
            time_t time=kI(obj)[row];
            struct tm*timinfo=localtime(&time);
            printf("%02d:%02d:%02d",timinfo->tm_hour-1,timinfo->tm_min,timinfo->tm_sec);
          }
        }break;
        case(KT):{
          I i=kI(obj)[row];
          if(ni!=i){
            time_t time=i/1000;
            struct tm*timinfo=localtime(&time);
            printf("%02d:%02d:%02d.%03d",
                   timinfo->tm_hour-1,
                   timinfo->tm_min,
                   timinfo->tm_sec,
                   i%1000);
          }
        }break;
        default:{
          printf("type %d not supported by this client",obj->t);
        }break;
      }
    }
    printf("\n");
  }
  r0(flip);
  return 0;
}
