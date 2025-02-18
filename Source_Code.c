/* Student name: Ali Shaikh Qasem,  id: 1212171 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define rows1 100
#define columns1 100
#define rows2 100
#define columns2 100
#define numOfThreads 2
#define numOfProcs 4


int matrixOne [rows1] [columns1];
int matrixTwo [rows2] [columns2];
int result [rows1] [columns2];

int startPoints [numOfThreads];
int endPoints [numOfThreads];
int threadsCompleted = 0;

void * threadFunc (void *arg);

int main()
{
    fillMatrixOne();
    fillMatrixTwo();
    //printMatrix(matrixOne);
    //printf("\n\n");
    //printMatrix(matrixTwo);

    // naive solution without threads or processes:
    //*****************************************************************************************************************************************************

    // defining start and end time
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    normalSolution();
    clock_gettime(CLOCK_MONOTONIC, &end);
    //printf("\n\n normal solution result: \n\n");
    //printMatrix(result);

    double execution_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Naive solution time: %f seconds\n", execution_time);

    clearMatrix(result);

    //*****************************************************************************************************************************************************

    // multi-process solution:
    //*****************************************************************************************************************************************************

    //finding start and end points for each process
    int proc_start_points [numOfProcs];
    int proc_end_points [numOfProcs];

    findRanges(proc_start_points,proc_end_points,numOfProcs);

    //creating the shared memory
    int shared_memory_id = shmget(0,sizeof(int) * rows1 * columns1, IPC_CREAT | 0666);
    if(shared_memory_id == -1)
    {
       printf("Error, cannot create shared memory");
    }

    //Attach the shared memory to the result array
    int (*resultArray) [columns1] = shmat (shared_memory_id,NULL,0);

    //defining start and end time
    struct timespec start1, end1;

    clock_gettime(CLOCK_MONOTONIC, &start1);
    //creating child processes
    for(int i=0 ; i<numOfProcs ; i++)
    {
        pid_t pid  = fork();

        if(pid < 0 )
        {
           perror("fork failed\n");
        }
        else if (pid == 0 ) // the child process
        {
            childProcess(resultArray, proc_start_points[i], proc_end_points[i] );
            exit(1);

        }

    }

    //waiting for children to finish
    for(int i=0 ; i<numOfProcs ; i++)
    {
        wait(NULL);
    }


    clock_gettime(CLOCK_MONOTONIC, &end1);
    //printMatrix(resultArray);

    double execution_time1 = (end1.tv_sec - start1.tv_sec) + (end1.tv_nsec - start1.tv_nsec) / 1e9;
    printf("processes solution time: %f seconds\n", execution_time1);

    //detach and remove the shared memory
    if(shmdt(resultArray) != 0)
       printf("\nError in detaching shared memory\n");

    if(shmctl(shared_memory_id, IPC_RMID,NULL) != 0)
       printf("\nError in removing the shared memory\n");




    //***************************************************************************************************************************************************


    // multi-threaded solution:
    //***************************************************************************************************************************************************

    //defining start and end times
    struct timespec start2, end2;

    pthread_t thread [numOfThreads];

    findRanges(startPoints,endPoints,numOfThreads);

    //defining an array to hold threads numbers to avoid racing in the thread routine.
    int threadNum [numOfThreads];
    for(int i=0 ; i<numOfThreads ; i++)
    {
        threadNum[i] = i;
    }

    /* part 1: joinable threads ****************************************  */

    //creating threads
    clock_gettime(CLOCK_MONOTONIC, &start2);
    for(int i=0 ; i<numOfThreads ; i++)
    {

        // storing the thread number in a specific address so we won't lose the value
        int created = pthread_create(&thread[i], NULL ,&threadFunc, &threadNum[i] );
        if (created != 0)
            perror("failed to create the thread\n");

    }

    //joining threads
    for(int i=0 ; i<numOfThreads ; i++)
    {
        int joined = pthread_join(thread[i], NULL);
        if( joined != 0)
            perror("failed to join the thread\n");

    }
    clock_gettime(CLOCK_MONOTONIC, &end2);
    //printf("threaded solution result: \n\n");
    //printMatrix(result);
    threadsCompleted = 0;


    double execution_time2 = (end2.tv_sec - start2.tv_sec) + (end2.tv_nsec - start2.tv_nsec) / 1e9;
    printf("Joinable threads solution time: %f seconds\n", execution_time2);

    clearMatrix(result);

//    /* part 2: Detached threads **************************************** */

    //defining start and end times
    struct timespec start3, end3;


    //Setting the thread to be detached
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    //creating threads
    clock_gettime(CLOCK_MONOTONIC, &start3);
    for(int i=0 ; i<numOfThreads ; i++)
    {
        int created = pthread_create(&thread[i], &attr ,&threadFunc, &threadNum[i] );
        if (created != 0)
            perror("failed to create the thread\n");

    }

    //destroying detached attributes
    pthread_attr_destroy(&attr);

    //waiting for detached threads to finish their tasks
    while (threadsCompleted < numOfThreads)
    {
        //do nothing (wait for threads to finish).
    }
    clock_gettime(CLOCK_MONOTONIC, &end3);

    double execution_time3 = (end3.tv_sec - start3.tv_sec) + (end3.tv_nsec - start3.tv_nsec) / 1e9;
    printf("Detached threads solution time: %f seconds\n", execution_time3);


    //printMatrix(result);
    clearMatrix(result);
    threadsCompleted = 0;

//    /* part 3: Mix of joinable and detached threads ****************************************  */

    //defining start and end times
    struct timespec start4, end4;

    //Setting the detached threads attributes
    pthread_attr_t attr1;
    pthread_attr_init(&attr1);
    pthread_attr_setdetachstate(&attr1,PTHREAD_CREATE_DETACHED);

    //creating threads
    clock_gettime(CLOCK_MONOTONIC, &start4);
    for(int i=0 ; i<numOfThreads ; i++)
    {
        if(i < numOfThreads/2) // joinable threads
        {
            int created = pthread_create(&thread[i], NULL ,&threadFunc, &threadNum[i] );
            if (created != 0)
               perror("failed to create the thread\n");
        }
        else // detached threads
        {
            int created = pthread_create(&thread[i], &attr1 ,&threadFunc, &threadNum[i] );
            if (created != 0)
               perror("failed to create the thread\n");
        }
    }

    //joining joinable threads
    for(int i=0 ; i<numOfThreads/2 ; i++)
    {
        int joined = pthread_join(thread[i], NULL);
        if( joined != 0)
            perror("failed to join the thread\n");

    }

    //destroying detached attributes
    pthread_attr_destroy(&attr1);

    //waiting for detached threads to finish
    while (threadsCompleted < numOfThreads)
    {
        // do nothing (wait for threads to finish their work).
    }
    clock_gettime(CLOCK_MONOTONIC, &end4);

    double execution_time4 = (end4.tv_sec - start4.tv_sec) + (end4.tv_nsec - start4.tv_nsec) / 1e9;
    printf("Mixed threads solution time: %f seconds\n", execution_time4);


    //printMatrix(result);
    clearMatrix(result);
    threadsCompleted = 0;

    //*****************************************************************************************************************************************************

    return 0;
}

void fillMatrixOne ()
{
    // my id = 1212171.
    for(int i=0 ; i<rows1 ; i++)
    {
        for(int j=0 ; j<columns1 ;j++)
        {
            switch ((i*columns1 + j + 1)%7)
            {
                case 1: matrixOne[i][j] = 1;
                break;
                case 2: matrixOne[i][j] = 2;
                break;
                case 3: matrixOne[i][j] = 1;
                break;
                case 4: matrixOne[i][j] = 2;
                break;
                case 5: matrixOne[i][j] = 1;
                break;
                case 6: matrixOne[i][j] = 7;
                break;
                case 0: matrixOne[i][j] = 1;
                break;

            }
        }
    }

}
void fillMatrixTwo ()
{
    // my student number * birth year = 1212171 * 2003 = 2427978513.
    for(int i=0 ; i<rows2 ; i++)
    {
        for(int j=0 ; j<columns2 ;j++)
        {
            switch ((i*columns2 + j + 1)%10)
            {
                case 1: matrixTwo[i][j] = 2;
                break;
                case 2: matrixTwo[i][j] = 4;
                break;
                case 3: matrixTwo[i][j] = 2;
                break;
                case 4: matrixTwo[i][j] = 7;
                break;
                case 5: matrixTwo[i][j] = 9;
                break;
                case 6: matrixTwo[i][j] = 7;
                break;
                case 7: matrixTwo[i][j] = 8;
                break;
                case 8: matrixTwo[i][j] = 5;
                break;
                case 9: matrixTwo[i][j] = 1;
                break;
                case 0: matrixTwo[i][j] = 3;
                break;

            }
        }
    }



}
void printMatrix (int matrix [rows1][columns1])
{
    for(int i=0 ; i<rows1 ; i++)
    {
        for(int j=0; j<columns1 ; j++)
        {
            printf ("%d  ",matrix[i][j]);
        }
        printf("\n");
    }
}
void normalSolution()
{
    for(int i=0 ; i<rows1 ;i++)
    {
        for(int j=0 ; j<columns2 ; j++)
        {
            result[i][j] = 0;
            for(int k=0 ; k<columns2 ; k++)
                result[i][j] += matrixOne[i][k]*matrixTwo[k][j];
        }
    }
}
void * threadFunc (void *arg)
{
    int thIndex = *(int*)arg;
    //printf("%d\n",thIndex);

    for(int i=startPoints[thIndex] ; i<= endPoints[thIndex] ;i++)
    {
        for(int j=0 ; j<columns2 ; j++)
        {
            result[i][j] = 0;
            for(int k=0 ; k<columns2;k++)
                result[i][j] += matrixOne[i][k]*matrixTwo[k][j];
        }
    }
    //incrementing the number of completed threads (helpful with detached threads)
    threadsCompleted++;
}
void childProcess (int result_Array [rows1][columns1] , int start , int end)
{
   for(int i=start ; i<= end ;i++)
    {
        for(int j=0 ; j<columns2 ; j++)
        {
            result_Array[i][j] =0;
            for(int k=0 ; k<columns2;k++)
                result_Array[i][j] += matrixOne[i][k]*matrixTwo[k][j];
        }
    }
}

void findRanges (int start_Points [], int end_Points [], int number)
{
    int portion = rows1/number;
    int thread_proc_Portion [number];
    int remainingElements = rows1 - (portion * number);

    // caculating the portion of each thread/process
    for(int i=0 ; i<number ; i++)
    {
        thread_proc_Portion [i] = portion;
    }
    // in the case the portion result gives a rational number, we distribute the remaining rows to the threads/processes
    for(int i=remainingElements, j=0 ; i>0 ;  j++,i--)
    {
        thread_proc_Portion[j] += 1;
    }
    // finding the start and end points for each thread/process
    for(int i=0 ; i<number ; i++)
    {
        if (i==0)
        {
            start_Points[i] = 0;
            end_Points[i] = thread_proc_Portion[i] - 1;
        }
        else
        {
            start_Points[i] = end_Points[i-1] + 1;
            end_Points[i] = start_Points[i] + thread_proc_Portion[i] - 1;
        }
        //printf("thread/process %d starts at %d ends at %d\n ",i+1,start_Points[i],end_Points[i]);

    }

}
void clearMatrix (int matrix [rows1][columns1])
{
    for(int i=0 ; i<rows1 ; i++)
      for(int j=0 ; j<columns1 ; j++)
        matrix[i][j] = 0;
}