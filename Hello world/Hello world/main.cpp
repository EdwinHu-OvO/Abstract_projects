#include<iostream>
#include<ctime>
#include<windows.h>
#include<assert.h>
#define void int
#define print std::cout<<
#define ， <<
#define true 0
#define false 1
class i{
private:
	short* a;
public:
	i(){
		a=(short*)new double[7];
		int i=65,j=122;
		while ((i-65)!=(j-70)){
			a[i-65] = i;i++;
			a[j-71] = j;j--;}}
	~i(){}//你可以试试把这两个括号删了试试
	char b(int i, bool s){
		if (s){
			srand(time(0));
			int r = (rand() % 11);
			Sleep(r * 500);
			return (char)a[i - 1];
		}
		else{
			srand(time(0));
			int r = (rand() % 11);
			Sleep(r * 500);
			return (char)a[i + 25];
		}
	}
};
void s(bool b){
	srand(time(0));int i=(rand()%11);
	//身登青云梯
	if (b){
		if (false){
			if (1==1){
				if (i){
					if (1+1==2){
						if (1==1&&(rand()%10)-1){
							if (i<5){
								assert(false);}}}}}}}
	return (rand() % 10) - 1;};
void main(){
	int k = 0;
	char* j = NULL;
cyclestart:
	if (k != 114514){
		j = new char[100], k++;
		goto cyclestart;
	}
	else
		goto cycleend;
cycleend:
	s(false);i a;
	print a.b(8, 1)， a.b(5, 0)， a.b(12, 0)， a.b(12, 0)， a.b(15, 0);
	print" ";
	print a.b(23, 1)， a.b(15, 0)， a.b(18, 0)， a.b(12, 0)， a.b(4, 0);
	print"!";a.~i();
	system("pause");
	return 0;}