#include "pixutils.h"
#include "lodepng.h"
#include "bmp.h"

//private methods
static pixMap *pixMap_init(); //allocate memory for pixMap object, set variables to zero, and return a pointer to the object
static void pixMap_reset();  //free the allocated memoray and set to zero but do not free memory for pixMap
static void pixMap_copy(pixMap *dest,pixMap *source); //copy one pixmap to another
static int pixMap_read(pixMap *p,char *filename); //read in a pixmap from file
static int pixMap_cmp(const void *a, const void *b); //comparator for use in qsort function

static pixMap* pixMap_init(){
	//allocate memory for pixMap object, set variables to zero, and return a pointer to the object
	pixMap *p=malloc(sizeof(pixMap));
	if(!p)exit(1);
	p->image=0;
	p->pixArray=0;
	p->width=0;
	p->height=0;
	return p;
}	
pixMap* pixMap_init_filename(char *filename){
	//use pixMap_int to create the pixMap object
	//use pixMap read to fill the fields of the object
	//return the pointer to the new object
	pixMap *p = pixMap_init();
	pixMap_read(p, filename);
	return p;
}
static int pixMap_read(pixMap *p,char *filename){
	//read and allocate image, read in width and height using using lodepng_decode32_file
	//example is in lodepng/examples - one liner
	//then allocate p->pixArray to hold p->height pointers
	//set p->pixArray[0] to p->image and p->pixArray[i]=p->pixArray[i-1]+p->width
	unsigned error;
	error = lodepng_decode32_file(&p->image, &p->width, &p->height, filename);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
	p->pixArray=malloc(p->height * p->width * sizeof(rgba)); 
	
	p->pixArray[0]=((rgba*) p->image);
	int i;
	for(i = 1; i < p->height; i++) {
		p->pixArray[i] = p->pixArray[i-1]+p->width;
	}
	
	return 0;
}

void pixMap_write_bmp16(pixMap *p, char *filename) {
	//get pointer to new BMP16_map
	BMP16_map* bmp = BMP16_map_init(p->height, p->width, 0, 5, 6, 5);
	int i, j;
	for(i = 0; i < p->height; i++) {
		for(j = 0; j < p->width; j++) {
			//get bits of each value from p->pixArray
			uint16_t rValue = p->pixArray[i][j].r >> 3;
			uint16_t gValue = p->pixArray[i][j].g >> 2;
			uint16_t bValue = p->pixArray[i][j].b >> 3;
			
			uint16_t bmp16= (rValue << 11) | (gValue << 5) | (bValue);
			bmp->pixArray[(bmp->height) - i - 1][j] = bmp16;
		}
	}
	BMP16_write(bmp, filename);
	BMP16_map_destroy(bmp);
}

void pixMap_sort(pixMap *p) {
	qsort(p->image, p->height * p->width, sizeof(rgba), pixMap_cmp);
}

static int pixMap_cmp(const void *a, const void *b) {
    const rgba *ra = (rgba *) a;
    const rgba *rb = (rgba *) b;
    int sum1 = ra->r + ra->g + ra->b;
    int sum2 = rb->r + rb->g + rb->b;
    return(sum1 - sum2);
}

static void pixMap_copy(pixMap *dest,pixMap *source){
	//if source image is zero then reset dest and copy width and height
	if(!source) {
		pixMap_reset(dest);
		dest->width = source->width;
		dest->height = source->height;
	}
	//if source image is not zero
	if(source) {
	  //if width or height are different
	  if(source->width != dest->width || source->height != dest->height) {
	    //if width*height is different then we need to allocate dest->image
	    if(source->width * source->height != dest->width * dest->height) {
	      if(dest->image == 0) {
	      	dest->image = malloc(source->width * source->height * sizeof(rgba));
	      } else {
	      	dest->image = realloc(dest->image, source->width * source->height * sizeof(rgba));
	      }
	    }
	    //if dest->height is different
	    if(dest->height != source->height) {
		  //malloc or realloc dest->pixArray depending on whether dest->pixArray is zero
	      if(dest->pixArray == 0) {
	      	dest->pixArray = malloc(source->height * source->width * sizeof(rgba));
	      } else {
	      	dest->pixArray = realloc(dest->pixArray, source->height * source->width * sizeof(rgba));
	      }
	    }
	  }	    
	  //even if the height is the same set dest->pixArray[0] to dest->image and dest->pixArray[i]=dest->pixArray[i-1]+source->width 
	  dest->pixArray[0] = ((rgba*) dest->image);
	  int i;
	  for(i = 1; i < source->height; i++) {
	      dest->pixArray[i]=dest->pixArray[i-1]+source->width ;
	  }
	  //do a memcpy from source->image to dest->image
	  //set dest->width and dest->height to source values
	  memcpy(dest->image, source->image, (source->height * source-> width * sizeof(rgba)));
	  dest->width = source->width;
	  dest->height = source->height;
	
	}
}


static void pixMap_reset(pixMap *p){
	if(p) {
		free(p->image);
		free(p->pixArray);
	}
	p->image=0;
	p->pixArray=0;
	p->width=0;
	p->height=0;
}	


void pixMap_destroy(pixMap *p){
	pixMap_reset(p);
	free(p);
}


void pixMap_rotate (pixMap *p, float theta){
 //make a new temp blank pixMap structure
 	pixMap *temp = pixMap_init();
 //copy p to temp
	pixMap_copy(temp, p);
 //set the values in the image to zero using memset - Note that the 3rd argument of memset is the size in BYTES
	temp->image = memset(temp->image, 0, temp->width * temp->height * sizeof(rgba));
 //calculate the coordinates ox and oy of the middle of the png graphic
	int ox = p->width / 2;
	int oy = p->height / 2;
 //calculate the values of sine and cosine used by the rotation formula 
	float c = cos(degreesToRadians(-theta));
	float s = sin(degreesToRadians(-theta));
	int x, y;
	//for(int y=0;y<p->height;y++){   //two for loops to loop through each pixel in the original
	 //for(int x=0;x<p->width;x++){
	for(y = 0; y < p->height; y++) {
		for(x = 0; x < p->width; x++) {
	    	//calculate the new rotated coordinates rotx roty using the formula from 
	    	//http://stackoverflow.com/questions/2259476/rotating-a-point-about-another-point-2d
	    	//use the answer from stackoverflowery
		    //However this answer assumes that y is going from the bottom to the top (mathematical convention)
	        //but the pixmap starts at the upper left hand corner and height grows DOWN (scan order)
	        //so use this formula instead where c is cos(degreesToRadians(theta)) and s is sin(degreeToRadians(theta))
	        //    float rotx = c*(x-ox) - s * (oy-y) + ox;
    	    //    float roty = -(s*(x-ox) + c * (oy-y) - oy);
    	    
	    	float rotx = c*(x-ox) - s * (oy-y) + ox;
			float roty = -(s*(x-ox) + c * (oy-y) - oy);
			//round the coordinates to the nearest integer in your calculations (add 0.5 and cast to integer)
			int coorx = (int)(rotx + 0.5);
			int coory = (int)(roty + 0.5);
			//if old coordinates are within the height and width limits
			if(coorx < p->width  && coory < p->height) {
			    //copy the pixel at the old coords to the pixel to the temporary copy using memcpy
			    memcpy(temp->pixArray[y]+x,p->pixArray[coory]+coorx,sizeof(rgba));
			}
		    	
		}
	}
	//copy the temp pixMap to the original
	//destroy the temp;
	pixMap_copy(p, temp);
	pixMap_destroy(temp);
}

void pixMap_gray (pixMap *p){
	//for() loop through pixels using two for loops 
	int i, j;
	for(i = 0; i < p->height; i++) {
		for(j = 0; j < p->width; j++) {
		//calculate average value of r and g and b values (assign to a float variable)
	    //for example the red value of for the pixel at height i, width j would be p->pixel[i][j].r
	    //round float variable to integer (add 0.5 before casting to integer)
	    //assign the rounded value to r,g and b values
	    
	    float avg = ((p->pixArray[i][j].r + p->pixArray[i][j].g + p->pixArray[i][j].b) / 3) + 0.5;
	    int val = (int) avg;
	    p->pixArray[i][j].r = val;
	    p->pixArray[i][j].g = val;
	    p->pixArray[i][j].b = val;
		}
	}
}

int pixMap_write(pixMap *p,char *filename){
 //write out to filename using lodepng_encode32_file
 //example is in lodepng/examples - one liner
	unsigned error = lodepng_encode32_file(filename, p->image, p->width, p->height);
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
	return 0;
}	 
