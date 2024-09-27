#include "time_heap.hpp"
#include <iostream>

void f(int i){
	std::cout<<"hello world"<<std::endl;
}

int main(){
	TimeHeap th;
	th.addTimer(1, std::chrono::milliseconds(2000), [](int)
				{ f(1); });
	while(1){
		th.tick();
	}
}