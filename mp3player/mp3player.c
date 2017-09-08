/*	팀 세얼간이 프로젝트
//	팀원: 박동준, 안용현, 이형민
//	조장: 이형민
//	주제: mp3player 
//	상세: 
//			동영상 플레이어 제작을 위해 ffmpeg 을 arm 컴파일로 포팅
//			포팅 완료후 보드 MDS2450에서 실행 결과 소리는 재생에 문제가 없었으나,
//			보드의 성능이 낮아 30frame 영상 1초 재생에 30초 이상의 시간이 필요한 상황 확인
//			SDL2(simple directmedia layout)를 적용해도 큰 성능향상이 기대가 안되고 
//			SDL2 포팅 난이도가 현저하게 높아 mp3 player 제작으로 방향 선회
//			mp3 player가 원하는만큼의 퍼포먼스를 보이면 다시 도전 할 계획
//
//	제작:
//			tslib(Touch Screen Library)로 터치스크린 사용 					[적용 완료]
//			fb(framebuffer)에 UI 생성										[적용 완료]
//			madplayer를 system 함수를 사용해서 기능 구현					[적용 완료]
//			플레이 리스트 													[적용 완료]
//			Previous, Next 버튼 기능 구현									[적용 완료]
//			재생중인 노래 뿌려주기 											[적용 완료]
//			재생버튼, 일시정지 버튼 스위칭									[적용 완료]
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

/*            구조체 선언                  */
struct Music{
	char name[30][40];
	int cnt;
};	

/*            함수 선언                  */

// 24bit BMP 파일 메모리에 적재
void read_bmp(char *filename, char **pDib, char **data, int *cols, int *rows);

// BMP 메모리를 FB 메모리에 메모리 복사
void Draw_Lcd(int x, int y, char argv[]);

// 불러온 BMP 파일 메모리 해제
void close_bmp(char **pDib);

// 버튼 터치 체크를 위해 원의 방정식을 구현
int circle(int c_x,int c_y,int x, int y, int r);

//플레이 리스트 생성
void make_list();

//플레이 리스트 출력
void print_list();

int main(void)
{	
	//초기화 시작
	//터치 스크린 초기화
	struct tsdev *ts;
	char *tsdevice=NULL;
	// TSLIB_TSDEVICE에 터치스크린 DD(Device Driver)를 정의 해둠
	if( (tsdevice = getenv("TSLIB_TSDEVICE")) != NULL )			
	{
		ts = ts_open(tsdevice,0);
	} 
	else 
	{
		// 정의 해둔게 없을때 직접 터치스크린 DD 오픈
		if (!(ts = ts_open("/dev/input/event0", 0)))			
			ts = ts_open("/dev/touchscreen/ucb1x00", 0);
	}

	if (!ts) { //터치스크린 오픈 불가, 에러 메세지 후 종료
		perror("ts_open"); exit(1); }

	if (ts_config(ts)) { //터치스크린 설정 이상, 에러 메세지 후 종료
		perror("ts_config"); exit(1); }
			
	if(open_framebuffer()) { //화면에 글자를 뿌리기 위한 함수
		close_framebuffer(); exit(1); }
		
	int hTTY =open("/dev/tty",O_WRONLY|O_NONBLOCK);
	
	//초기화 완료
	int max_cnt=MAX_CNT_MAX;
	int i,j, play_chk=0;
	
	Draw_Lcd(0,0,"image/startscreen.bmp");
	usleep(2000000);
	Draw_Lcd(0,0,"image/background.bmp"); // 배경화면 로드
	
	char play_buf[255]; //플레이를 옵션 및 재생곡 저장을 위한 버퍼
	memset(play_buf,0,100*sizeof(char)); //버퍼 초기화

	sprintf(play_buf,"madplay -o /dev/dsp -q -v --downsample -A -10 --tty-control music/* &\n");
	system(play_buf);
	usleep(500000);
	ioctl(hTTY,TIOCSTI,"s");
	
	struct Music mu={0};
	make_list(&mu); // 플레이리스트를 버퍼에 저장

	while (1)  					//전체 루프 나가면 프로그램 종료
	{	
		struct ts_sample samp;	//tslib 구조체 터치스크린에 관한 데이터
		int ret;	//터치 리턴값 저장
		
		print_list(&mu); //버퍼에 담긴 플레이리스트를 화면에 출력
		
		ret = ts_read(ts, &samp, 1);	// 터치를 기다린다

		if (ret < 0){
			perror("ts_read"); exit(1);}
		
		if (ret != 1) continue;
		
		if(samp.pressure==0) //터치스크린 땟을때
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
		else if(samp.pressure == 255) //터치스크린 눌렀을때
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
		
	fbfd = open(FBDEVFILE, O_RDWR); //프레임 버퍼 DD 오픈
	if(fbfd < 0){ perror("fbdev open"); exit(1); } //error
	
	/* mmap 함수를 이용하여 flame buffer의 메모리의 가상 주소를 얻었냄 */
	/* 1 pixel당 2byte가 필요하므로 *2 */
	pfbmap = (unsigned short *)
		mmap(0, COLS*ROWS*2,PROT_READ|PROT_WRITE, MAP_SHARED, fbfd, 0);
	if((unsigned)pfbmap == (unsigned)-1){ perror("fbdev mmap"); exit(1); }

	/*  bmp파일을 읽어 드림 */	
	read_bmp(argv, &pData, &data, &cols,&rows);
		
	for(j=0;j<rows;j++)
	{
		k = j*cols*3;  //한 픽셀에 3byte인것을 2byte로 바꿈
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
/*  bmp 파일에서 읽은 data를 프레임 버퍼의 메모리로 카피 함 */
	memcpy(pfbmap,bmpdata1, COLS*ROWS*2);
	
	munmap(pfbmap,ROWS*COLS*2); //mmap 해제
	close_bmp(&pData); //메모리 해제
	close(fbfd); //fb device driver 닫음
}
/* bmp파일에서 그림 정보와 데이터를 뽑아 내는 함수 */
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
	
	/*  BMP 마크를 확인 */
	if(ID[0] != 'B' && ID[1] != 'M') { fclose(fp); return; }

	/* bmp 파일에서 BITMAPFILEHEADER 만큼 읽음 */
	nread = fread(&bmpHeader.bfSize,1,sizeof(BITMAPFILEHEADER),fp);
	size = bmpHeader.bfSize - sizeof(BITMAPFILEHEADER);	

	/* 위에서 얻은 size만큰 메모리를 잡고 일어서 pDib에 넣음*/
	*pDib = (unsigned char *)malloc(size);	
	fread(*pDib,1,size,fp);
	
	/* 위에서 읽은 pDib에서 BITMAPINFOHEADER를 추출함 */
	bmpInfoHeader = (BITMAPINFOHEADER *)*pDib;

	/* 24비트 true 칼라 있때만 사용 할 수 있도록 함 */
	if(24 != bmpInfoHeader->biBitCount)
	{
		printf("It supports only 24bit bmp!\n");
		fclose(fp);
		return;
	}
	/* BITMAPINFOHEADER에서 가로와 세로 사이즈을 구함 */
	*cols = bmpInfoHeader->biWidth;
	*rows = bmpInfoHeader->biHeight;
	
	/* 실제 데이터의 주소를 가져옴 */
	/* 처음 위치부터 데이터가 위치한 곳 까지의 offset이므로 bmpHeader 만큼 빼주고, 앞서 signature를 읽으면서 소비한 2바이트도 빼주어야 함. */
	*data = (char *)(*pDib + bmpHeader.bfOffBits - sizeof(bmpHeader)-2);
	
	fclose(fp); // 파일 닫기
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
	setcolor(1,0x00000000); //글자 폰트 색 설정
	for(i=0;i<mu->cnt;i++)
		Lcd_Puts(170,35+i*14,1,mu->name[i],1,1);
}
