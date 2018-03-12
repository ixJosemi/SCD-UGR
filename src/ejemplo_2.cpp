#include <iostream>
#include <thread>

using namespace std;

void func_1(){
	for(unsigned long i = 0; i < 5000; i++)
		cout << "Hebra 1, i =  " << i << endl;
}

void func_2(){
	for(unsigned long i = 0; i < 5000; i++)
		cout << "Hebra 2, i = " << i << endl;
}

int main(){
	thread hebra1, hebra2;
	
	hebra1 = thread(func_1);
	hebra2 = thread(func_2);
}
