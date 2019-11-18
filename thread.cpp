#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

//Make image values global for ease of use
int width, height, channels;
int count = 0;

typedef struct {
 unsigned char* img_data;
} image_type;

image_type Grab(){ //Imports image and places it into memory

    //std::cout << count++ << std::endl;
    //STB_Image library: Read in an image as unsigned char
    unsigned char *img = stbi_load("InputMed.png", &width, &height, &channels, 0);
    //STB_Image Error checking from example
    if(img == NULL) {printf("Error in loading the image\n");exit(1);} //Error Checking, copied from stb_image library example
    image_type image;
    image.img_data = img;
    return image;
}

void analyze(image_type image){ //Grabs image via pointer and performs transpose
  
 // std::cout << count++ << std::endl;
  unsigned char* img = image.img_data;
  //Allocate space for new image
     //Allocate space for new image
  size_t img_size = width * height * channels;
  unsigned char *new_img = (unsigned char*)malloc(img_size);

     //STB_Image library error checking
  if(new_img == NULL) {
    printf("Unable to allocate memory for the gray image.\n");
    exit(1);
  }
     //Allocate space for input to live as an array in memory for processing
  //unsigned char out[img_size];
  unsigned char red[width*height];
unsigned char green[width*height];
unsigned char blue[width*height];
    


  
  //Taken from STB_Image library example => used to read image to array Out[] for processing
int k = 0;
  for(unsigned char *p = img; p != img + img_size; p += channels) { //It's segfaulting here
   
    //Path of the pointer to this loop:
    // 1 - grab places it into an image_type struct
    // 2 - digitizer() puts that struct into a buffer
    // 3 - tracker() unpacks that buffer and retrieves a image_type
    // 4 - analyze() takes in that image_type and unpacks the img_data ptr to the img variable
    // 5 - this loop segfaults, but works without threading overhead and the digitize/tracker functions

  //std::cout << *(p) << std::endl;
 red[k]   = (uint8_t)*(p);
  green[k] = (uint8_t)*(p+1);
  blue[k++] = (uint8_t)*(p+2); 
}


     //Divide input string to each color layer to be processed individually

      //Optimize this into other loop
   
     //Perform transpose of each color layer => rotates image CW 90 degrees
        int N = height;  

        unsigned char blueT[width*height];
        unsigned char greenT[width*height];
        unsigned char redT[width*height];
        for(int i = 0; i < N; i++){
          for(int j = 0; j < N; j++){
           blueT[j + (i * N)] = blue[(j * N) + i];
           greenT[ j + (i * N)] = green[(j * N) + i];
           redT[j + ( i * N)] = red[(j * N) + i];
         }
       }


   unsigned char redTR[width*height];
   unsigned char blueTR[width*height];
   unsigned char greenTR[width*height];
   k = 0;
   for(int row = 0; row < height; ++row){
    for(int index = width-1; index >= 0; --index){
      redTR[width*row + index] = redT[k];
      blueTR[width*row + index] = blueT[k];
      greenTR[width*row + index] = greenT[k++];
    }
   }

     //Rebuilds processed output image from each color layer
   unsigned char Recon[width*height*channels];
   for(int i = 0; i < width * height; ++i){
    Recon[3 * i+0] = redT[i];
    Recon[3 * i+1] = greenT[i];
    Recon[3 * i+2] = blueT[i];
  }

     //STB_Image: Create output image and free memory and allocated space
  stbi_write_png("Output.png", width, height, channels, Recon, width * channels);
  stbi_image_free(img);
  free(new_img);
  return;
}


static const unsigned MAX = 8;
void *digitizer();    //Function prototype
void *tracker();     //Function prototype

pthread_cond_t buf_notfull  = PTHREAD_COND_INITIALIZER;
pthread_cond_t buf_notempty = PTHREAD_COND_INITIALIZER;

/* For safe condition variable usage, must use a boolean predicate and  */
/* a mutex with the condition.                                          */
//int bufavail = 0; //this line was included in the pseudocode..... why?
int bufavail = MAX;
image_type frame_buf[MAX];
pthread_mutex_t buflock= PTHREAD_MUTEX_INITIALIZER;

void* digitizer(void* a) { //handles mutex locking and calls grab() for processing       
  image_type dig_image;
  int tail = 0;
  
  while(true){
    dig_image = Grab(); //Grab() Takes care of repeatedlty grabbing the image into memory
    pthread_mutex_lock(&buflock);
    
    if (bufavail == 0){
      pthread_cond_wait(&buf_notempty,&buflock);     
    }
    pthread_mutex_unlock(&buflock);

    frame_buf[tail % MAX] = dig_image;
    tail = tail + 1; 
    pthread_mutex_lock(&buflock);
    bufavail = bufavail - 1;

    pthread_mutex_unlock(&buflock);
    if(bufavail < MAX){
      pthread_cond_broadcast(&buf_notfull);
    }
  }
  return 0; 
}

void* tracker(void* a) { //handles mutex locking and calls anaylyze() for processing       
  image_type track_image;
  int head = 0;
  
  while(true){

    pthread_mutex_lock(&buflock);
    if (bufavail == MAX){
      pthread_cond_wait(&buf_notfull, &buflock);
    }
    pthread_mutex_unlock(&buflock);

    track_image = frame_buf[head % MAX];
    head = head + 1;
    pthread_mutex_lock(&buflock);
    bufavail = bufavail + 1;
    pthread_mutex_unlock(&buflock);
    if(bufavail > 0){
      pthread_cond_broadcast(&buf_notempty);
    }
    analyze(track_image);
  }
  return 0;
} 



int main() {
 pthread_t Digitize, Tracker;
    /* Create independent threads each of which will execute function */
 
 pthread_create(&Digitize, NULL, digitizer, NULL);
 pthread_create(&Tracker, NULL, tracker, NULL);

     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

 pthread_join(Digitize, NULL);
 pthread_join(Tracker, NULL); 

 exit(0);
}
