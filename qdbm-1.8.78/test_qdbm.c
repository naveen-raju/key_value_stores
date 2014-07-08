#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <depot.h>
#include <pthread.h>
#include <string.h>


typedef struct dummy_data {
   unsigned long x, y, z;
} DUMMY_DATA, *PDUMMY_DATA;

#define BLOCK_SIZE 4096
int main(int argc, char *argv[])
{
   DEPOT            *dbmap = NULL;
   char             *str_prefix = "dbbm_key_prefix_";
   char             *key, keystr[24];
   unsigned long    i;
   int              rc = 0;
   PDUMMY_DATA      datap;
   
   if (argc < 3) {
      fprintf(stderr, "Usage : <prog> <dbpath> <datasize>\n");
      exit(1);
   }

   dbmap = dpopen(argv[1], DP_OWRITER|DP_OCREAT|DP_OSPARSE|DP_ONOLCK, -1);
   if (!dbmap) {
      fprintf(stderr, "Unable to open the dbbm file \n");
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
      asprintf(keystr, "%s%lu", str_prefix, i);
      datap->x = i;
      datap->y = i+1;
      datap->z = i+2;
      if (!dpput(dbmap, keystr, strlen(keystr), (char *) datap, sizeof(DUMMY_DATA), 
            DP_DOVER)) {
         fprintf(stderr, "Unable to insert to qdbm\n");
      };
   }

   if(!dpclose(dbmap)){
      fprintf(stderr, "dpclose: %s\n", dperrmsg(dpecode));
      return 1;
   }

   //read data 
   dbmap = dpopen(argv[1], DP_OREADER, -1); 
   if (!dbmap) {
      fprintf(stderr, "Unable to open the dbbm file \n");
      exit(1);
   }

   fprintf(stdout, "Starting read of the database sequentially\n");

   if(!dpiterinit(dbmap)){
      fprintf(stderr, "dpiterinit: %s\n", dperrmsg(dpecode));
   }
   
   /* scan with the iterator */
   while ((key = dpiternext(dbmap, NULL)) != NULL) {
      if (!(datap = (PDUMMY_DATA) dpget(dbmap, key, -1, 0, sizeof(DUMMY_DATA), NULL))) {
         fprintf(stderr, "Value is not found for key %s\n", key);
         fprintf(stderr, "dpget: %s\n", dperrmsg(dpecode));
         free(key);
         break;
      }
/*      fprintf(stdout, "Data read for dbm : x=%lu y=%lu z=%lu\n", 
            datap->x, datap->y, datap->z);*/
      free(datap);
   }
   
   fprintf(stdout, "End of the database reached\n");
   
   if(!dpclose(dbmap)){
      fprintf(stderr, "dpclose: %s\n", dperrmsg(dpecode));
      return 1;
   }

   return 0;
}
