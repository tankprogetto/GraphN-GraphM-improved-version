#include <iostream>
#include <fstream>
#include <vector>
#include "./core/graph.hpp"
#include "./core/memory_control.hpp"
using namespace std;
  
int main(int argc, char** argv)
{
 string path(argv[1]);
 cout << "read from " << path << endl;
 Entry * num;
 num = new Entry [300]; 
 ifstream file(path, ios::in | ios::binary);
if(!file)
{
 cout << "error" << endl;
 return 0;
}
 int i = 0;
 while(file.read((char *)&num[0],300*sizeof(Entry)))   
 {

	i++;
 }
  	for(int p=0;p<=299;p+=1)
	{
		cout <<num[p].source<< " " << num[p].number<<" "<<num[p].offset<< " "<<num[p].belong_part_j<<" "<<num[p].belong_part_i<<endl;
	}
 //file.close();
 return 0;
}
