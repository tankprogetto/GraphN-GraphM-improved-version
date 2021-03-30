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

class memory{       //存chunk块的
	int size;
	int chunk_size;
	//指向首部的指针
	int * mem_table
	void *  mem;
	void ** pointer;
	
	memory()
	{
		size = 0;
		chunk_size = 0;
		mem = NULL;
		
		pointer = NULL ;
	}
	int init(int size,int chunk_size)
	{
		this->size=size;
		this->chunk_size=chunk_size;
		mem = (Edge *)calloc(size/chunk_size,chunk_size);
		assert(mem != NULL);
		return 0;
	}
	
	int chunk_in()
	{
		
	}
	
	
	
	
	~memory()
	{
		size = 0;
		chunk_size = 0;
		free(mem);
	}
	
	
}






#endif
