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
unsigned char* test;

typedef struct {
 unsigned char* img_data;
} image_type;

int FB_Max = 10;
image_type frame_buf[10];

image_type Grab(){ //Imports image and places it into memory

     //STB_Image library: Read in an image as unsigned char
 unsigned char *img = stbi_load("InputMed.png", &width, &height, &channels, 0);
     //STB_Image Error checking from example
     if(img == NULL) {
      printf("Error in loading the image\n");
      exit(1);
     } //Error Checking, copied from stb_image library example
    image_type image;
    image.img_data = img;
    
   return image;
 }

void analyze(image_type a){ //Grabs image via pointer and performs transpose
  
  unsigned char *img = a.img_data;

     //Allocate space for new image
  size_t img_size = width * height * channels;
  unsigned char *new_img = (unsigned char*)malloc(img_size);

     //STB_Image library error checking
  if(new_img == NULL) {
    printf("Unable to allocate memory for the gray image.\n");
    exit(1);
  }

     //Allocate space for input to live as an array in memory for processing
  unsigned char out[img_size];
  int i = 0;

     //Taken from STB_Image library example => used to read image to array Out[] for processing
  for(unsigned char *p = img; p != img + img_size; p += channels) {
    out[i]   = (uint8_t)*(p);
    out[i+1] = (uint8_t)*(p+1);
    out[i+2] = (uint8_t)*(p+2); 
    i += channels;
  }

     //Divide input string to each color layer to be processed individually
  unsigned char red[width*height];
  unsigned char green[width*height];
  unsigned char blue[width*height];
  int x = 0;
     for(int i = 0; i < img_size; i+=3){ // i+=3 to step over color channels
      red[x]     = out[i];
      green[x] = out[i + 1];
      blue[x]  = out[i + 2];
          x++; //Iterate to build color layers
        }

     //Perform transpose of each color layer => rotates image CW 90 degrees
        int N = height;  

        unsigned char blueT[width*height];
        for(int i = 0; i < N; i++){
          for(int j = 0; j < N; j++){
           blueT[j + (i * N)] = blue[(j * N) + i];
         }
       }

       unsigned char greenT[width*height];
       for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
         greenT[ j + (i * N)] = green[(j * N) + i];
       }
     }

     unsigned char redT[width*height];
     for(int i = 0; i < N; i++){ 
      for(int j = 0; j < N; j++){
       redT[j + ( i * N)] = red[(j * N) + i];
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
  stbi_write_png("output.png", width, height, channels, Recon, width * channels);
  stbi_image_free(img);
  free(new_img);
  return;
}

void Digitizer(){
  image_type image;
  for(int i = 0; i < FB_Max; ++i){
      image = Grab();
     frame_buf[i] = image;
   }
}
void Tracker(){
  for(int i = 0; i < FB_Max; ++i){
      analyze(frame_buf[i]);
   }
}


int main(){
  
  Digitizer();
  Tracker();
  return 0;
  
}