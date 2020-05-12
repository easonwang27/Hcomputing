#ifdef _APPLE_
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif 
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

//Implementation of OpenCL blocking data reading

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
    cl_int ret;
    cl_platform_id  platform=NULL;
    cl_device_id    device=NULL;
    cl_kernel       kernel=NULL;
    cl_context      context=NULL;
    cl_command_queue  queue =NULL;
    cl_program       program=NULL;
    cl_mem src1_memobj =NULL;
    cl_mem src2_memobj =NULL;
    cl_mem dst_memobj  =NULL;
    
    char *kernel_src =NULL;
    size_t kernel_len = 0;

    int *pHostBuffer   =NULL;
    int *pDeviceBuffer =NULL;


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
    const size_t contenLength = sizeof(int)*16*1024*1024;
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

    struct  timeval tsBegin,tsEnd;
    long t1Duration,t2Duration;
    gettimeofday(&tsBegin,NULL);
    err = clEnqueueWriteBuffer(queue,src1_memobj,CL_TRUE,0,contenLength,pHostBuffer,0,NULL,NULL);
    check_err(err,"data1 write");
    gettimeofday(&tsEnd,NULL);
    t1Duration = 1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+(tsEnd.tv_usec-tsBegin.tv_usec);
    gettimeofday(&tsBegin,NULL);
    err = clEnqueueWriteBuffer(queue,src2_memobj,CL_TRUE,0,contenLength,pHostBuffer,0,NULL,NULL);
    check_err(err,"data2 write");
    gettimeofday(&tsEnd,NULL);
    t1Duration = 1000000L*(tsEnd.tv_sec-tsBegin.tv_sec)+(tsEnd.tv_usec-tsBegin.tv_usec);

    printf("t1 duration: %ld,t2 duration:%ld",t1Duration,t2Duration);

    free(pHostBuffer);
    clReleaseMemObject(src1_memobj);
    clReleaseMemObject(src2_memobj);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    printf("program complete\n");
    return 0;

}