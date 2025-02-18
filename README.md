# Matrix Multiplication Using Multi-Processing and Multi-Threading

## Overview
This project explores different parallelization techniques to optimize matrix multiplication using multi-processing and multi-threading in a Linux environment. It compares the performance of different approaches and analyzes how synchronization and resource management impact execution time.

## Objectives
- Implement matrix multiplication using different parallelization strategies.
- Understand the concepts of multi-processing and multi-threading.
- Compare execution times and throughput for various approaches.
- Analyze the impact of synchronization on performance.

## Implemented Solutions
1. **Naive Solution**: A standard sequential matrix multiplication.
2. **Multi-Processing Solution**: Using the `fork()` function to create multiple processes.
3. **Multi-Threading Solution**: Implemented with the POSIX `pthread` library.
    - **Joinable Threads**
    - **Detached Threads**
    - **Mixed Joinable & Detached Threads**

## Methodology
- The `clock_gettime()` function was used to measure execution time.
- Each execution was run multiple times, and the average time was recorded.
- Various configurations of processes and threads were tested to determine the optimal setup.

## Results & Analysis
### **Naive Solution**
- Execution Time: **7.65 ms**
- Throughput: **130.7 operations per second**
- Performance is significantly lower than parallel approaches.

### **Multi-Processing Solution**
| Number of Processes | Execution Time (ms) | Throughput (ops/sec) |
|--------------------|------------------|------------------|
| 2  | 5.03 | 198.8 |
| 4  | 4.64 | 215.5 |
| 6  | 6.94 | 144.1 |
| 8  | 5.05 | 198.1 |
| 10 | 5.11 | 195.6 |
- The best performance was achieved with **4 processes**.
- More processes did not always improve performance due to resource overhead.

### **Multi-Threading Solution**
#### **Joinable Threads**
| Number of Threads | Execution Time (ms) | Throughput (ops/sec) |
|------------------|------------------|------------------|
| 2  | 3.84 | 260.4 |
| 4  | 3.81 | 262.4 |
| 6  | 3.74 | 267.4 |
| 8  | 3.45 | 289.8 |
| 10 | 4.29 | 233.1 |
- **8 threads** gave the best performance.
- More than 8 threads resulted in performance degradation due to overhead.

#### **Detached Threads**
| Number of Threads | Execution Time (ms) | Throughput (ops/sec) |
|------------------|------------------|------------------|
| 2  | 9.74 | 102.7 |
| 4  | 5.21 | 191.9 |
| 6  | 4.55 | 219.8 |
| 8  | 4.70 | 212.8 |
| 10 | 5.88 | 170.1 |
- **6 detached threads** performed the best.
- Performance was slightly lower than joinable threads due to synchronization overhead.

#### **Mixed Joinable & Detached Threads**
| Number of Threads | Execution Time (ms) | Throughput (ops/sec) |
|------------------|------------------|------------------|
| 2  | 4.84 | 206.6 |
| 4  | 4.62 | 216.4 |
| 6  | 4.06 | 246.3 |
| 8  | 4.65 | 215.1 |
| 10 | 4.82 | 207.5 |
- **6 threads (mixed)** provided the best balance between performance and synchronization.

## Conclusion
- **Joinable threads with 8 threads** was the most efficient solution, achieving the best execution time.
- **Multi-processing** improved performance but had more overhead compared to threading.
- **Detached threads performed worse than joinable threads**, due to the extra synchronization required.
- **Mixed joinable & detached threads** provided a good balance but did not outperform pure joinable threads.

## Notes
- These results were obtained in a **virtual machine**, which may not fully utilize system resources.
- Performance may vary depending on hardware specifications (CPU cores, clock speed, memory speed, etc.).

## Author
Ali Shaikh Qasem

