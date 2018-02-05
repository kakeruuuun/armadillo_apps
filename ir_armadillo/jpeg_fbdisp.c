/*
 * fbdisp_jpeg.c 
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <jpeglib.h>

#define DEVICE_NAME "/dev/fb0"
#define DIV_BYTE 8

#define COLOR_RED	0xf800
#define COLOR_GREEN	0x07e0
#define COLOR_BLUE	0x001f
#define COLOR_WHITE	0xffff
#define COLOR_BLACK	0x0000
#define COLOR_YELLOW	0xffe0

/* function prototype */
void send_current_error_msg(char *ptr);
void send_current_information(char *ptr);

int main(int argc, char *argv[])
{
	FILE            *fp;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY img;
	unsigned char *p;
	int i, j;
	int width;
	int height;

	// JPEGオブジェクトの初期化
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );

	// ファイルを開く
	fp = fopen( argv[1], "rb" );
	jpeg_stdio_src( &cinfo, fp );

	// ヘッダの読み込み
	jpeg_read_header( &cinfo, TRUE );

	// 展開の開始
	jpeg_start_decompress( &cinfo );

	// 幅と高さの取得
	width = cinfo.output_width;
	height = cinfo.output_height;

	// イメージを保持するメモリ領域の確保と初期化
	img = (JSAMPARRAY)malloc( sizeof( JSAMPROW ) * height );
	for ( i = 0; i < height; i++ ) {
		img[i] = (JSAMPROW)calloc( sizeof( JSAMPLE ), 3 * width );
	}

	// 全イメージデータを取得	
	while( cinfo.output_scanline < cinfo.output_height ) {
		jpeg_read_scanlines( &cinfo,
			img + cinfo.output_scanline,
			cinfo.output_height - cinfo.output_scanline
		);
	}

	// 展開の終了
	jpeg_finish_decompress( &cinfo );

	// JPEGオブジェクトの破棄
	jpeg_destroy_decompress( &cinfo );

	// ファイルを閉じる
	fclose( fp );

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
		p=img[f_line];	//１行分の画像データをポインタｐに
		int f_width=0;
		for ( i=0 ; i<xres ; i++ ) {
			if(i<x1 || j<y1 || i>=x1+width || j>=y1+height) {
				tcolor = COLOR_BLACK;
			} else {
				//ポインタpから３バイト取得（RGB)
				RGB[0] = *p++ >> 3;
				RGB[1] = *p++ >> 2;
				RGB[2] = *p++ >> 3;
				//LCD表示用RGB（１６ビット）に変換
				tcolor = (RGB[0] << 11 | RGB[1] << 5 | RGB[2]);
				if( ++f_width == width ) f_line++;	//１行分処理が終わったら次の行に
			}

			/* 格納位置計算 */
			location = ((i+vinfo.xoffset) * vbpp / DIV_BYTE) + (j+vinfo.yoffset) * line_len ;
			/* 着色 */
			*((unsigned short *)(fbptr + location)) = tcolor;
		}
	}
	munmap(fbptr,screensize);
	close(fd_framebuffer);
	for (i = 0; i < height; i++) free(img[i]);
	free(img);
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
