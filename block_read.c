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
    cl_mem dst_memobj;
    cl_kernel kernel;
    cl_kernel kernel2;
    cl_program program;
    cl_event evt1,evt2;
    char *kernel_src;
    int *pHostBuffer;
    int *pDeviceBuffer;
    
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
    dst_memobj = clCreateBuffer(context,CL_MEM_READ_WRITE,contenLength,NULL,&err);
    if((dst_memobj == NULL)||(err < 0))
    {
        perror("out mem fail");
        exit(1);
    }
    //path kernel 
    const char *pFileName = "./kernel.cl";
    FILE *fp = fopen(pFileName,"r");
    if(fp==NULL)
    {
        perror("can't find kernel.cl");
        exit(1);
    }
    fseek(fp,0,SEEK_END);
    const long kernel_len = ftell(fp);
    fseek(fp,0,SEEK_SET);
    kernel_src = malloc(kernel_len);
    fread(kernel_src,1,kernel_len,fp);
    fclose(fp);
    //create program
    program = clCreateProgramWithSource(context,1,(const char**)&kernel_src,&kernel_len,&err);
    if((program ==NULL)||(err < 0))
    {
        perror("create program fail");
        exit(1);
    }
    //build program
    err = clBuildProgram(program,1,&device,NULL,NULL,NULL);
    if(err < 0)
    {   size_t len;
        char buffer[1024];
        printf("error :failed to build program exe!\n");
        clGetProgramBuildInfo(program,device,CL_PROGRAM_BUILD_LOG,sizeof(buffer),buffer,&len);
        printf("%s\n",buffer);
        perror("build program fail");
        exit(1);
    }
    //create kernel , first kernel fun is "kernel_test"
    kernel = clCreateKernel(program,"kernel1_test",&err);
    if(kernel == NULL)
    {
        perror("create kernel fail");
        exit(1);
    }  
    //set kernel arg
    err = clSetKernelArg(kernel,0,sizeof(cl_mem),(void*)&dst_memobj);
    err|= clSetKernelArg(kernel,1,sizeof(cl_mem),(void*)&src1_memobj);
    err|= clSetKernelArg(kernel,2,sizeof(cl_mem),(void*)&src2_memobj);
    if(err < 0)
    {
        perror("set kernel arg fail");
        exit(1);
    }

    
    //create kernel2  kernel2_test
    //Now when the device is doing calculation, the host side is not interfered
    kernel2= clCreateKernel(program,"kernel2_test",&err);
    if(kernel2 ==NULL)
    {
        perror("some error\n");
        exit(1);
    }
    if(err < 0)
    {
        perror("create kernel2 fail");
        exit(1);
    }
   
    // Set the associated kernel2 parameter
    err= clSetKernelArg(kernel2,0,sizeof(cl_mem),&dst_memobj);
    err|=clSetKernelArg(kernel2,1,sizeof(cl_mem),&src1_memobj);
    err|=clSetKernelArg(kernel2,2,sizeof(cl_mem),&src2_memobj);
    if(err < 0)
    {
        perror("associated kernel2 parameter");
        exit(1);
    }
    // alloc 64MB data
    pHostBuffer = malloc(contenLength);
    for(int i = 0; i < contenLength;i++)
        pHostBuffer[i]= i;


    //listen evt1 src1_memobj transmission status
  
    err = clEnqueueWriteBuffer(queue,src1_memobj,CL_FALSE,0,contenLength,pHostBuffer,0,NULL,&evt1);
    check_err(err,"data1 write");
    err = clEnqueueWriteBuffer(queue,src2_memobj,CL_FALSE,0,contenLength,pHostBuffer,1,&evt1,&evt2); //after evt1 the evt2
    check_err(err,"data2 write");
   // printf("the current status of evt2 is :%d\n",status);
    //out put mem

    //get work_group max size
    size_t maxWorkGoupSize = 0;
    clGetDeviceInfo(device,CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(maxWorkGoupSize),(void*)&maxWorkGoupSize,NULL);
    if(err < 0)
    {
        perror("can't get device work_group max szie");
        exit(1);
    }
    printf("work group max size :%ld\n",maxWorkGoupSize);

    //wait data trs
    //wait 2 pid 
    clWaitForEvents(2,(cl_event[2]){evt1,evt2});
    
    clReleaseEvent(evt1);
    clReleaseEvent(evt2);
    evt1 =NULL;
    evt2 =NULL;
    //Determine the size of work items and the size of each team
    //Each team has maxWorkGoupSize work-items
    //the use evt1 track kernel1 fun exe status
    size_t work_group_size = 4;
    err = clEnqueueNDRangeKernel(queue,kernel,1,NULL,&contenLength,&work_group_size,0,NULL,&evt1);
    if(err < 0)
    {
        perror("enqueue kernel fun 1 fail");
        exit(1);
    }
  
    //this kernel2 must wait kernel1 Execution complete
    err = clEnqueueNDRangeKernel(queue,kernel2,1,NULL,&contenLength,&work_group_size,1,&evt1,&evt2);
    if(err < 0)
    {
        perror("enqueue kernel kernel2 fail\n");
        exit(1);
    }
    
    //read Return to verification
    pDeviceBuffer = (int *)malloc(contenLength);
    if(pDeviceBuffer ==NULL)
    {
        perror("pDeviceBuffer malloc fail");
        exit(1);
    }
    //read buffer form in kernel
    err = clEnqueueReadBuffer(queue,dst_memobj,CL_TRUE,0,contenLength,pDeviceBuffer,1,&evt2,NULL);
    if(err < 0)
    {
        perror("read pdevicebuffer fail");
        exit(1);
    }
    for(int i = 0;i < contenLength/sizeof(int);i++)
    {
        int testData= pHostBuffer[i]+pHostBuffer[i];
        printf("%d\n",pDeviceBuffer[i]);
        testData = testData*pHostBuffer[i]-pHostBuffer[i];
        if(testData != pDeviceBuffer[i])
        {
            printf("error occurred @ %d,result is %d\n",i,pDeviceBuffer[i]);
        }

    }
   
    free(pHostBuffer);
    free(pDeviceBuffer);
    free(kernel_src);  
    clReleaseMemObject(dst_memobj);
    clReleaseMemObject(src1_memobj);
    clReleaseMemObject(src2_memobj);
    printf("program complete\n");
    clReleaseKernel(kernel);
    clReleaseKernel(kernel2);
     printf("program complete\n");
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    printf("program complete\n");
    return 0;

}