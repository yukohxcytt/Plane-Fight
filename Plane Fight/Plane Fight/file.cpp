#include<stdio.h>
#include<easyx.h>
#include<time.h>
#include<stdlib.h>
#include<math.h>
#include<windows.h>
#include<conio.h>
//引入头文件
#pragma comment(lib,"Winmm.lib")
//引入库文件，播放背景音乐
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 800
#define PLANE_SIZE 50
#define ENEMY_NUM 8
#define ENEMY_INTERVAL 1.0
#define BULLET_NUM 10
#define BULLET_SIZE 10
//定义宏屏幕宽跟高，飞机大小，敌机数量跟速度，还有子弹的数量

typedef struct {
	int x;
	int y;
}position;
//位置

typedef struct
{
	position planePos;
	position planeBullets[BULLET_NUM];
	int bulletLen;
	int bulletSpeed;

}PLANE;

//飞机具有的属性，飞机位置，子弹位置，子弹数量跟子弹速度

void initGame();
void drawGame();
void updateGame();
void initEnemyPlane();
void destroyEnemy();
int innersect(position, position, int);
void destroyBullet();
//函数声明

PLANE myPlane;

PLANE enemyPlane[ENEMY_NUM];

int enemyPlaneLen;

static time_t startTime, endTime;

IMAGE img[4];
int score = 0;
int main()
{
	loadimage(&img[0], "img/res/背景.jpg", SCREEN_WIDTH, SCREEN_HEIGHT);
	loadimage(&img[1], "img/res/敌机.jpg", PLANE_SIZE, PLANE_SIZE);
	loadimage(&img[2], "img/res/飞机.jpg", PLANE_SIZE, PLANE_SIZE);
	loadimage(&img[3], "img/res/子弹.jpg",BULLET_SIZE,BULLET_SIZE);
	//导入四张图
	initGame();
	
	while (1)
	{	drawGame();
		updateGame();
		initEnemyPlane();
		destroyEnemy();
		destroyBullet();
		Sleep(1000 / 60);
	}
	getchar();
	return 0;
}
void initGame()//初始化函数
{
	initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
	score = 0;
	srand((unsigned)time(NULL)); //随机数种子
	myPlane.bulletLen = 0;
	myPlane.bulletSpeed = 6;
	myPlane.planePos = {SCREEN_WIDTH/2,SCREEN_HEIGHT - PLANE_SIZE / 2 };//让飞机的初始位置在中间
	enemyPlaneLen = 0;
	startTime = time(NULL);

}

void drawGame()
{
	BeginBatchDraw();//防止图片抖动
	putimage(0, 0, &img[0]);
	putimage(myPlane.planePos.x - PLANE_SIZE / 2, myPlane.planePos.y - PLANE_SIZE / 2,&img[2],SRCAND);//SRCAND是去除图片白边

	for (int i = 0;i < enemyPlaneLen;i++)
	{
		putimage(enemyPlane[i].planePos.x - PLANE_SIZE / 2, enemyPlane[i].planePos.y - PLANE_SIZE / 2, &img[1],SRCAND);//贴图敌机
	}

	for (int n = 0;n < myPlane.bulletLen;n++)
	{
		putimage(myPlane.planeBullets[n].x-BULLET_SIZE/2, myPlane.planeBullets[n].y - BULLET_SIZE / 2, &img[3], SRCAND);//贴图子弹
	}

	RECT rect = { 0,PLANE_SIZE,SCREEN_WIDTH,SCREEN_HEIGHT };//绘制一个覆盖下面的矩形
	setbkmode(TRANSPARENT);//去掉文字本身的背景色
	char str[30] = { 0 };//保存字符串
	sprintf(str, "分数：%d", score);//将双引号里面的东西转换成字符串
	drawtext(str, &rect, DT_TOP | DT_CENTER);//在矩形顶部居中的位置写字
	EndBatchDraw();
}

void updateGame()
{
	if (GetAsyncKeyState('W') & 0x8000 && myPlane.planePos.y - PLANE_SIZE / 2 >= 0)
	{
		myPlane.planePos.y -= 4;
	}
	if (GetAsyncKeyState('S') & 0x8000 && myPlane.planePos.y + PLANE_SIZE / 2 <= SCREEN_HEIGHT)
	{
		myPlane.planePos.y += 4;
	}
	if (GetAsyncKeyState('A') & 0x8000 && myPlane.planePos.x - PLANE_SIZE /2 >= 0)
	{
		myPlane.planePos.x -= 4;
	}
	if (GetAsyncKeyState('D') & 0x8000 && myPlane.planePos.x + PLANE_SIZE / 2 <= SCREEN_WIDTH)
	{
		myPlane.planePos.x += 4;
	}
	//自机移动+限制边界
	static bool spaceHeld = false;//记录上一帧的状态
	bool spaceNow = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;//这一帧的状态，要是按下去的就是1
	if (spaceNow && !spaceHeld && myPlane.bulletLen < BULLET_NUM)//如果上一帧没按且当前按了，而且小于子弹容量的话就发射
	{
			PlaySound("img/res/shoot_a.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NOWAIT);
			myPlane.planeBullets[myPlane.bulletLen] = myPlane.planePos;
			myPlane.bulletLen++;
	}
	spaceHeld = spaceNow; 

	for (int i = 0;i < enemyPlaneLen;i++)//敌机的移动
	{
		enemyPlane[i].planePos.y += 2;
	}

	for (int i = 0;i < myPlane.bulletLen;i++)//子弹的移动
	{
		myPlane.planeBullets[i].y -= myPlane.bulletSpeed;
	}
	
	int live = 0;
	
	for (int i = 0;i < myPlane.bulletLen;i++)//子弹回收
	{
		if (myPlane.planeBullets[i].y + BULLET_SIZE / 2 >= 0)
		{
			myPlane.planeBullets[live] = myPlane.planeBullets[i];
			live++;
		}
		
	}
	myPlane.bulletLen = live;

}

void initEnemyPlane()
{
	endTime = time(NULL);
	double elapsedTime = difftime(endTime, startTime);//计算时间差
	if (elapsedTime >= ENEMY_INTERVAL && enemyPlaneLen < ENEMY_NUM)
	{
		int x = (rand() % (SCREEN_WIDTH - 2 * PLANE_SIZE)) + PLANE_SIZE;//让敌机在中间区域刷新
		int y = -PLANE_SIZE;
		enemyPlane[enemyPlaneLen].planePos.x = x;
		enemyPlane[enemyPlaneLen].planePos.y = y;
		enemyPlaneLen++;
		startTime = endTime;
	}
}

void destroyEnemy()//回收敌机
{
	int alive = 0;
	for (int i = 0;i < enemyPlaneLen;i++)//回收超过边界的飞机
	{
		if (enemyPlane[i].planePos.y - PLANE_SIZE / 2 <= SCREEN_HEIGHT)
		{
			enemyPlane[alive].planePos = enemyPlane[i].planePos;
			alive++;
		}

		if (innersect(myPlane.planePos, enemyPlane[i].planePos, PLANE_SIZE))
		{
			if (MessageBox(GetHWnd(), "游戏结束，是否需要继续游戏", "提示", MB_YESNO)==IDYES)//弹出一个窗口
			{
				initGame();
				return;/*这个函数的作用是结束这个函数，如果不写这个会出现的话，会出现你撞击了第一架飞机，然后后面的飞机会刷新，第一架飞机不会刷新的情况，
				那是因为下面的赋值会照常进行，initGame函数让敌机数量归零之后，enemyPlaneLen的值又被alive的值覆盖了*/
			}
			else 
			{
				exit(0);//结束程序
			}
		}
	}
	enemyPlaneLen = alive;//告诉游戏还有多少架飞机
}


int innersect(position c1, position c2, int radius)//判断飞机是否相撞
{
	return abs(c1.x - c2.x) <= radius && abs(c1.y - c2.y) <= radius;
}

void destroyBullet()
{
	for (int i = 0;i < enemyPlaneLen;i++)//将每一架敌机都跟每一颗子弹比对
	{
		for (int j = 0;j < myPlane.bulletLen;j++)
		{
			if (innersect(enemyPlane[i].planePos, myPlane.planeBullets[j], BULLET_SIZE / 2 + PLANE_SIZE / 2))//判定
			{
				for (int x = j;x < myPlane.bulletLen-1;x++)
				{
					myPlane.planeBullets[x] = myPlane.planeBullets[x + 1];
				}

				for (int x = i;x < enemyPlaneLen - 1;x++)
				{
					enemyPlane[x] = enemyPlane[x + 1];
				}
				myPlane.bulletLen--;
				enemyPlaneLen--;
				score += 100;
				i--;//万一两架飞机贴着，如果不减的话会漏掉一架
				break;//打中后后面的子弹不用审查了
				
			}
		}
	}
}