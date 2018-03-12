#include <iostream>
#include <future>

using namespace std;

void func_1(){
	for(unsigned long i = 0; i < 5000; i++)
		cout << "Hebra 1, i =  " << i << endl;
}

void func_2(){
	for(unsigned long i = 0; i < 5000; i++)
		cout << "Hebra 2, i = " << i << endl;
}

long factorial)(int n){
	return n > 0 ? n* factorial(n-1) : 1;
}

int main(){
	thread hebra1(func_1), hebra2(func_2);
}
