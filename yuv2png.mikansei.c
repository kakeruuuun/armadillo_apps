/*
 *	YUV422からPNGへの変換
 *  34行目あたりを追加する
*/

#include <stdio.h>
#include <stdlib.h>
#include <png.h>

#define WIDTH (320)
#define HEIGHT (240)

#define Y1 YUYV[0]
#define U  YUYV[1]
#define Y2 YUYV[2]
#define V  YUYV[3]

void write_png(char *file_name, unsigned char **image);

int main(int argc, char *argv[])
{
	FILE *fp_yuv;
	int i, j, width_length;
	unsigned char YUYV[4];
	int R, G, B;
	unsigned char R1, G1, B1;
	unsigned char R2, G2, B2;
	unsigned char **img;

	if ((fp_yuv = fopen(argv[1], "rb")) == NULL)
	{
		fprintf(stderr, "cannot open %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	width_length = WIDTH * 3;
	img = (png_bytepp)malloc(sizeof(png_bytep) * HEIGHT);
	for (i = 0; i < HEIGHT; i++)
	{
		img[i] = (png_bytep)malloc(sizeof(png_byte) * width_length);
		for (j = 0; j < width_length; j += 6)
		{
			//Y1,U,Y2,Vを取得
			fread(YUYV, 1, 4, fp_yuv);
			//RGBを計算
			R1 = 1.164 * (Y1 - 16) + 1.569 * (V - 128);
			G1 = 1.164 * (Y1 - 16) - 0.391 * (U - 128) - 0.813 * (V - 128);
			B1 = 1.164 * (Y1 - 16) + 2.018 * (U - 128);
			R2 = 1.164 * (Y2 - 16) + 1.569 * (V - 128);
			G2 = 1.164 * (Y2 - 16) - 0.391 * (U - 128) - 0.813 * (V - 128);
			B2 = 1.164 * (Y2 - 16) + 2.018 * (U - 128);

			img[i][j + 0] = (unsigned char)R1;
			img[i][j + 1] = (unsigned char)G1;
			img[i][j + 2] = (unsigned char)B1;
			img[i][j + 3] = (unsigned char)R2;
			img[i][j + 4] = (unsigned char)G2;
			img[i][j + 5] = (unsigned char)B2;
		}
	}
	fclose(fp_yuv);
	write_png(argv[2], img);
	for (i = 0; i < HEIGHT; i++)
	{
		free(img[i]);
	}
	free(img);
	return 0;
}

void write_png(char *file_name, unsigned char **image)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;

	fp = fopen(file_name, "wb");
	png_ptr = png_create_write_struct(
		PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, WIDTH, HEIGHT,
				 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, image);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	return;
}
