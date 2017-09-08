/*	�� ������ ������Ʈ
//	����: �ڵ���, �ȿ���, ������
//	����: ������
//	����: mp3player 
//	��: 
//			������ �÷��̾� ������ ���� ffmpeg �� arm �����Ϸ� ����
//			���� �Ϸ��� ���� MDS2450���� ���� ��� �Ҹ��� ����� ������ ��������,
//			������ ������ ���� 30frame ���� 1�� ����� 30�� �̻��� �ð��� �ʿ��� ��Ȳ Ȯ��
//			SDL2(simple directmedia layout)�� �����ص� ū ��������� ��밡 �ȵǰ� 
//			SDL2 ���� ���̵��� �����ϰ� ���� mp3 player �������� ���� ��ȸ
//			mp3 player�� ���ϴ¸�ŭ�� �����ս��� ���̸� �ٽ� ���� �� ��ȹ
//
//	����:
//			tslib(Touch Screen Library)�� ��ġ��ũ�� ��� 					[���� �Ϸ�]
//			fb(framebuffer)�� UI ����										[���� �Ϸ�]
//			madplayer�� system �Լ��� ����ؼ� ��� ����					[���� �Ϸ�]
//			�÷��� ����Ʈ 													[���� �Ϸ�]
//			Previous, Next ��ư ��� ����									[���� �Ϸ�]
//			������� �뷡 �ѷ��ֱ� 											[���� �Ϸ�]
//			�����ư, �Ͻ����� ��ư ����Ī									[���� �Ϸ�]
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <dirent.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/fb.h>
#include <tslib.h>

#include "bmp.h"
#include "fbutils.h"

#define FBDEVFILE "/dev/fb0"

#define	COLS	480
#define	ROWS	272

#define MAX_CNT_MAX 10
#define MAX_CNT_MIN 5

/*            ����ü ����                  */
struct Music{
	char name[30][40];
	int cnt;
};	

/*            �Լ� ����                  */

// 24bit BMP ���� �޸𸮿� ����
void read_bmp(char *filename, char **pDib, char **data, int *cols, int *rows);

// BMP �޸𸮸� FB �޸𸮿� �޸� ����
void Draw_Lcd(int x, int y, char argv[]);

// �ҷ��� BMP ���� �޸� ����
void close_bmp(char **pDib);

// ��ư ��ġ üũ�� ���� ���� �������� ����
int circle(int c_x,int c_y,int x, int y, int r);

//�÷��� ����Ʈ ����
void make_list();

//�÷��� ����Ʈ ���
void print_list();

int main(void)
{	
	//�ʱ�ȭ ����
	//��ġ ��ũ�� �ʱ�ȭ
	struct tsdev *ts;
	char *tsdevice=NULL;
	// TSLIB_TSDEVICE�� ��ġ��ũ�� DD(Device Driver)�� ���� �ص�
	if( (tsdevice = getenv("TSLIB_TSDEVICE")) != NULL )			
	{
		ts = ts_open(tsdevice,0);
	} 
	else 
	{
		// ���� �صа� ������ ���� ��ġ��ũ�� DD ����
		if (!(ts = ts_open("/dev/input/event0", 0)))			
			ts = ts_open("/dev/touchscreen/ucb1x00", 0);
	}

	if (!ts) { //��ġ��ũ�� ���� �Ұ�, ���� �޼��� �� ����
		perror("ts_open"); exit(1); }

	if (ts_config(ts)) { //��ġ��ũ�� ���� �̻�, ���� �޼��� �� ����
		perror("ts_config"); exit(1); }
			
	if(open_framebuffer()) { //ȭ�鿡 ���ڸ� �Ѹ��� ���� �Լ�
		close_framebuffer(); exit(1); }
		
	int hTTY =open("/dev/tty",O_WRONLY|O_NONBLOCK);
	
	//�ʱ�ȭ �Ϸ�
	int max_cnt=MAX_CNT_MAX;
	int i,j, play_chk=0;
	
	Draw_Lcd(0,0,"image/startscreen.bmp");
	usleep(2000000);
	Draw_Lcd(0,0,"image/background.bmp"); // ���ȭ�� �ε�
	
	char play_buf[255]; //�÷��̸� �ɼ� �� ����� ������ ���� ����
	memset(play_buf,0,100*sizeof(char)); //���� �ʱ�ȭ

	sprintf(play_buf,"madplay -o /dev/dsp -q -v --downsample -A -10 --tty-control music/* &\n");
	system(play_buf);
	usleep(500000);
	ioctl(hTTY,TIOCSTI,"s");
	
	struct Music mu={0};
	make_list(&mu); // �÷��̸���Ʈ�� ���ۿ� ����

	while (1)  					//��ü ���� ������ ���α׷� ����
	{	
		struct ts_sample samp;	//tslib ����ü ��ġ��ũ���� ���� ������
		int ret;	//��ġ ���ϰ� ����
		
		print_list(&mu); //���ۿ� ��� �÷��̸���Ʈ�� ȭ�鿡 ���
		
		ret = ts_read(ts, &samp, 1);	// ��ġ�� ��ٸ���

		if (ret < 0){
			perror("ts_read"); exit(1);}
		
		if (ret != 1) continue;
		
		if(samp.pressure==0) //��ġ��ũ�� ������
		{	
			print_list(&mu);
			max_cnt=MAX_CNT_MAX;
			
			if(circle(samp.x,samp.y,55,234,26))
			{ // <
				Draw_Lcd(29,208,"image/butt/b_prev.bmp");
				ioctl(hTTY,TIOCSTI,"b");
			}
			else if(circle(samp.x,samp.y,128,234,26))
			{ // play
				if(play_chk==0) 
				{
					Draw_Lcd(102,208,"image/butt/b_pause.bmp");
					play_chk=1;
				}
				else if(play_chk==1)
				{
					Draw_Lcd(102,208,"image/butt/b_play.bmp");
					play_chk=0;
				}
				ioctl(hTTY,TIOCSTI,"p");									
			}
			else if(circle(samp.x,samp.y,200,234,26))
			{ // stop
				Draw_Lcd(174,208,"image/butt/b_stop.bmp");
				ioctl(hTTY,TIOCSTI,"s");
				play_chk=0;
				Draw_Lcd(102,208,"image/butt/b_play.bmp");
			}
			else if(circle(samp.x,samp.y,272,234,26))
			{ // >
				Draw_Lcd(246,208,"image/butt/b_next.bmp");
				ioctl(hTTY,TIOCSTI,"f");				
			}
			else if(circle(samp.x,samp.y,346,234,26))
			{ // +
				Draw_Lcd(320,208,"image/butt/b_plus.bmp");
				for(j=0;j<4;j++)
					ioctl(hTTY,TIOCSTI,"+");
			}
			else if(circle(samp.x,samp.y,418,234,26))
			{ // -
				Draw_Lcd(392,208,"image/butt/b_minus.bmp");
				for(j=0;j<4;j++)
					ioctl(hTTY,TIOCSTI,"-");				
			}
			else if(circle(samp.x,samp.y,67,104,44))
			{ // all is well
				ioctl(hTTY,TIOCSTI,"q");
				system("killall madplay");
				close(hTTY);
				break;				
			}
		}//end if
		else if(samp.pressure == 255) //��ġ��ũ�� ��������
		{	
			print_list(&mu);
			
			if(circle(samp.x,samp.y,55,234,26))
			{ // <
				Draw_Lcd(29,208,"image/butt/w_prev.bmp");
			}
			else if(circle(samp.x,samp.y,128,234,26))
			{ // play
				if(play_chk==0) Draw_Lcd(102,208,"image/butt/w_play.bmp");
				else if(play_chk==1) Draw_Lcd(102,208,"image/butt/w_pause.bmp");
			}
			else if(circle(samp.x,samp.y,200,234,26))
			{ //stop
				Draw_Lcd(174,208,"image/butt/w_stop.bmp");
			}
			else if(circle(samp.x,samp.y,272,234,26))
			{ // >
				Draw_Lcd(246,208,"image/butt/w_next.bmp");
			}
			else if(circle(samp.x,samp.y,346,234,26))//75 234 26
			{ // +
				Draw_Lcd(320,208,"image/butt/w_plus.bmp");
				if(max_cnt==j++)
				{
					ioctl(hTTY,TIOCSTI,"+");
					if(max_cnt>MAX_CNT_MIN) max_cnt--;
					j=0;
				}
			}
			else if(circle(samp.x,samp.y,418,234,26))
			{ // -
				Draw_Lcd(392,208,"image/butt/w_minus.bmp");
				if(max_cnt==j++)
				{
					ioctl(hTTY,TIOCSTI,"-");					
                    if(max_cnt > MAX_CNT_MIN) max_cnt--;
					j=0;
				}
			}
			
		}
				
	}	
	close_framebuffer();
	ts_close(ts);
	return 0;
}

void Draw_Lcd(int x, int y, char argv[])
{
	char r,g,b;
	int cols, rows;
	char *pData,*data;
	
	unsigned short bmpdata1[ROWS][COLS];
	unsigned short pixel;
	
	unsigned short *pfbmap;
	struct fb_var_screeninfo fbvar;
	int fbfd;
	int i,j,k;
		
	fbfd = open(FBDEVFILE, O_RDWR); //������ ���� DD ����
	if(fbfd < 0){ perror("fbdev open"); exit(1); } //error
	
	/* mmap �Լ��� �̿��Ͽ� flame buffer�� �޸��� ���� �ּҸ� ����� */
	/* 1 pixel�� 2byte�� �ʿ��ϹǷ� *2 */
	pfbmap = (unsigned short *)
		mmap(0, COLS*ROWS*2,PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0);
	if((unsigned)pfbmap == (unsigned)-1){ perror("fbdev mmap"); exit(1); }

	/*  bmp������ �о� �帲 */	
	read_bmp(argv, &pData, &data, &cols,&rows);
		
	for(j=0;j<rows;j++)
	{
		k = j*cols*3;  //�� �ȼ��� 3byte�ΰ��� 2byte�� �ٲ�
		//printf("cols = %d, rows = %d, k = %d\n", cols,rows, k);

		for(i=0;i<cols;i++)
		{ 
			b = *(data + (k + i*3));
			g = *(data + (k + i*3+1));
			r = *(data + (k + i*3+2));
			
			pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
		
			bmpdata1[(rows-1-j+y)][i+x] = pixel;
		}
	} 
/*  bmp ���Ͽ��� ���� data�� ������ ������ �޸𸮷� ī�� �� */
	memcpy(pfbmap,bmpdata1, COLS*ROWS*2);
	
	munmap(pfbmap,ROWS*COLS*2); //mmap ����
	close_bmp(&pData); //�޸� ����
	close(fbfd); //fb device driver ����
}
/* bmp���Ͽ��� �׸� ������ �����͸� �̾� ���� �Լ� */
void read_bmp(char *filename, char **pDib, char **data, int *cols, int *rows)
{
	BITMAPFILEHEADER bmpHeader;
	BITMAPINFOHEADER *bmpInfoHeader;
	unsigned int size;
	unsigned char ID[2];
	int nread;
	FILE *fp;
	
	fp = fopen(filename,"rb");
	if(fp == NULL) { printf("ERROR\n"); return; }
	
	ID[0] = fgetc(fp);
	ID[1] = fgetc(fp);
	
	/*  BMP ��ũ�� Ȯ�� */
	if(ID[0] != 'B' && ID[1] != 'M') { fclose(fp); return; }

	/* bmp ���Ͽ��� BITMAPFILEHEADER ��ŭ ���� */
	nread = fread(&bmpHeader.bfSize,1,sizeof(BITMAPFILEHEADER),fp);
	size = bmpHeader.bfSize - sizeof(BITMAPFILEHEADER);	

	/* ������ ���� size��ū �޸𸮸� ��� �Ͼ pDib�� ����*/
	*pDib = (unsigned char *)malloc(size);	
	fread(*pDib,1,size,fp);
	
	/* ������ ���� pDib���� BITMAPINFOHEADER�� ������ */
	bmpInfoHeader = (BITMAPINFOHEADER *)*pDib;

	/* 24��Ʈ true Į�� �ֶ��� ��� �� �� �ֵ��� �� */
	if(24 != bmpInfoHeader->biBitCount)
	{
		printf("It supports only 24bit bmp!\n");
		fclose(fp);
		return;
	}
	/* BITMAPINFOHEADER���� ���ο� ���� �������� ���� */
	*cols = bmpInfoHeader->biWidth;
	*rows = bmpInfoHeader->biHeight;
	
	/* ���� �������� �ּҸ� ������ */
	/* ó�� ��ġ���� �����Ͱ� ��ġ�� �� ������ offset�̹Ƿ� bmpHeader ��ŭ ���ְ�, �ռ� signature�� �����鼭 �Һ��� 2����Ʈ�� ���־�� ��. */
	*data = (char *)(*pDib + bmpHeader.bfOffBits - sizeof(bmpHeader)-2);
	
	fclose(fp); // ���� �ݱ�
}

int circle(int c_x,int c_y,int x, int y, int r)
{
	return ((c_x-x)*(c_x-x) + (c_y-y)*(c_y-y) < r*r);
}

void close_bmp(char **pDib)
{	
	free(*pDib);
}

void make_list(struct Music *mu)
{	
	mu->cnt=0;
	int i;
	char trash[5];
	
	FILE*tl=fopen("list.txt","r");
	fscanf(tl,"%d\n",&mu->cnt);
	for(i=0;i<mu->cnt;i++)
	{
		fscanf(tl,"music/%[^\n]s",&mu->name[i]);
		fscanf(tl,"\n",&trash);
	}
	fclose(tl); 
}

void print_list(struct Music *mu)
{
	int i;
	setcolor(1,0x00000000); //���� ��Ʈ �� ����
	for(i=0;i<mu->cnt;i++)
		Lcd_Puts(170,35+i*14,1,mu->name[i],1,1);
}
