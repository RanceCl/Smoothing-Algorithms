
	/*
	ECE 4310
	Lab 1
	Roderick "Rance" White
	
	** This program reads a PPM image.
	** It smooths it using a standard nxn mean filter.
	** The program also demonstrates how to time a piece of code.
	**
	** To compile, must link using -lrt  (man clock_gettime() function).
	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void Basic_2D_Convolution(unsigned char *image, int ROWS, int COLS, int MS, int W);
void Separable_Filter(unsigned char *image, int ROWS, int COLS, int MS, int W);
void Sliding_Window(unsigned char *image, int ROWS, int COLS, int MS, int W);
void Write_Image(unsigned char *image, int ROWS, int COLS, int WhichFilter);


int main()

{
	FILE						*fpt;
	unsigned char		*image;
	char						header[320];
	int							ROWS,COLS,BYTES;

	int 						MeanSize=7;							// This will the the n x n mean filter value.
	int 						WindowSize=MeanSize/2;	// The window of local-area information

	printf("Filter size: %d; Window size: %d\n",MeanSize,WindowSize);

	/* read image */
	if ((fpt=fopen("bridge.ppm","rb")) == NULL)
	{
		printf("Unable to open bridge.ppm for reading\n");
		exit(0);
	}

	/* read image header (simple 8-bit greyscale PPM only) */
	fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);	
	if (strcmp(header,"P5") != 0  ||  BYTES != 255)
	{
		printf("Not a greyscale 8-bit PPM image\n");
		exit(0);
	}

	/* allocate dynamic memory for image */
	image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	header[0]=fgetc(fpt);	/* read white-space character that separates header */
	fread(image,1,COLS*ROWS,fpt);
	fclose(fpt);


	/* Basic 2D Convolution */
	Basic_2D_Convolution(image, ROWS, COLS, MeanSize, WindowSize);							// Smooth Image

	/* Separable Filter */
	Separable_Filter(image, ROWS, COLS, MeanSize, WindowSize);									// Smooth Image

	/* Separable Filter with Sliding Window */
	Sliding_Window(image, ROWS, COLS, MeanSize, WindowSize);											// Smooth Image
}



/* This will smooth out the inputted image using basic 2D Convolution */
void Basic_2D_Convolution(unsigned char *image, int ROWS, int COLS, int MS, int W)
{
	unsigned char		*smoothed;
	int							r,c,r2,c2,sum;
	struct 					timespec	tp1,tp2;
	
	printf("\n--------------Basic 2D Convolution-------------\n");
	
	/* allocate memory for smoothed version of image */
	smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
	
	/* query timer */
	clock_gettime(CLOCK_REALTIME,&tp1);
	printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, skipping the border points */
	for (r=W; r<ROWS-W; r++){
		for (c=W; c<COLS-W; c++){
			sum=0;
			for (r2=-W; r2<=W; r2++)
				for (c2=-W; c2<=W; c2++)
					sum+=image[(r+r2)*COLS+(c+c2)];
			smoothed[r*COLS+c]=sum/(MS*MS);
		}
	}

	/* query timer */
	clock_gettime(CLOCK_REALTIME,&tp2);
	printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
	printf("%ld\n",tp2.tv_nsec-tp1.tv_nsec);
	
	/* Write to correct image */
	Write_Image(smoothed, ROWS, COLS, 1);

}

/* This will smooth out the inputted image using 2 1D Separable Filters */
void Separable_Filter(unsigned char *image, int ROWS, int COLS, int MS, int W)
{
	unsigned char 	*smoothed;
	float 					*smoothed1;
	int							r,c,r2,c2,sum;
	struct 					timespec	tp1,tp2;
	
	printf("\n--------------Separable Filter-------------\n");
	
	/* allocate memory for each 1D array */
	smoothed1=(float *)calloc(ROWS*COLS,sizeof(float));
	smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

	/* query timer */
	clock_gettime(CLOCK_REALTIME,&tp1);
	printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, skipping the border points */
	//The Row 1D Filter
	for (r=0; r<ROWS; r++)
	{
		for (c=W; c<COLS-W; c++)
		{
			sum=0;
			for (c2=-W; c2<=W; c2++)
				sum+=image[(r)*COLS+(c+c2)];
			smoothed1[r*COLS+c]=sum;
		}
	}

	//The Column 1D Filter
	for (c=0; c<COLS; c++)
	{
		for (r=W; r<ROWS-W; r++)
		{
			sum=0;
			for (r2=-W; r2<=W; r2++)
				sum+=smoothed1[(r+r2)*COLS+(c)];
			smoothed[r*COLS+c]=sum/(MS*MS);
		}
	}

	/* query timer */
	clock_gettime(CLOCK_REALTIME,&tp2);
	printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
	printf("%ld\n",tp2.tv_nsec-tp1.tv_nsec);
	
	/* Write to correct image */
	Write_Image(smoothed, ROWS, COLS, 2);
}


/* This will smooth out the inputted image using the Sliding Window Method */
void Sliding_Window(unsigned char *image, int ROWS, int COLS, int MS, int W)
{
	unsigned char		*smoothed;
	float						*smoothed1;
	int							r,c,r2,c2,sum;
	struct 					timespec	tp1,tp2;
	
	printf("\n--------------Sliding Window-------------\n");
	
	/* allocate memory for each 1D array */
	smoothed1=(float *)calloc(ROWS*COLS,sizeof(float));
	smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));

	/* query timer */
	clock_gettime(CLOCK_REALTIME,&tp1);
	printf("%ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, skipping the border points */
	//The Row 1D Filter
	for (r=0; r<ROWS; r++)
	{
		// Initial window
		sum=0;												// Reset sum to 0 for each new row
		c=W;													// Reset column indicator to the window edge

		// Summation of the row within the initial window region
		for (c2=-W; c2<=W; c2++)
			sum+=image[(r)*COLS+(c+c2)];
		smoothed1[r*COLS+c]=sum;
		
		// Slide the window along the row, adding and subtracting the ends as it moves.
		for (c=W+1; c<COLS-W; c++)
		{
			sum+=image[(r)*COLS+(c+W)];				//Add the new front edge (column) of the window
			sum-=image[(r)*COLS+(c-W-1)];			//Subtract the previous back edge (column) of the window
			smoothed1[r*COLS+c]=sum;
		}
	}

	//The Column 1D Filter
	for (c=0; c<COLS; c++)
	{
		// Initial window
		sum=0;												// Reset sum to 0 for each new column
		r=W;													// Reset row indicator to the window edge
		
		// Summation of the column within the initial window region
		for (r2=-W; r2<=W; r2++)
			sum+=smoothed1[(r+r2)*COLS+(c)];
		smoothed[r*COLS+c]=sum/(MS*MS);
			
		// Slide the window along the column, adding and subtracting the ends as it moves.
		for (r=W+1; r<ROWS-W; r++)
		{		
			sum+=smoothed1[(r+W)*COLS+(c)];				//Add the new front edge (row) of the window
			sum-=smoothed1[(r-W-1)*COLS+(c)];			//Subtract the previous back edge (row) of the window
			smoothed[r*COLS+c]=sum/(MS*MS);
		}
	}

	/* query timer */
	clock_gettime(CLOCK_REALTIME,&tp2);
	printf("%ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
	printf("%ld\n",tp2.tv_nsec-tp1.tv_nsec);
	
	/* Write to correct image */
	Write_Image(smoothed, ROWS, COLS, 3);
}


/* This function serves to write the image to the appropriate file for more concise code */
void Write_Image(unsigned char *image, int ROWS, int COLS, int WhichFilter)
{
	FILE						*fpt;
	/* write out smoothed image to see result */
	switch (WhichFilter)
	{
		// Write to 2D Convolution Image
		case 1 :
			fpt=fopen("smoothed2DConvolution.ppm","w");
			break;
		// Write to Seperable Filter Image
		case 2 : 
			fpt=fopen("smoothedSeperableFilter.ppm","w");
			break;
		// Write to Sliding Window Image
		case 3 : 
			fpt=fopen("smoothedSlidingWindow.ppm","w");
			break;
		default :
			fpt=fopen("smoothed.ppm","w");
	}
	fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	fwrite(image,COLS*ROWS,1,fpt);
	fclose(fpt);
}








