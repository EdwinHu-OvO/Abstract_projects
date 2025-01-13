#include<iostream>
#include"struct.h"
using namespace std;
void control();
bool print(int);
void HideCursor();
void zeroXy(int x, int y);
void printTitle();
char m[4][4] = {
	{ 'C', 'S', 'P', 'D' },
	{ 'S', 'S', 'S', 'B' },
	{ 'C', 'P', 'D', 'B' },
	{ 'C', 'D', 'P', 'B' }
};
Node* node;
Input input;
Env env;
int getmax(int a, int b)
{
	int i;
	a > b ? i = a : i = b;
	return i;
}
int getmin(int a, int b)
{
	int i;
	a < b ? i = a : i = b;
	return i;
}
void getAcross(bool red, bool blue, bool star, bool LED)
{
	int x1=0, x2=3, y1=0, y2=3;
	if (red){
		x1 = getmax(x1, 0);
		x2 = getmin(x2, 3);
		y1 = getmax(y1, 1);
		y2 = getmin(y2, 2);
	}
	if (blue){
		x1 = getmax(x1, 1);
		x2 = getmin(x2, 2);
		y1 = getmax(y1, 0);
		y2 = getmin(y2, 3);
	}
	if (star){
		x1 = getmax(x1, 0);
		x2 = getmin(x2, 3);
		y1 = getmax(y1, 2);
		y2 = getmin(y2, 3);
	}
	if (LED){
		x1 = getmax(x1, 2);
		x2 = getmin(x2, 3);
		y1 = getmax(y1, 0);
		y2 = getmin(y2, 4);
	}
	else if (!(red || blue || star || LED))
		x1 = x2 = y1 = y2 = 0;

	int s = (x2+1 - x1)*(y2+1 - y1);
	node = new Node [sizeof(Node)*s];

	for (int n = 0; n <= s-1;){
		for (int i = x1; i <= x2; i++){
			for (int j = y1; j <= y2; j++){
				node[n].val = m[j][i];
				node[n].loc[0] = i;
				node[n].loc[1] = j;
				n++;
			}
		}
	}
}
void isCut(Node* node,Env env)
{
	int flag = 0;
	for (int i = 0; i <= sizeof(node); i++)
	{
		if (node[i].val == 'B'){
			if (env.b){
				cout << "Cut!!";
				flag = 1;
				break;
			}
			else break;
		}
	}
	if (!flag){
		for (int i = 0; i <= sizeof(node); i++)
		{
			if (node[i].val == 'P'){
				if (env.p){
					cout << "Cut!!";
					flag = 1;
					break;
				}
				else break;
			}
		}
	}
	if (!flag){
		for (int i = 0; i <= sizeof(node); i++)
		{
			if (node[i].val == 'S'){
				if (env.d){
					cout << "Cut!!";
					flag = 1;
					break;
				}
				else break;
			}
		}
	}
	if (!flag){
		for (int i = 0; i <= sizeof(node); i++)
		{
			if (node[i].val == 'D'){
				cout << "NOT Cut!!";
				flag = 1;
				break;
			}
		}
	}
	if (!flag){
		cout << "Cut!!";
	}
	system("pause");
	system("cls");
}
int main()
{
	HideCursor();
	while (true){
		input.blue = input.red = input.star = input.LED = 0;
		env.b = env.d = env.p = 0;
		print(1);
		while (print(2)){
			isCut(node, env);
			//cout << env.b << env.p << env.d;
			for (int n = 0; n <= 15;){
				cout << node[n].val;
						n++;
			}
			input.blue = input.red = input.star = input.LED = 0;
			delete[] node;
		}
	}
	return 0;
}