#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <malloc.h>
#include <omp.h>
#include <string.h>

#include <thread>
#include <vector>
#include <functional>

#include "core/constants.hpp"
#include "core/type.hpp"
#include "core/bitmap.hpp"
#include "core/atomic.hpp"
#include "core/queue.hpp"
#include "core/partition.hpp"
#include "core/bigvector.hpp"
#include "core/time.hpp"

class Table_memory{       //存chunk块的
	int size;
	int Vertex_num;
	int chunk_size;
	//指向首部的指针
	Entry *  mem;
	int parallelism;
	void ** pointer;
	Table_memory()
	{
		size = 0;
		chunk_size = 0;
		parallelism = 1;
		mem = NULL;
		pointer = NULL ;
	}
	int init(int chunk_size, int VertexId_num, int parallelism)
	{
		this->size=VertexId_num*sizeof(Entry);
		this->chunk_size=chunk_size;
		this->Vertex_num = VertexId_num;
		mem = calloc(VertexId_num,Entry);
		assert(mem != NULL);
		int i = 0;
		int count = 0; //读完某个文件mem在哪,所有读完后应等于VertexId_num-1;
		//int set_flag = 0;     
		for(i=0;i<parallelism;i++)
		{
			char file_no[4];
			itoa(i, file_no, 10);
			ifstream file("chunk-table-"+=file_no, ios::in | ios::binary);
			if(!file)
			{
				cout << "error" << endl;
				return 0;
			}
			
			 while(file.read((char *)&mem[set_flag],GetFileLength("chunk-table-"+=file_no)))   
			{
				count++;
				//set_flag++;
			}
		}
		assert(count == VertexId_num-1);
		return 0;
	}
	long GetFileLength(string strPath)         //获取文件长度
	{
		ifstream fin(strPath.c_str(), ios::in | ios::binary);
		fin.seekg(0, ios_base::end);
		streampos pos = fin.tellg();
		long lSize = static_cast<long>(pos);
		fin.close();
		return lSize;
	}
	std::tuple<int, int, int, long> get_VertexId_message(VertexId ver)
	{
		for(int i=0;i<this->Vertex_num;i++)
		{
			if(ver == mem[i].source)
				return std::make_tuple(mem[i].number, mem[i].offset, mem[i].belong_part_j, mem[i].belong_part_i);
		}	
		
	}
	
	~Table_memory()
	{
		size = 0;
		chunk_size = 0;
		free(mem);
	}
	
	
}






#endif