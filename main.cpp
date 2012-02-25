// OpenCL tutorial 1

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

//opencv header
#include <stdio.h>
#include <cv.h>
#include <highgui.h> 

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif


cl_program load_program(cl_context context, const char* filename)
{
std::ifstream in(filename, std::ios_base::binary);
if(!in.good()) {
return 0;
}

// get file length
in.seekg(0, std::ios_base::end);
size_t length = in.tellg();
in.seekg(0, std::ios_base::beg);

// read program source
std::vector<char> data(length + 1);
in.read(&data[0], length);
data[length] = 0;

// create and build program 
const char* source = &data[0];
cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
if(program == 0) {
return 0;
}

if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
return 0;
}

return program;
}


cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height)
{
    IplImage *Image1 = cvLoadImage(fileName,CV_LOAD_IMAGE_COLOR);
	width = Image1->width;
	height = Image1->height;
    cl_int errNum;
    cl_mem clImage;
	cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;
    clImage = clCreateImage2D(context,
                            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                            &clImageFormat,
                            width,
                            height,
                            0,
							Image1->imageData,
                            &errNum);

	    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error creating CL image object" << std::endl;
        return 0;
    }




/*
	    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(fileName, 0);
    FIBITMAP* image = FreeImage_Load(format, fileName);

    // Convert to 32-bit image
    FIBITMAP* temp = image;
    image = FreeImage_ConvertTo32Bits(image);
    FreeImage_Unload(temp);

    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);

    char *buffer = new char[width * height * 4];
    memcpy(buffer, FreeImage_GetBits(image), width * height * 4);

    FreeImage_Unload(image);

    // Create OpenCL image
    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;

    cl_int errNum;
    cl_mem clImage;
    clImage = clCreateImage2D(context,
                            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                            &clImageFormat,
                            width,
                            height,
                            0,
                            buffer,
                            &errNum);

    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error creating CL image object" << std::endl;
        return 0;
    }
	*/

    return clImage;
}


int main()
{
cl_int err;
cl_uint num;
err = clGetPlatformIDs(0, 0, &num);
if(err != CL_SUCCESS) {
std::cerr << "Unable to get platforms\n";
return 0;
}

std::vector<cl_platform_id> platforms(num);
err = clGetPlatformIDs(num, &platforms[0], &num);
if(err != CL_SUCCESS) {
std::cerr << "Unable to get platform ID\n";
return 0;
}

cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 };
cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
if(context == 0) {
std::cerr << "Can't create OpenCL context\n";
return 0;
}

size_t cb;
clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
std::string devname;
devname.resize(cb);
clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
std::cout << "Device: " << devname.c_str() << "\n";

cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
if(queue == 0) {
std::cerr << "Can't create command queue\n";
clReleaseContext(context);
return 0;
}

const int DATA_SIZE = 1048576;
std::vector<float> a(DATA_SIZE), b(DATA_SIZE), res(DATA_SIZE);
for(int i = 0; i < DATA_SIZE; i++) {
a[i] = std::rand();
b[i] = std::rand();
}

cl_mem cl_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * DATA_SIZE, &a[0], NULL);
cl_mem cl_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * DATA_SIZE, &b[0], NULL);
cl_mem cl_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * DATA_SIZE, NULL, NULL);
if(cl_a == 0 || cl_b == 0 || cl_res == 0) {
std::cerr << "Can't create OpenCL buffer\n";
clReleaseMemObject(cl_a);
clReleaseMemObject(cl_b);
clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);
return 0;
}

cl_program program = load_program(context, "shader.cl");
if(program == 0) {
std::cerr << "Can't load or build program\n";
clReleaseMemObject(cl_a);
clReleaseMemObject(cl_b);
clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);
return 0;
}

cl_kernel adder = clCreateKernel(program, "adder", 0);
if(adder == 0) {
std::cerr << "Can't load kernel\n";
clReleaseProgram(program);
clReleaseMemObject(cl_a);
clReleaseMemObject(cl_b);
clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);
return 0;
}

clSetKernelArg(adder, 0, sizeof(cl_mem), &cl_a);
clSetKernelArg(adder, 1, sizeof(cl_mem), &cl_b);
clSetKernelArg(adder, 2, sizeof(cl_mem), &cl_res);

size_t work_size = DATA_SIZE;
err = clEnqueueNDRangeKernel(queue, adder, 1, 0, &work_size, 0, 0, 0, 0);

if(err == CL_SUCCESS) {
err = clEnqueueReadBuffer(queue, cl_res, CL_TRUE, 0, sizeof(float) * DATA_SIZE, &res[0], 0, 0, 0);
}

if(err == CL_SUCCESS) {
bool correct = true;
for(int i = 0; i < DATA_SIZE; i++) {
if(a[i] + b[i] != res[i]) {
correct = false;
break;
}
}

if(correct) {
std::cout << "Data is correct\n";




//opencv test
/*
    char FileName[10]="aaa.jpg";

    IplImage *Image1 = cvLoadImage(FileName,CV_LOAD_IMAGE_COLOR);



    cvNamedWindow("Show Image",0);

    cvResizeWindow("Show Image",300,400);


	CvScalar s = cvGet2D(Image1, 5,5);
		//CvScalar s1 =cvScalar(255,255,255);
		for(int i = 0;i <500;i++)
			for(int j = 0;j< 500;j++)
	 cvSet2D(Image1,i,j,s);

    cvShowImage("Show Image",Image1);

	


	printf("%lf %lf %lf %lf\n",s.val[0],s.val[1],s.val[2],s.val[3]);

    cvWaitKey(0); 

    cvDestroyWindow("Show Image");

    cvReleaseImage(&Image1);


	*/










}
else {
std::cout << "Data is incorrect\n";
}
}
else {
std::cerr << "Can't run kernel or read back data\n";
}

clReleaseKernel(adder);
clReleaseProgram(program);
clReleaseMemObject(cl_a);
clReleaseMemObject(cl_b);
clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);

std::system("PAUSE");

return 0;
}