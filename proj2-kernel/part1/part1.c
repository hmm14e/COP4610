#include<unistd.h>

int main(){

    for(int i=0; i<8; i++)
	access(".", F_OK);

    return 0;

}
