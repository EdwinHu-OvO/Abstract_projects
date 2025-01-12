#include<iostream>
#include<conio.h>
#include<Windows.h>
#include"struct.h"
#include<string>
using namespace std;
extern Env env;
extern Input input;
void getAcross(bool red, bool blue, bool star, bool LED);
void printTitle()
{
	string art[] = {
		"                            _                          ",
		" |\\ |  _  |_   _   _|      |_    ._  |  _   _|  _   _  ",
		" | \\| (_) |_) (_) (_| \\/   |_ >< |_) | (_) (_| (/_ _>  ",
		"                _     /          |                     ",
		"  _|      _    /   _  ._ _  ._  |  _    \\    / o ._ _  ",
		" (_| |_| (/_   \\_ (_) | | | |_) | (/_ >< \\/\\/  | | (/_ ",
		"                            |                          "
    };
	cout << endl;
	for (const auto& line : art) {
		cout << line << endl;
	}
}
void zeroXy(int x, int y)
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(handle, pos);
}
void HideCursor()
{
	CONSOLE_CURSOR_INFO cursor_info = { 1, 0 };
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);
}
int control(){
	while (_kbhit())
	{
		int key = _getch();
		if (key == 224 || key == 0) { // 特殊按键的前缀码（方向键）
			key = _getch(); // 获取方向键的具体值

			switch (key) {
			case 72:
				return -1;
			case 80:
				return 1;
			case 75:
				return -1;
			case 77:
				return 1;
			}
		}
		else{
			switch (key){
			case 'd':
				return 1;
			case 'a':
				return -1;
			case 'w':
				return -1;
			case 's':
				return 1;
			case ' ':
				return -9;
			case 13:
				return -9;
			default:
				return 0;
			}
		}
	}
	return 0;
}
void menu(int l, int sec){
	if (l == 1){
		zeroXy(0, 0);
		printTitle();
		if (sec == 1){
			cout << ">Parallel          ";
			if (env.p) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " Parallel          ";
			if (env.p) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		if (sec == 2){
			cout << ">Batt >= 2         ";
			if (env.b) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " Batt >= 2         ";
			if (env.b) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		if (sec == 3){
			cout << ">Serial end double ";
			if (env.d) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " Serial end double ";
			if (env.d) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		cout << endl;
		if (sec == 4){
			cout << "                  >[next]<";
		}
		else{
			cout << "                   [next] ";
		}
	}
	else if (l == 2){
		zeroXy(0, 0);
		printTitle();
		if (sec == 1){
			cout << ">red              ";
			if (input.red) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " red              ";
			if (input.red) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		if (sec == 2){
			cout << ">blue             ";
			if (input.blue) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " blue             ";
			if (input.blue) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		if (sec == 3){
			cout << ">star             ";
			if (input.star) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " star             ";
			if (input.star) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		if (sec == 4){
			cout << ">LED?             ";
			if (input.LED) cout << "[True]< \n";
			else cout << "[False]<\n";
		}
		else{
			cout << " LED?             ";
			if (input.LED) cout << "[True]  \n";
			else cout << "[False] \n";
		}
		cout << endl;
		if (sec == 5){
			cout << "  >[next]<    [结束模块]       \n";
		}
		else if (sec == 6){
			cout << "   [next]    >[结束模块]<      \n";
		}
		else{
			cout << "   [next]     [结束模块]       \n";
		}
	}
}
bool setval(bool b, int& i){
	if (b == 0 && i == -9){
		i = 0;
		return 1;
	}
	else if (b == 1 && i == -9){
		i = 0;
		return 0;
	}
	return b;
}
bool print(int a){
	int sec = 1;
	if (a == 1){
		sec = 1;
		while (true){
			if (sec == 1){
				menu(1, sec);
				int tmp = control();
				env.p=setval(env.p, tmp);
				sec += tmp;
				if (sec > 4)sec = 4;
				if (sec < 1)sec = 1;
			}
			if (sec == 2){
				menu(1, sec);
				int tmp = control();
				env.b = setval(env.b, tmp);
				sec += tmp;
				if (sec > 4)sec = 4;
				if (sec < 1)sec = 1;
			}
			if (sec == 3){
				menu(1, sec);
				int tmp = control();
				env.d = setval(env.d, tmp);
				sec += tmp;
				if (sec > 4)sec = 4;
				if (sec < 1)sec = 1;
			}if (sec == 4){
				menu(1, sec);
				int tmp = control();
				if (tmp == -9){
					system("cls");
					break;
				}
				sec += tmp;
				if (sec > 4)sec = 4;
				if (sec < 1)sec = 1;
			}
		}
	}
	else if (a == 2){
		sec = 1;
		while (true){
			if (sec == 1){
				menu(2, sec);
				int tmp = control();
				input.red = setval(input.red, tmp);
				sec += tmp;
				if (sec > 6)sec = 6;
				if (sec < 1)sec = 1;
			}
			if (sec == 2){
				menu(2, sec);
				int tmp = control();
				input.blue = setval(input.blue, tmp);
				sec += tmp;
				if (sec > 6)sec = 6;
				if (sec < 1)sec = 1;
			}
			if (sec == 3){
				menu(2, sec);
				int tmp = control();
				input.star = setval(input.star, tmp);
				sec += tmp;
				if (sec > 6)sec = 6;
				if (sec < 1)sec = 1;
			}
			if (sec == 4){
				menu(2, sec);
				int tmp = control();
				input.LED = setval(input.LED, tmp);
				sec += tmp;
				if (sec > 6)sec = 6;
				if (sec < 1)sec = 1;
			}
			if (sec == 5){
				menu(2, sec);
				int tmp = control();
				if (tmp == -9){
					getAcross(input.red, input.blue, input.star, input.LED);
					system("cls");
					return 1;
				}
				sec += tmp;
				if (sec > 6)sec = 6;
				if (sec < 1)sec = 1;
			}
			if (sec == 6){
				menu(2, sec);
				int tmp = control();
				if (tmp == -9){
					system("cls");
					return 0;
				}
				sec += tmp;
				if (sec > 6)sec = 6;
				if (sec < 1)sec = 1;
			}
		}
	}
	return 0;
}