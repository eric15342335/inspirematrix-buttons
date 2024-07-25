// generate the mel_mx.h file that is used to generate mel bins from FFT output
// This reads the 'mel.txt' (from python make_mel_filter.py) and converts it.
#include <stdio.h>
#include <math.h>

#define FFT 128
#define BITS 7 // log2 of FFT
#define FF2 (FFT/2+1)
#define MEL 20


/****** radix 2 bit-reversed-permutation test routine **/
void setup_bins(int b[FFT])
{
  int n;
  unsigned int x[FFT*2];
  unsigned int nforward, nreversed, count, temp;
  
  
  for(n=0; n<FFT; n++)                // fill the test array
  {
    x[n] = n;
  }


  for(n=1; n<FFT-1; n++)         // reorganise array x[n] in bit-reversed order
  {
    nreversed = n;
    count = BITS-1;

    for(nforward=n>>1; nforward; nforward>>=1)   
    {
       nreversed <<= 1;
       nreversed |= nforward & 1;   // give LSB of nforward to nreversed
       count--;
    }
    nreversed <<=count;      // compensation for missed iterations
    nreversed &= FFT-1;        // clear all bits more significant than N-1

    if(n<nreversed)          // swap condition
    {
       temp = x[n];
       x[n] = x[nreversed];
       x[nreversed] = temp;
    }
  }
  b[0]=0; for (n=0; n<FF2; n++) b[n+1]=x[FFT-n-1];
  //for (n=0; n<FF2; n++) printf("%i,",b[n]); printf("\n");
}


//-----------------------------------------------------------------------
void main()
{
  FILE *fp;
  int i,n,z;
  int b[FF2]; //map of correct order of FFT output bins
  int output[10000],opsize=0;
  double wts[MEL][FF2]; // FFT-to-mel matrix weights
  
  // read in the (MELxFF2) 'mel.txt' matrix generated by the Python script...
  fp=fopen("mel.txt","r");
  for (i=0; i<MEL; i++) {
    for (n=0; n<FF2-1; n++) z=fscanf(fp,"%lf,",&wts[i][n]);
    z=fscanf(fp,"%lf\n",&wts[i][FF2-1]);
  }
  fclose(fp);
  
  // generate map of output bins from the simple FFT function
  // (They are in a complicated order!)
  setup_bins(b);
  
  // calculate the (FFT bin,weight) combinations for each of the mel bins...
  opsize=0;
  for (i=0; i<MEL; i++) {
    for (n=0; n<FF2; n++) if (wts[i][n]!=0) {
       
       output[opsize]=b[n]; opsize++;
       output[opsize]=(int)(wts[i][n]*25080.0); opsize++;
    }
    output[opsize]=0; opsize++;
  }
  printf("output size=%i\n",opsize);
  
  
  // now generate the output file...
  fp=fopen("mel_mx.h","w");
  fprintf(fp,"const static unsigned int mel_mx[%i]={\n",opsize);
  for (i=0; i<opsize; i++)
    if (output[i]==0) fprintf(fp,"0,\n");
    else {fprintf(fp,"%i,%i,\n",output[i],output[i+1]); i++;}
  fprintf(fp,"};\n");
  fclose(fp);
  
}