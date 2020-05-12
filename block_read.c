#ifdef _APPLE_
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

//Implementation of OpenCL blocking data reading
#define check_print() printf("this is trace\n")

int  check_err(cl_int err,const char *s)
{
    if(err < 0)
    {
        perror("%s error");
        exit(1);
    }
}
static volatile bool canContinue = false;
static void MyEventHandler(cl_event event,cl_int status,void *userData)
{
    printf("%s\n",userData);
    if(status ==CL_SUBMITTED)
    {
        printf("the current status is submitted.\n");
    }
    canContinue = true;
}

int main(void)
{
    cl_int err;
    cl_platform_id  platform;
    cl_device_id    device;
    cl_context      context;
    cl_command_queue  queue;
    cl_mem src1_memobj;
    cl_mem src2_memobj;

    int *pHostBuffer;
   // int *pDeviceBuffer;
    //step1 get platform
    err = clGetPlatformIDs(1,&platform,NULL);
    check_err(err,"get platform");
    
    //step2 get device
    err = clGetDeviceIDs(platform,CL_DEVICE_TYPE_CPU,1,&device,NULL);
    check_err(err,"get device");
    //step3 Setting up context according to device
    context= clCreateContext(NULL,1,&device,NULL,NULL,&err);
    if(context ==NULL)
    {
        perror("context is null");
        exit(1);
    }
   
    check_err(err,"crate context");
    //step4 create queue
    queue = clCreateCommandQueue(context,device,0,&err);
    if(queue ==NULL)
    {
        perror("queue is null");
        exit(1);
    }
    //step5 create buffer mem type
    const size_t contenLength = 1024;
   
    src1_memobj = clCreateBuffer(context,CL_MEM_READ_ONLY,contenLength,NULL,&err);
   
    if(src1_memobj==NULL)
    {
        perror("src1_memobj is null");
        exit(1);
    }
    src2_memobj = clCreateBuffer(context,CL_MEM_READ_ONLY,contenLength,NULL,&err);
    if(src2_memobj==NULL)
    {
        perror("src2_memobj is null");
        exit(1);
    }
    
    // alloc 64MB data
    pHostBuffer = malloc(contenLength);
    for(int i = 0; i < contenLength;i++)
        pHostBuffer[i]= i;

    //listen evt1 src1_memobj transmission status

    cl_event evt1,evt2;
    err = clEnqueueWriteBuffer(queue,src1_memobj,CL_FALSE,0,contenLength,pHostBuffer,0,NULL,&evt1);
    check_err(err,"data1 write");
   
    err = clEnqueueWriteBuffer(queue,src2_memobj,CL_FALSE,0,contenLength,pHostBuffer,1,&evt1,&evt2); //after evt1 the evt2
    check_err(err,"data2 write");

    clFlush(queue);
    struct timeval tsBegin,tsEend;
    gettimeofday(&tsBegin,NULL);
    clWaitForEvents(1,&evt2);
    gettimeofday(&tsEend,NULL);
    long duration = 1000000L *(tsEend.tv_sec-tsBegin.tv_sec)+(tsEend.tv_usec-tsBegin.tv_usec);   
    printf("wait time spent:%ldus\n",duration);
    cl_int status;

    err = clGetEventInfo(evt2,CL_EVENT_COMMAND_EXECUTION_STATUS,sizeof(status),&status,NULL);
    switch (status)
    {
    case CL_QUEUED:
        printf("evt2 status is CL_QUEUED\n");/* code */
        break;
    case CL_RUNNING:
        printf("evt2 status is CL_RUNNING\n");/* code */
        break;
    case CL_COMPLETE:
        printf("evt2 status is CL_COMPLETE\n");/* code */
        break;
    case CL_SUBMITTED:
        printf("evt2 status is CL_SUBMITTED\n");/* code */
        break;

    default:
        break;
    }
    printf("the current status of evt2 is :%d\n",status);
    //clReleaseEvent(evt1);
    //clReleaseEvent(evt2);
    free(pHostBuffer);
    
    
    clReleaseMemObject(src1_memobj);
    clReleaseMemObject(src2_memobj);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    printf("program complete\n");
    return 0;

}