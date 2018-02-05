#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/fs.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/fb0"
#define DIV_BYTE 8

#define X_PIXEL_MAX 480
#define Y_LINE_MAX  272

#define COLOR_RED	0xf800
#define COLOR_GREEN	0x07e0
#define COLOR_BLUE	0x001f
#define COLOR_WHITE	0xffff
#define COLOR_BLACK	0x0000
#define COLOR_YELLOW	0xffe0

int main(int argc, char *argv[])
{
	int fd;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;
	long int size ;
	long int index;
	char *p ;
	FILE *fp;
	unsigned char dummy[15];
	unsigned char inbuf[4];
	unsigned char temp[3];
	int RGB[3];

	int i , j ;	//for loop counter
	int k = 0;
	int cnt = 0;
	int width,height,depth,len,ox,oy;	//FB infomation
	unsigned short tcolor ;

	/* 読み書き用にファイルを開く */
	fd = open( DEVICE_NAME , O_RDWR);
	if ( !fd ) {
	    fprintf(stderr,"Framebuffer device open error !\n");
	    exit(1);
	}
	printf( "Open finished" );

	/* 固定スクリーン情報取得 */
	if ( ioctl( fd , FBIOGET_FSCREENINFO , &finfo ) ) {
	    fprintf(stderr,"Fixed information not gotton !\n");
	    exit(2);
	}
	printf( "Fit screen info get finished\n" );

	/* 変動スクリーン情報取得 */
	if ( ioctl( fd , FBIOGET_VSCREENINFO , &vinfo ) ) {
	    fprintf(stderr,"Variable information not gotton !\n");
	    exit(3);
	}
	printf( "Valuable screen info get finished\n" );

	width = vinfo.xres ;
	height = vinfo.yres ;
	depth = vinfo.bits_per_pixel ;
	/* バイト単位でのスクリーンのサイズを計算 */
	size = width * height * depth / DIV_BYTE ;

	len = finfo.line_length ;
	ox = vinfo.xoffset ;
	oy = vinfo.yoffset ;

	printf( "width=%d\n", width );
	printf( "height=%d\n", height );
	printf( "depth=%d\n", depth );
	printf( "size=%d\n", size );
	printf( "len=%d\n", len );
	printf( "ox=%d\n", ox );
	printf( "oy=%d\n", oy );

	/* デバイスをメモリにマップする */
	p = (char *)mmap(0,size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	if ( (int)p == -1 ) {
	    fprintf(stderr,"Don't get framebuffer device to memory !");
	    exit(4);
	}

	if((fp = fopen(argv[1], "rb")) == NULL)
	{
		perror("fopen\n");
	}


	for(j=0; j<height; j++) {
		for ( i=0 ; i<width ; i++ ) {
			
			
			if(i >= 320 || j >= 240)
			{
				tcolor = COLOR_BLACK;
			}
			else
			{
				if(0 == cnt)
				{

					fread(inbuf, 1, 4, fp);
				}
							
				RGB[0] = 1.164 * (inbuf[cnt] - 16) + 1.569 * (inbuf[3] - 128);
				RGB[1] = 1.164 * (inbuf[cnt] - 16) - 0.391 * (inbuf[1] - 128) - 0.813 * (inbuf[3] - 128); 
				RGB[2] = 1.164 * (inbuf[cnt] - 16) + 2.018 * (inbuf[1] - 128);
				for(k = 0; k < 3; k++)
				{
					if(RGB[k] < 0)
					{
						RGB[k] = 0;
					}
					else if(RGB[k] > 255)
					{
						RGB[k] = 255;
					}
				}
				
				temp[0] = (unsigned char)RGB[0] >> 3;
				temp[1] = (unsigned char)RGB[1] >> 2;
				temp[2] = (unsigned char)RGB[2] >> 3;
				tcolor = ((temp[0] <<11) | (temp[1] << 5) | temp[2]);
				if(4 <= (cnt += 2))
				{
					cnt = 0;
				}
			}
			/* 格納位置計算 */
			index = ((i+ox) * depth / DIV_BYTE) + (j+oy) * len ;
			/* 着色 */
			*((unsigned short *)(p + index)) = tcolor;
			
		}
	}
	fclose(fp);
	munmap(p,size);
	close(fd);
	return 0;
}