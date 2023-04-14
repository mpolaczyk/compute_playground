# computePlayground

Author: [planet620]

The idea behind this repository is to:

- Gather performance focused code snippets

- Compare computing techniques

Projects:

- areaBenchmark - inspired by [Clean" Code, Horrible Performance]. Shows how naive OOP approach differs in performance from other techniques. 
    - Benchmarks:
        - Object oriented
        - Object oriented using PPL
    	- Struct with switch statement
    	- Array of coefficient
    	- Vectorized with SSE
    	- Vectorized with AVX
    	- Vectorized with AVX512
    	- Vectorized with ISPC (compiler set to use AVX instruction set)
    	- Vectorized with ISPC (compiler set to use AVX instruction set) using task parallelism
    	- Vectorized with AVX using threads
    	- Vectorized with AVX using thread pool
    	- Vectorized with AVX using PPL parallel transform
    - For different input data sizes: 256, 512, 4096, 32768, 262144, 1048576 shapes
    - Results:
        - OOP overhead is incredible
            - Switch statement immproves speed 3-4 times
            - Array of coefficients improves speed 4-6 times
            - Vectorization gives order of magnitude on top of that
        - For small datasets: Single threaded approaches are the fastest
            - Vectorized with ISPC is 50 times faster than OOP
        - For biggest datasets: Multi threaded solutions take over
            - PPL+AVX approach is 128 times faster than OOP
        - In general multithreading should be used for very big datasets, otherwise it has no sense
        - Using simple threads or ISPC task parallelism has very expensive overhead (thread creation)
            - Thread pool is 10 times faster than them
            - PPL is 20-140 times faster than them

![Results](chart.jpg)

### Dependencies

- Concurrent code uses Concurrency Visualizer
	- [Concurrency Visualizer SDK]
	- Extensions -> Add Concurrency Visualizer -> Restart VS and install -> Analyze -> Concurrency Visualizer -> Add SDK to project
- SIMD abstraction achieved with Intel SPMD Program Compiler [ISPC]
    - Add ispc.exe to PATH
	- Pre-Build Event is used to compile *.ispc programs.
- Performance measurement is done with [Google Benchmark]

[//]: # (links)

   [planet620]: <https://mpolaczyk.pl>
   [Concurrency Visualizer SDK]: <https://learn.microsoft.com/en-us/archive/blogs/visualizeparallel/introducing-the-concurrency-visualizer-sdk>
   [Google Benchmark]: <https://github.com/google/benchmark>
   [Clean" Code, Horrible Performance]: <https://www.youtube.com/watch?v=tD5NrevFtbU&ab_channel=MollyRocket>
   [ISPC]: <https://ispc.github.io/index.html>