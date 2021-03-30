    int set_range_table() {             //结论1:bitmap在预处理后的streamedges没有卵用
		int value = 0;
		Queue<std::tuple<int, long, long, int ,int> > tasks(65536);
		std::vector<std::thread> threads;
		long read_bytes = 0;

		long total_bytes = 0;
		for (int i=0;i<partitions;i++) {
			//if (!should_access_shard[i]) continue;//continue 会跳过当前循环中的代码，强迫开始下一次循环。
			for (int j=0;j<partitions;j++) {
				total_bytes += fsize[i][j];    //fsize（物理划分的大小）
			}
		}
		int read_mode;
		if (memory_bytes < total_bytes) {
			read_mode = O_RDONLY | O_DIRECT;
			// printf("use direct I/O\n");
		} else {
			read_mode = O_RDONLY;
			           // 确定printf("use buffered I/O\n");
		}

		int fin;
		long offset = 0;
		int update_mode = 1;
		long block_size_threads = (block_size/parallelism)/ edge_unit * edge_unit;
		//printf("block_size_threads: %ld\n", block_size_threads);
		switch(update_mode) {
		case 0: // source oriented update（源定向更新（工作准备））     两种遍历block的方式一种行优先一种列优先（这个大概是行优先）
			threads.clear();    //清空线程
			for (int ti=0;ti<parallelism;ti++) {
				threads.emplace_back([&](int thread_id){       //插入一个新的线程
					int local_value = 0;
					long local_read_bytes = 0;
					while (true) {
						int fin, partition_id,chunkmark;
						long offset, length;
						std::tie(fin, offset, length, partition_id, chunkmark) = tasks.pop();
						if (fin==-1) break;
						char * buffer = buffer_pool[thread_id];     //buffer_pool页表池子
						long bytes = pread(fin, buffer, length, offset);          //成功时返回读取的字节数      offset偏移量 读取地址=文件开始+offset。注意，执行后，文件偏移指针不变
						assert(bytes>0);
						local_read_bytes += bytes;
						// CHECK: start position should be offset % edge_unit
						for (long pos=offset % edge_unit;pos+edge_unit<=bytes;pos+=edge_unit) {
							Edge & e = *(Edge*)(buffer+pos);          //读出的边
							//if (bitmap==nullptr || bitmap->get_bit(e.source)) {
							//	local_value += process_pagerank(e);       //第一个函数的并发执行
							//}
						}
					}
					write_add(&value, local_value);    //确保原子性
					write_add(&read_bytes, local_read_bytes);
				}, ti);
			}
			fin = open((path+"/row").c_str(), read_mode);
			posix_fadvise(fin, 0, 0, POSIX_FADV_SEQUENTIAL);    //清理缓存
			for (int i=0;i<partitions;i++) {
				//if (!should_access_shard[i]) continue;
				for (int j=0;j<partitions;j++) {                      //Pji表示第j个作业共享的已加载图形结构分区Pi
					long begin_offset = row_offset[i*partitions+j];
					if (begin_offset - offset >= PAGESIZE) {
						offset = begin_offset / PAGESIZE * PAGESIZE;        
					}
					long end_offset = row_offset[i*partitions+j+1];
					if (end_offset <= offset) continue;
					while (end_offset - offset >= IOSIZE) {
						tasks.push(std::make_tuple(fin, offset, IOSIZE, i, j));
						offset += IOSIZE;
					}
					if (end_offset > offset) {
						tasks.push(std::make_tuple(fin, offset, (end_offset - offset + PAGESIZE - 1) / PAGESIZE * PAGESIZE, i, j));
						offset += (end_offset - offset + PAGESIZE - 1) / PAGESIZE * PAGESIZE;
					}
				}
			}
			for (int i=0;i<parallelism;i++) {
				tasks.push(std::make_tuple(-1, 0, 0, -1, -1));
			}
			for (int i=0;i<parallelism;i++) {
				threads[i].join();
			}
			break;
		case 1: // target oriented update     591579114
			fin = open((path+"/column").c_str(), read_mode);
			posix_fadvise(fin, 0, 0, POSIX_FADV_SEQUENTIAL);

			for (int cur_partition=0;cur_partition<partitions;cur_partition+=partition_batch) {
				VertexId begin_vid, end_vid;
				begin_vid = get_partition_range(vertices, partitions, cur_partition).first;
				if (cur_partition+partition_batch>=partitions) {
					end_vid = vertices;
				} else {
					end_vid = get_partition_range(vertices, partitions, cur_partition+partition_batch).first;
				}
			//	pre_source_window(std::make_pair(begin_vid, end_vid));
				// printf("pre %d %d\n", begin_vid, end_vid);
				threads.clear();
				for (int ti=0;ti<parallelism;ti++) {
					threads.emplace_back([&](int thread_id){
						int local_value = 0;
						long local_read_bytes = 0;
						while (true) {
							int fin, partition_id,chunkmark;
							long offset, length;
							VertexId startv,endv;
							startv = 2147483646;
							endv = 0;
							std::tie(fin, offset, length, partition_id, chunkmark) = tasks.pop();
							if (fin==-1) break;
							char * buffer = buffer_pool[thread_id];
							long bytes = pread(fin, buffer, length, offset);
							assert(bytes>0);
							local_read_bytes += bytes;
							// CHECK: start position should be offset % edge_unit
                            long pos;
							for (pos=offset % edge_unit; pos+block_size_threads<=bytes; pos+=block_size_threads) {
                                //if(should_access_shard_pagerank[partition_id])
                                    for(long pos_block = pos; pos_block+edge_unit<=pos+block_size_threads; pos_block+=edge_unit){
                                        Edge & e = *(Edge*)(buffer+pos_block);
										if(startv >= e.source)
											{
												startv = e.source;  //表的开始源节点
											}
										if(endv <= e.source)
											{
												endv = e.source;  //表的开始源节点
											}
										
	
                                        //process_pagerank(e);
                                    }
									range_table[chunkmark*partition_id+partitions] = endv - startv;
									
							}//最后一个之前的循环处理
                            //printf("range%d:%d-%d,size:%d\n",cur_partition*partition_batch+partition_id,startv,endv,endv - startv);
						   // range_table[thread_id+partition_id*partitions] = endv - startv;
							if(pos < bytes){
                                //if(should_access_shard_pagerank[partition_id])
                                    for(long pos_block = pos; pos_block+edge_unit<=bytes; pos_block+=edge_unit){
                                        Edge & e = *(Edge*)(buffer+pos_block);
										if(startv > e.source)
											{
												startv = e.source;  //表的开始源节点
											}
											if(endv < e.source)
											{
												endv = e.source;  //表的开始源节点
											}
											//range_table[cur_partition+partition_id*partitions] = endv - startv;
                                        //process_pagerank(e);
                                    }
                            //printf("range%d:%d-%d,size:%d\n",cur_partition*partition_batch+partition_id,startv,endv,endv - startv);
						    range_table[chunkmark*partition_id+partitions] = endv - startv;
							}
					    //startv = 2147483646;
					    //endv = 0;
						}

						write_add(&value, local_value);
						write_add(&read_bytes, local_read_bytes);
					}, ti);
				}
				offset = 0;
				for (int j=0;j<partitions;j++) {
					for (int i=cur_partition;i<cur_partition+partition_batch;i++) {
						if (i>=partitions) break;
						//if (!should_access_shard[i]) continue;
						long begin_offset = column_offset[j*partitions+i];
						/*if (begin_offset - offset >= PAGESIZE) {
							offset = begin_offset / PAGESIZE * PAGESIZE;
						}*/
						offset = begin_offset;

						long end_offset = column_offset[j*partitions+i+1];
						if (end_offset <= offset) continue;
						while (end_offset - offset >= IOSIZE) {
							tasks.push(std::make_tuple(fin, offset, IOSIZE, i ,j));
							offset += IOSIZE;
						}
						if (end_offset > offset) {
							tasks.push(std::make_tuple(fin, offset, end_offset - offset, i, j));
							offset += (end_offset - offset);
						}
					}
				}
				for (int i=0;i<parallelism;i++) {
					tasks.push(std::make_tuple(-1, 0, 0, -1, -1));
				}
				for (int i=0;i<parallelism;i++) {
					threads[i].join();
				}
				//post_source_window(std::make_pair(begin_vid, end_vid));
				// printf("post %d %d\n", begin_vid, end_vid);
				
				
				
			}

			break;
		default:
			assert(false);
		}

		close(fin);
		// printf("streamed %ld bytes of edges\n", read_bytes);
		return value;
	}