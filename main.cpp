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
	std::cerr<<"ifstream in not goot"<<std::endl;
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
	std::cerr<<"clCreateProgramwithSource fault"<<std::endl;
return 0;
}

if(clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) {
	std::cerr<<"clBuildProgram fault"<<std::endl;
return 0;
}

return program;
}


cl_mem LoadImage(cl_context context, char *fileName, int &width, int &height)
{
    IplImage *Image1 = cvLoadImage(fileName,1);
	IplImage *ImageDst = cvCreateImage(cvGetSize(Image1), IPL_DEPTH_8U ,4);;
	width = Image1->width;
	height = Image1->height;
	printf("Image name: %s , width:%d, height:%d\n",fileName,width,height);
    
	cvCvtColor(Image1, ImageDst ,CV_RGB2RGBA);//testing

	 //show image
	    cvNamedWindow("Show Image",0);

    cvResizeWindow("Show Image",800,800);
    cvShowImage("Show Image",Image1);
	std::cout<<"This is origional image, press any key to do opencl process"<<std::endl;
    cvWaitKey(0); 
	std::cout<<"processing"<<std::endl;
    cvDestroyWindow("Show Image");
    cvReleaseImage(&Image1);

	//*/
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
							ImageDst->imageData,
                            &errNum);

	    if (errNum != CL_SUCCESS)
    {
        std::cerr << "Error creating CL image object" <<errNum<< std::endl;
        return 0;
    }

//		printf("%s\n",clImage);
//	printf("--------------------------------------------------\n");


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
cl_mem imageObjects[2] = { 0, 0 };
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

/*
const int DATA_SIZE =1048576;
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
*/


int width,height;
imageObjects[0] = LoadImage(context, "aaa.jpg", width, height);
   if (imageObjects[0] == 0)
    {
        std::cerr << "Error loading: aaa.jpg"<< std::endl;
        return 1;
    }

    // Create ouput image object
    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;
    imageObjects[1] = clCreateImage2D(context,
                                       CL_MEM_WRITE_ONLY,
                                       &clImageFormat,
                                       width,
                                       height,
                                       0,
                                       NULL,
                                       &err);

    if (err != CL_SUCCESS)
    {
        std::cerr << "Error creating CL output image object." << std::endl;
        return 1;
    }




cl_program program = load_program(context, "shader.cl");
if(program == 0) {
std::cerr << "Can't load or build program\n";
//clReleaseMemObject(cl_a);
//clReleaseMemObject(cl_b);
//clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);
return 0;
}

cl_kernel adder = clCreateKernel(program, "adder", 0);
if(adder == 0) {
std::cerr << "Can't load kernel\n";
clReleaseProgram(program);
//clReleaseMemObject(cl_a);
//clReleaseMemObject(cl_b);
//clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);
return 0;
}


    // Create sampler for sampling image object
    cl_sampler sampler = clCreateSampler(context,
                              CL_FALSE, // Non-normalized coordinates
                              CL_ADDRESS_CLAMP_TO_EDGE,
                              CL_FILTER_NEAREST,
                              &err);

    if (err != CL_SUCCESS)
    {
        std::cerr << "Error creating CL sampler object." << std::endl;
      
        return 1;
    }



//set arg
    err = clSetKernelArg(adder, 0, sizeof(cl_mem), &imageObjects[0]);
    err |= clSetKernelArg(adder, 1, sizeof(cl_mem), &imageObjects[1]);
    err |= clSetKernelArg(adder, 2, sizeof(cl_sampler), &sampler);
    err |= clSetKernelArg(adder, 3, sizeof(cl_int), &width);
    err |= clSetKernelArg(adder, 4, sizeof(cl_int), &height);

    if (err != CL_SUCCESS)
    {
        std::cerr << "Error setting kernel arguments." << std::endl;
        system("PAUSE");
        return 1;
    }
//clSetKernelArg(adder, 0, sizeof(cl_mem), &cl_a);
//clSetKernelArg(adder, 1, sizeof(cl_mem), &cl_b);
//clSetKernelArg(adder, 2, sizeof(cl_mem), &cl_res);
    size_t globalWorkSize[2] =  { width,
                                  height };

    // Queue the kernel up for execution
    err = clEnqueueNDRangeKernel(queue, adder, 2, NULL,
                                    globalWorkSize, 0,
                                    0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error queuing kernel for execution." << std::endl;
        //Cleanup(context, commandQueue, program, kernel, imageObjects, sampler);
        return 1;
    }



	    // Read the output buffer back to the Host
    char *buffer = new char [width * height * 4];
    size_t origin[3] = { 0, 0, 0 };
    size_t region[3] = { width, height, 1};
    err = clEnqueueReadImage(queue, imageObjects[1], CL_TRUE,
                                origin, region, 0, 0, buffer,
                                0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        std::cerr << "Error reading result buffer." << std::endl;
       // Cleanup(context, commandQueue, program, kernel, imageObjects, sampler);
        return 1;
    }

	
	 //show image
	IplImage *outImage = cvCreateImageHeader(cvSize(width,height),IPL_DEPTH_8U,4);
	  cvSetData(outImage,buffer,width*4);
	
	
	/*/convert RGBA to BGR
	IplImage *bgrImage = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
  
    cvCvtColor(outImage, bgrImage ,CV_RGBA2BGR);//testing
	//*/
		
           /*/ debug use 
		for(int i = 1900;i <height;i++)
			for(int j = 2250;j< width;j++)
			{
				CvScalar s = cvGet2D(bgrImage, i,j);
				printf("%lf %lf %lf %lf i = %d , j = %d\n",s.val[0],s.val[1],s.val[2],s.val[3],i,j);
			}

			/*/



	    cvNamedWindow("Show Image",0);

    cvResizeWindow("Show Image",800,800);


    cvShowImage("Show Image",outImage);

	std::cout<<"grayscale image by opencl"<<std::endl;
    cvWaitKey(0); 

    cvDestroyWindow("Show Image");

//    cvReleaseImage(&outImage);

	//*/






	  std::cout << std::endl;
    std::cout << "Executed program succesfully." << std::endl;
	
	/*
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


	




}
else {
std::cout << "Data is incorrect\n";
}
}
else {
std::cerr << "Can't run kernel or read back data\n";
}
*/
clReleaseKernel(adder);
clReleaseProgram(program);
//clReleaseMemObject(cl_a);
//clReleaseMemObject(cl_b);
//clReleaseMemObject(cl_res);
clReleaseCommandQueue(queue);
clReleaseContext(context);

std::system("PAUSE");

return 0;
}