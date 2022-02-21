#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "../genVibSeq.h"

#define SAMP_LEN 40    //cc
#define SIN_TABLE_WEIGHT 128

int main(int argc, char *argv[]){
  FILE *fd;
  int samps[SAMP_LEN];

  if( argc != 3 ){
    fprintf(stderr," usage is: %s mode outfile.csv\n", argv[0]);
    fprintf(stderr,"    mode = 1 for normal, 2 for abnormal\n");
    exit(1);
  }
  else{
    ais_init_sin_table(SIN_TABLE_WEIGHT);
  }

  fd = fopen(argv[2], "w");
  // Print out the column headers
  // Pandas wants these to be there!
  for(int i = 0; i < SAMP_LEN; i++){
    char tmpbuf[100];
    sprintf(tmpbuf, "samp%d", i);
    fprintf(fd, "%s", tmpbuf);
    if( i == SAMP_LEN - 1) fprintf(fd, "\n");
    else fprintf(fd, ",");
  }

  // Compute some vectors to train on
  for(int i = 0; i < 500; i++){
    get_sample_vector(atoi(argv[1]), SAMP_LEN, samps);
    for(int j = 0; j < SAMP_LEN; j++){
      fprintf(fd, "%d", samps[j]);
      if( j == SAMP_LEN - 1) fprintf(fd, "\n");
      else fprintf(fd, ",");
    }
  }
  fclose(fd);
}
