#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#define Y1 YUYV[0]
#define U  YUYV[1]
#define Y2 YUYV[2]
#define V  YUYV[3]

#define WIDTH (320)
#define HEIGHT (240)
int main(int argc, char **argv)
{

	// JPEGオブジェクト, エラーハンドラを確保
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int i, j;
	unsigned char YUYV[4];
	int R;
	int G;
	int B;

	// 出力ファイルのハンドラを確保、出力ファイル名を指定
	FILE *fp;
	FILE *fp_yuv;
	char *filename = argv[2];

	// 画像のサイズ
	int width = WIDTH;
	int height = HEIGHT;

	//エラーハンドラにデフォルト値を設定
	cinfo.err = jpeg_std_error(&jerr);

	// JPEGオブジェクトを初期化
	jpeg_create_compress(&cinfo);

	// 出力ファイルをオープン
	if ((fp = fopen(filename, "wb")) == NULL)
	{
		fprintf(stderr, "cannot open %s\n", filename);
		exit(EXIT_FAILURE);
	}

	// 出力ファイルを指定
	jpeg_stdio_dest(&cinfo, fp);

	// 画像のパラメータを設定
	cinfo.image_width = width;

	// 画像の幅
	cinfo.image_height = height;

	// 画像の高さ
	cinfo.input_components = 3;

	// 1pixelあたりのカラー要素数
	cinfo.in_color_space = JCS_RGB;

	// 入力画像の形式
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 75, TRUE);

	// 圧縮効率（0～100）
	// 圧縮を開始
	jpeg_start_compress(&cinfo, TRUE);


	if ((fp_yuv = fopen(argv[1], "rb")) == NULL)
	{
		fprintf(stderr, "cannot open %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	// データ（RGB値）を生成
	JSAMPARRAY img = (JSAMPARRAY)malloc(sizeof(JSAMPROW) * height);
	for (i = 0; i < height; i++)
	{
		img[i] = (JSAMPROW)malloc(sizeof(JSAMPLE) * 3 * width);
		for (j = 0; j < width; j++)
		{
			if((j % 2) == 0){
				fread(YUYV, 1, 4, fp_yuv);
				R = 1.164 * (Y1 - 16) + 1.569 * (V - 128);
				G = 1.164 * (Y1 - 16) - 0.391 * (U - 128) - 0.813 * (V - 128);
				B = 1.164 * (Y1 - 16) + 2.018 * (U - 128);
			}else{
				R = 1.164 * (Y2 - 16) + 1.569 * (V - 128);
				G = 1.164 * (Y2 - 16) - 0.391 * (U - 128) - 0.813 * (V - 128);
				B = 1.164 * (Y2 - 16) + 2.018 * (U - 128);
			}
			R = ((R < 0) ? 0 : ((R > 0xff) ? 0xff : R));
			G = ((G < 0) ? 0 : ((G > 0xff) ? 0xff : G));
			B = ((B < 0) ? 0 : ((B > 0xff) ? 0xff : B));
			img[i][j * 3 + 0] = (unsigned char)R;
			img[i][j * 3 + 1] = (unsigned char)G;
			img[i][j * 3 + 2] = (unsigned char)B;
		}
	}

	// 1行でなく画像全体を出力
	jpeg_write_scanlines(&cinfo, img, height);

	// 圧縮を終了
	jpeg_finish_compress(&cinfo);

	// JPEGオブジェクトを破棄
	jpeg_destroy_compress(&cinfo);

	for (i = 0; i < height; i++)
	{
		free(img[i]);
	}

	free(img);
	fclose(fp);
}
