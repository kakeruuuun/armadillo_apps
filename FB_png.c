/*
 * fbdisp_png.c 
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <png.h>

#define DEVICE_NAME "/dev/fb0"
#define DIV_BYTE 8

#define COLOR_RED	0xf800
#define COLOR_GREEN	0x07e0
#define COLOR_BLUE	0x001f
#define COLOR_WHITE	0xffff
#define COLOR_BLACK	0x0000
#define COLOR_YELLOW	0xffe0

#define PNG_BYTES_TO_CHECK (4)

/* function prototype */
void send_current_error_msg(char *ptr);
void send_current_information(char *ptr);

void check_if_png(char *file_name, FILE **fp);
void read_png_info(FILE *fp, png_structp *png_ptr, png_infop *info_ptr);
void read_png_image(FILE *fp, png_structp png_ptr, png_infop info_ptr,
	png_bytepp *image, png_uint_32 *width, png_uint_32 *height);

int main(int argc, char *argv[])
{
	FILE            *fp;
	png_structp     png_ptr;
	png_infop       info_ptr;
	unsigned char   **image;
	unsigned long   width, height;
	png_uint_32     i;
	png_uint_32     j;
	unsigned char *p;
	
	check_if_png(argv[1], &fp);
	read_png_info(fp, &png_ptr, &info_ptr);
	read_png_image(fp, png_ptr, info_ptr, &image, &width, &height);


{                                                               // 以下６行はgAMAチャンクを取得し表示します
	double          file_gamma;
	
	if (png_get_gAMA(png_ptr, info_ptr, &file_gamma))
	        printf("gamma = %lf\n", file_gamma);
}

{                                                               // 以下８行はtEXtチャンクを取得し表示します
	png_textp       text_ptr;
	int             num_text;
	
	if (png_get_text(png_ptr, info_ptr, &text_ptr, &num_text))
	        for (i = 0; i < num_text; i++)
	                printf("%s = %s\n", text_ptr[i].key, text_ptr[i].text);
}
	int fd_framebuffer ;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int screensize ;
	long int location;
	char *fbptr ;
	char tmp[DIV_BYTE*10];

	int x1 , y1 ;
	int xres,yres,vbpp,line_len;
	unsigned short tcolor ;

	unsigned char RGB[3];
 
	int x, y, max;
	char type[5];

	/* 読み書き用にファイルを開く */
	fd_framebuffer = open( DEVICE_NAME , O_RDWR);
	if ( !fd_framebuffer ) {
	    send_current_error_msg("Framebuffer device open error !");
	    exit(1);
	}
	send_current_information("The framebuffer device was opened !");

	/* 固定スクリーン情報取得 */
	if ( ioctl( fd_framebuffer , FBIOGET_FSCREENINFO , &finfo ) ) {
	    send_current_error_msg("Fixed information not gotton !");
	    exit(2);
	}

	/* 変動スクリーン情報取得 */
	if ( ioctl( fd_framebuffer , FBIOGET_VSCREENINFO , &vinfo ) ) {
	    send_current_error_msg("Variable information not gotton !");
	    exit(3);
	}
	xres = vinfo.xres ;
	yres = vinfo.yres ;
	vbpp = vinfo.bits_per_pixel ;
	line_len = finfo.line_length ;
	sprintf( tmp , "%d(pixel)x%d(line), %dbpp(bits per pixel)",xres,yres,vbpp);
	send_current_information( tmp );

	/* バイト単位でのスクリーンのサイズを計算 */
	screensize = xres * yres * vbpp / DIV_BYTE ;

	/* デバイスをメモリにマップする */
	fbptr = (char *)mmap(0,screensize,PROT_READ | PROT_WRITE,MAP_SHARED,fd_framebuffer,0);
	if ( (int)fbptr == -1 ) {
	    send_current_error_msg("Don't get framebuffer device to memory !");
	    exit(4);
	}
	send_current_information("The framebuffer device was mapped !");

	x1 = (xres - width)/2;
	y1 = (yres - height)/2;

int f_line=0;
	for(j=0; j<yres; j++) {
		p=image[f_line];	//１行分の画像データをポインタpに
int f_width=0;
		for ( i=0 ; i<xres ; i++ ) {
			if(i<x1 || j<y1 || i>=x1+width || j>=y1+height) {
				tcolor = COLOR_BLACK;
			} else {
				//ポインタｐからRGB値を取得
				RGB[0] = *p++ >> 3;
				RGB[1] = *p++ >> 2;
				RGB[2] = *p++ >> 3;

				p++;	//Alpha値読み飛ばし
				//LCD用RGB（１６ビット）に変換
				tcolor = (RGB[0] << 11 | RGB[1] << 5 | RGB[2]);
				if( ++f_width == width ) f_line++;	//１行分の処理が終わったら次の行に
			}

			/* 格納位置計算 */
			location = ((i+vinfo.xoffset) * vbpp / DIV_BYTE) + (j+vinfo.yoffset) * line_len ;
			/* 着色 */
			*((unsigned short *)(fbptr + location)) = tcolor;
		}
	}
	munmap(fbptr,screensize);
	close(fd_framebuffer);
	for (i = 0; i < height; i++) free(image[i]);
	free(image);
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	fclose(fp);
	return 0;
}

void send_current_error_msg(char *ptr)
{
	fprintf( stderr , "%s\n" , ptr );
}

void send_current_information(char *ptr)
{
	fprintf( stdout , "%s\n" , ptr );

}

void check_if_png(char *file_name, FILE **fp)
{
	char    sig[PNG_BYTES_TO_CHECK];
	
	if ((*fp = fopen(file_name, "rb")) == NULL)
	        exit(EXIT_FAILURE);
	if (fread(sig, 1, PNG_BYTES_TO_CHECK, *fp) != PNG_BYTES_TO_CHECK) {
	        fclose(*fp);
	        exit(EXIT_FAILURE);
	}
	//if (!png_sig_cmp(sig, 0, PNG_BYTES_TO_CHECK)) {
	if (!png_check_sig(sig, PNG_BYTES_TO_CHECK)) {
	        fclose(*fp);
	        exit(EXIT_FAILURE);
	}
}

void read_png_info(FILE *fp, png_structp *png_ptr, png_infop *info_ptr)
{
	*png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (*png_ptr == NULL) {
	        fclose(fp);
	        exit(EXIT_FAILURE);
	}
	*info_ptr = png_create_info_struct(*png_ptr);
	if (*info_ptr == NULL) {
	        png_destroy_read_struct(png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	        fclose(fp);
	        exit(EXIT_FAILURE);
	}
	if (setjmp((*png_ptr)->jmpbuf)) {
	        png_destroy_read_struct(png_ptr, info_ptr, (png_infopp)NULL);
	        fclose(fp);
	        exit(EXIT_FAILURE);
	}
	png_init_io(*png_ptr, fp);
	png_set_sig_bytes(*png_ptr, PNG_BYTES_TO_CHECK);
	png_read_info(*png_ptr, *info_ptr);
}

void read_png_image(FILE *fp, png_structp png_ptr, png_infop info_ptr,
	png_bytepp *image, png_uint_32 *width, png_uint_32 *height)
{
	png_uint_32     i, j;
	
	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);
	if ((*image = (png_bytepp)malloc(*height * sizeof(png_bytep))) == NULL) {
	        fclose(fp);
	        exit(EXIT_FAILURE);
	}
	for (i = 0; i < *height; i++) {
	        (*image)[i] = (png_bytep)malloc(png_get_rowbytes(png_ptr, info_ptr));
	        if ((*image)[i] == NULL) {
	                for (j = 0; j < i; j++) free((*image)[j]);
	                free(*image);
	                fclose(fp);
	                exit(EXIT_FAILURE);
	        }
	}
	png_read_image(png_ptr, *image);
}

