#include "pixutils.h"
//#include "bmp.h"

int main(int argc, char *argv[]){
  char *inputfile=0,*outputfile=0;
  float degrees=0, grayFlag=0;
  int isBMP = 0, isSort= 0;
  
  //write the parser yourself or use a package like https://github.com/skeeto/optparse
  
  //check for flags -i -o -r -g - can be in any order
  //-i is followed by the input png
  //-o is followed the output png
  //-r is followd by the rotation angle in degrees (float) <optional for user>
  //-g is whether th png should be grayed <optional for user>
  int i = 1;
  while(i < argc) {
	  	if(strcmp(argv[i],"-i")==0){
			inputfile=argv[i+1];
			i+=2;
		} else if(strcmp(argv[i],"-o")==0){
			outputfile=argv[i+1];
			i+=2;
		} else if(strcmp(argv[i], "-g")==0) {
			grayFlag=1;
			i++;
		} else if(strcmp(argv[i], "-r")==0) {
			degrees=atof(argv[i+1]);
			i+=2;
		} else if(strcmp(argv[i], "-b")==0) {
		  isBMP = 1;
		  i++;
    } else if(strcmp(argv[i], "-s")==0) {
      isSort = 1;
      i++;
    } else {
			fprintf(stderr, "Error parsing command line.");
			exit(1);
		}
  }

  if(!inputfile && !outputfile) {
    exit(1);
  }
  pixMap *p=pixMap_init_filename(inputfile);
  if(isSort) {
    pixMap_sort(p);
  }
  if(degrees)pixMap_rotate(p,degrees);
  if(grayFlag)pixMap_gray(p);
  if(isBMP) {
    //convert and write to BMP file
    pixMap_write_bmp16(p, outputfile);
  } else {
    pixMap_write(p,outputfile);
  }
  pixMap_destroy(p);
  
  return 0;
}

