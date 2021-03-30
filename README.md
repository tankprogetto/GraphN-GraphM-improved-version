# GraphN-GraphM-
随着现实世界中图处理需求的快速增长，大量迭代图处理作业同时在同一基础图上运行。而现有的并发图分析处理系统存在大量冗余数据存储和访问开销现有的并发图分析处理系统的存储系统则存在块表信息过大，对内存的利用效率不高等方面的问题，这些问题的原因一方面是块表信息的数据结构不合理，另一方面是块表信息过大后造成挤占可用内存空间的问题。 为解决此问题我们在现有的并发图分析处理系统的存储系统GraphM上实现了包括块表信息存储结构改进，块表优先级调度策略，轻量级的内存管理系统等多项重大改进。改进后的系统称作GraphN。实验表明：通过优化块表信息的数据结构，减少非必要的文件读写开销，将会造成更多读写开销的块缓存在内存中等方式，GraphN的块表信息仅占GraphM块表信息大小的千分之一，且同条件下同一任务所需时间缩短20%以上。
注意此工程为https://github.com/Program-Anonymity/GraphM 的在块表划分，存储方式与调度算法改进后的改进版本，其整体运行流程与GraphM并无差异，因此以下部分引用自GraphM的readme.md
并附上了GraphN的运行截图供参考。使用的图样例为：[LiveJournal](http://snap.stanford.edu/data/soc-LiveJournal1.html)

# Integrated with existing graph processing systems
Sharing() call is inserted between successive graph loads in the existing systems (e.g., the function EdgeStreams() in GridGraph), and init() call is implemented before the processing. Besides, declarations are made while traversing the graph structure for fine-grained synchronization.

# Compilation
Compilers supporting basic C++11 features (lambdas, threads, etc.) and OpenMP are required, the other requirements and the compiled method are same as the original systems. Take GridGraph as an example:
```
make
```
# Preprocessing
Before running concurrent applications on a graph, the original graph data needs to be first partitioned into the grid format for GridGraph. To partition the original graph data:
```
./bin/preprocess -i [input path] -o [output path] -v [vertices] -p [partitions] -t [edge type: 0=unweighted, 1=weighted]
```
Then, the graph partitions need to be further logically labeled into chunks. In order to label the graph data, just give the size of the last-level cache and the size of the graph data:
```
./bin/Preprocessing [path]  [cache size in MB] [graph size in MB] [memory budget in GB]
```
For example, we want to divide the grid format [LiveJournal](http://snap.stanford.edu/data/soc-LiveJournal1.html) graph into chunks using a machine with 20M Last-level Cache and 8 GB RAM:
```
./bin/Preprocessing /data/LiveJournal 20 526.38 8
```
![image](https://user-images.githubusercontent.com/50234138/113051479-998c3c00-91d8-11eb-9805-766eebcc5abf.png)

it should come like this：
![image](https://user-images.githubusercontent.com/50234138/113051546-aad54880-91d8-11eb-8a44-5cf24cee7266.png)
the chunk file like this：
![image](https://user-images.githubusercontent.com/50234138/113051621-c6d8ea00-91d8-11eb-951e-4847c6612c18.png)



# Running Applications
We concurrently submmit the PageRank, WCC, BFS, SSSP to GridGraph-M through the concurrent_jobs application. To concurrently run these applications, just need to give the follwing parameters:
```
./bin/concurrent_jobs [path] [number of submissions] [number of iterations] [start vertex id] [cache size in MB] [graph size in MB] [memory budget in GB]
```
For example, to run 10 iterations of above four algorithms as eight jobs (i.e., submitting the same job twice in succession) on the LiveJournal:
```
./bin/concurrent_jobs /data/LiveJournal 2 10 0 20 526.38 8
```
![image](https://user-images.githubusercontent.com/50234138/113051756-ec65f380-91d8-11eb-9453-252a7e6e735e.png)
![image](https://user-images.githubusercontent.com/50234138/113051782-f25bd480-91d8-11eb-9a0e-4cc4a08c2346.png)

#引用自：
Jin Zhao, Yu Zhang, Xiaofei Liao, Ligang He, Bingsheng He, Hai Jin, Haikun Liu, and Yicheng Chen. GraphM: An Efficient Storage System for High Throughput of Concurrent Graph Processing. Proceedings of the 2019 International Conference for High Performance Computing, Networking, Storage, and Analysis.

To cite GraphM, you can use the following BibTeX entry:
@inproceedings{DBLP:conf/sc/ZhaoZLHH0LC19,
  author    = {Jin Zhao and Yu Zhang and Xiaofei Liao and Ligang He and Bingsheng He and Hai Jin and Haikun Liu and Yicheng Chen},
  title     = {GraphM: an efficient storage system for high throughput of concurrent graph processing},
  booktitle = {Proceedings of the International Conference for High Performance Computing,
               Networking, Storage and Analysis, {SC} 2019, Denver, Colorado, USA,
               November 17-19, 2019},
  pages     = {3:1--3:14},
  publisher = {{ACM}},
  year      = {2019},
  url       = {https://doi.org/10.1145/3295500.3356143},
  doi       = {10.1145/3295500.3356143},
  timestamp = {Mon, 10 Aug 2020 08:13:11 +0200},
  biburl    = {https://dblp.org/rec/conf/sc/ZhaoZLHH0LC19.bib},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}

