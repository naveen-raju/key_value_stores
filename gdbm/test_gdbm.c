#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <gdbm.h>
#include <pthread.h>
#include <string.h>


typedef struct dummy_data {
   unsigned long x, y, z;
} DUMMY_DATA, *PDUMMY_DATA;

#define BLOCK_SIZE 4096
int main(int argc, char *argv[])
{
   GDBM_FILE        dbmap = NULL;
   char             *str_prefix = "gdbm_key_prefix_";
   datum            key;
   datum            value;
   char             strkey[24];
   unsigned long    i;
   int              rc = 0;
   PDUMMY_DATA      datap;
   
   if (argc < 3) {
      fprintf(stderr, "Usage : <prog> <dbpath> <datasize>\n");
      exit(1);
   }

   dbmap = gdbm_open(argv[1], BLOCK_SIZE, GDBM_WRCREAT, 0666, NULL);
   if (!dbmap) {
      fprintf(stderr, "Unable to open the gdbm file \n");
      exit(1);
   }

   datap = (PDUMMY_DATA) malloc(sizeof(DUMMY_DATA));
   if (!datap) {
      fprintf(stderr, "Malloc error %s\n", strerror(errno));
      exit(1);
   }

   unsigned long datasize = strtoul(argv[2], NULL, 10);
   // push data 
   for (i = 0; i < datasize; i++) {
      sprintf(strkey, "%s%lu", str_prefix, i);
      key.dptr = strkey;
      key.dsize = strlen(strkey);
      datap->x = i;
      datap->y = i+1;
      datap->z = i+2;
      value.dptr = (char *) datap;
      value.dsize = sizeof(DUMMY_DATA);
      gdbm_store(dbmap, key, value, GDBM_INSERT);
   }

   fprintf(stdout, "Starting read of the database sequentially\n");

   key = gdbm_firstkey(dbmap);
   while (key.dptr) {
      value = gdbm_fetch(dbmap, key);
      if (!value.dptr) {
         fprintf(stderr, "Value is not found for key %s\n", key.dptr);
      } else {
         datap = (PDUMMY_DATA) value.dptr;
      /*   fprintf(stdout, "Data read for dbm : x=%lu y=%lu z=%lu\n", 
               datap->x, datap->y, datap->z);*/

      }
      key = gdbm_nextkey(dbmap, key);
   }

   fprintf(stdout, "End of the database reached\n");
   gdbm_close(dbmap);

   return 0;
}
