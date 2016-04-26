#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"

BMP_Image *Convert_24_to_16_BMP_Image(BMP_Image *image) 
{
	int new_height = image->header.height;
	int new_width = image->header.width;

	int new_bits = 16;
	int maybe_bytes = new_width * 3;
	int new_maybe_bytes = maybe_bytes;
	while (new_maybe_bytes % 4 != 0) {
		new_maybe_bytes ++;	
	}
	int new_bytes = new_width * new_bits / 8;

	BMP_Image *t_image = (BMP_Image*)malloc(sizeof(BMP_Image));

	if (t_image == NULL) {
		fprintf(stderr, "Error in allocation of t_image\n");		
		return NULL;	
	}

	t_image->header = image->header;
	
	t_image->header.bits = new_bits;
	
	int new_image_size = new_height * new_bytes;

	t_image->header.imagesize = new_image_size;
	t_image->header.size = new_image_size + sizeof(BMP_Header);

	t_image->data = (unsigned char*)calloc(new_image_size, sizeof(unsigned char));

	if (t_image->data == NULL) {
		fprintf(stderr, "Error in allocation of t_image->data\n");
		free(t_image);
		return NULL;	
	}

	//unsigned char array[new_height][new_bytes];
	
	unsigned char **array = (unsigned char **)malloc(sizeof(unsigned char *) * new_height);
	if (array == NULL) {
		free(t_image->data);
		free(t_image);
		fprintf(stderr, "Error in allocation of array\n");
		return NULL;	
	}

	int i;

	for (i = 0; i < new_height; i++) {
		array[i] = (unsigned char *)malloc(sizeof(unsigned char) * new_bytes);
		if (array[i] == NULL) {	
			while (i > 0) {
				i--;
				free(array[i]);
			}
			fprintf(stderr, "Error in allocation of array[i]\n");
			free(array);
			free(t_image->data);
			free(t_image);
			return NULL;
		}
	}

	int count = 0;
	int j;
	//printf("before padding = %d new bytes = %d\n", before_padding, new_bytes);
	//printf("maybe_bytes = %d new_maybe_bytes = %d\n", maybe_bytes, new_maybe_bytes);		
	
	for (i = 0; i < new_height;i++) {
		int check = 0;
		int check2 = 0;
		while (check2 < maybe_bytes) {
			unsigned char one = image->data[count++] >> 3;
			unsigned char two = image->data[count++] >> 3;
			unsigned char three = image->data[count++] >> 3;
			check2 += 3;
			array[i][check++] = (one) | (two << 5);
			array[i][check++] = ((two >> 3) | (three << 2)) & (0x7F);
		}
		
		while (check2 < new_maybe_bytes) {
			//array[i][check] = (unsigned char) 0x00;
			check++;
			check2++;
			count++;
		}
	}
	count = 0;

	for (i = 0; i < new_height;i++) {
		for (j = 0; j < new_bytes; j++) {
			t_image->data[count++] = array[i][j];			
		}	
	}

	for (i = 0; i < new_height; i++) {
		free(array[i]);
	}
	free(array);
	return t_image;
}

BMP_Image *Convert_16_to_24_BMP_Image(BMP_Image *image)
{
	int new_height = image->header.height;
	int new_width = image->header.width;


	int new_bits = 24;

	int new_bytes = new_width * new_bits / 8;
	int before_padding = new_width * new_bits / 8;

	BMP_Image *t_image = (BMP_Image*)malloc(sizeof(BMP_Image));

	t_image->header = image->header;

	t_image->header.bits = new_bits;
	t_image->header.width = new_width;
	t_image->header.height = new_height;
	
	
	while (new_bytes % 4 != 0) {
		new_bytes ++;	
	}

	int new_image_size = new_height * new_bytes;

	t_image->header.imagesize = new_image_size;
	t_image->header.size = new_image_size + sizeof(BMP_Header);

	t_image->data = (unsigned char*)calloc(new_image_size, sizeof(unsigned char));

	unsigned char array[new_height][new_bytes];
	
	int count = 0;
	int i, j;
	for (i = 0; i < new_height;i++) {
		int check = 0;

		while (check < before_padding) {
			unsigned short combine1 = ((unsigned short) image->data[count + 1]) << 8;
			unsigned short combine2 = ((unsigned short) image->data[count]);
			unsigned short combine = combine1 | combine2;
			count += 2;
			array[i][check++] = (unsigned char) (((combine & BLUE_MASK) >> 0) & 0xFF) * 255 / 31;
			array[i][check++] = (unsigned char) (((combine & GREEN_MASK) >> 5) & 0xFF) * 255 / 31;
			array[i][check++] = (unsigned char) (((combine & RED_MASK) >> 10) & 0xFF) * 255 / 31;
		}
		while (check < new_bytes) {
			array[i][check] = (unsigned char) 0x00;
			check++;
		}
	}

	count = 0;
	for (i = 0; i < new_height;i++) {
		for (j = 0; j < new_bytes; j++) {
			t_image->data[count++] = array[i][j];			
		}	
	}

	return t_image;
}


////////////////////////// PE 11 Functions ////////////////////////

int Is_BMP_Header_Valid(BMP_Header* header, FILE *fptr) {
  // Make sure this is a BMP file
  if (header->type != 0x4d42) {
     return FALSE;
  }
  // skip the two unused reserved fields

  // check the offset from beginning of file to image data
  // essentially the size of the BMP header
  // BMP_HEADER_SIZE for this exercise/assignment
  if (header->offset != BMP_HEADER_SIZE) {
     return FALSE;
  }
      
  // check the DIB header size == DIB_HEADER_SIZE
  // For this exercise/assignment
  if (header->DIB_header_size != DIB_HEADER_SIZE) {
     return FALSE;
  }

  // Make sure there is only one image plane
  if (header->planes != 1) {
    return FALSE;
  }
  // Make sure there is no compression
  if (header->compression != 0) {
    return FALSE;
  }

  // skip the test for xresolution, yresolution

  // ncolours and importantcolours should be 0
  if (header->ncolours != 0) {
    return FALSE;
  }
  if (header->importantcolours != 0) {
    return FALSE;
  }
  
  // Make sure we are getting 24 bits per pixel
  // or 16 bits per pixel
  // only for this assignment
  if (header->bits != 24 && header->bits != 16) {
    return FALSE;
  }

  // fill in extra to check for file size, image size
  // based on bits, width, and height

	fseek(fptr, 0, SEEK_END);
	int image_size = ftell(fptr) - 54;
	if (header->imagesize != image_size) {
		fprintf(stderr, "Header issue relating to the imagesize\n");
		return FALSE;
	}
	
	if (header->size != image_size + 54) {
		fprintf(stderr, "Header issue relating to the size\n");
		return FALSE;
	}


  return TRUE;
}


BMP_Image *Read_BMP_Image(FILE* fptr) {

  // go to the beginning of the file

	BMP_Image *bmp_image = NULL;

  //Allocate memory for BMP_Image*;

	bmp_image = (BMP_Image*)malloc(sizeof(BMP_Image));

	if (bmp_image == NULL) {
		fprintf(stderr, "Error allocating memory (bmp_image/ Read_BMP_Image)\n");		
		return NULL;
	}

  //Read the first 54 bytes of the source into the header

	fseek(fptr, 0, SEEK_SET);
	int header_size = fread(&bmp_image->header, sizeof(BMP_Header), 1, fptr); 

	if (header_size != 1) {
		fprintf(stderr, "Error reading the header\n");
		free(bmp_image);
		return NULL;
	}	
	
	if (Is_BMP_Header_Valid (&bmp_image->header, fptr) == FALSE) {
		fprintf(stderr, "Header is not valid\n");
		free(bmp_image);		
		return NULL;
	}
	
  // Allocate memory for image data

	fseek(fptr, 0, SEEK_END);
	int image_size = ftell(fptr) - sizeof(BMP_Header);

	bmp_image->data = (unsigned char*)malloc(sizeof(unsigned char) * image_size);

	if (bmp_image->data == NULL) {
		fprintf(stderr, "Error allocating memory (bmp_image/ Read_BMP_Image)\n");	
		free(bmp_image);		
		return NULL;
	} 

  // read in the image data	

	fseek(fptr, 0, SEEK_SET); 
	fseek(fptr, sizeof(BMP_Header), SEEK_SET); 
	int total_image = fread(bmp_image->data, sizeof(char), image_size, fptr); 

	if (total_image != image_size) {
		printf("%d %d\n", total_image, image_size);
		free(bmp_image->data);
		free(bmp_image);
		fprintf(stderr, "Cannot read image\n");
		return NULL;	
	}

  return bmp_image;
}

int Write_BMP_Image(FILE* fptr, BMP_Image* image) 
{
   // go to the beginning of the file
	fseek(fptr, 0, SEEK_SET);

   // write header
	int header_written = fwrite (&image->header, sizeof(BMP_Header), 1, fptr);
	if (header_written != 1) {
		fprintf(stderr, "Cannot write image to file correctly\n");
		return FALSE;	
	}	
	fseek(fptr, sizeof(BMP_Header), SEEK_SET); 

   // write image data

	int image_written  = fwrite (image->data, sizeof(char), image->header.imagesize, fptr);
	if (image_written != image->header.imagesize) {
		fprintf(stderr, "Cannot write image to file correctly\n");
		return FALSE;	
	}	

   return TRUE;
}


void Free_BMP_Image(BMP_Image* image) {
	free(image->data);
	free(image);
}
