#ifdef _APPLE_
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    cl_event evt1;
    clock_t start, end;
    start = clock();  
    err = clEnqueueWriteBuffer(queue,src1_memobj,CL_FALSE,0,contenLength,pHostBuffer,0,NULL,&evt1);
    check_err(err,"data1 write");
    end = clock(); 
    double seconds  =(double)(end - start)/CLOCKS_PER_SEC;

    clock_t start_1, end_1;
    start_1 = clock();  
    err = clEnqueueWriteBuffer(queue,src2_memobj,CL_TRUE,0,contenLength,pHostBuffer,0,NULL,NULL);
    check_err(err,"data2 write");
    end_1 = clock(); 
    double seconds_1  =(double)(end_1 - start_1)/CLOCKS_PER_SEC;

    printf("t1 duration: %f,t2 duration:%f\n",seconds,seconds_1);

    free(pHostBuffer);
    clReleaseMemObject(src1_memobj);
    clReleaseMemObject(src2_memobj);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    printf("program complete\n");
    return 0;

}