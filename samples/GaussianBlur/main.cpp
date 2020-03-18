
/**********************************************************************
Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/

#include "RadeonImageFilters.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DEVICE 0

#ifdef RIF_USE_METAL
#define BACKEND_TYPE RIF_BACKEND_API_METAL
#else
#define BACKEND_TYPE RIF_BACKEND_API_OPENCL
#endif // RIF_USE_METAL

#include "../ImageTools/ImageTools.h"

int main()
{
   rif_int status = RIF_SUCCESS;
   rif_context context = nullptr;
   rif_command_queue queue = nullptr;
   rif_image_filter filter = nullptr;
   rif_image inputImage = nullptr;
   rif_image outputImage = nullptr;

   // First create context and queue
   int deviceCount = 0;
   status = rifGetDeviceCount(BACKEND_TYPE, &deviceCount);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }
   if (deviceCount > 0 || status)
   {
      status = rifCreateContext(RIF_API_VERSION, BACKEND_TYPE, DEVICE, nullptr, &context);
      if (status != RIF_SUCCESS || !context)
      {
         return -1;
      }
   }
   status = rifContextCreateCommandQueue(context, &queue);
   if (status != RIF_SUCCESS || !queue)
   {
      return -1;
   }

   //Create image filter
   status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_GAUSSIAN_BLUR, &filter);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }

   // Load input image
   inputImage = ImageTools::LoadImage("images/input.png", context);
   if (!inputImage)
   {
      return -1;
   }

   //Create output image
   rif_image_desc desc;
   size_t retSize;
   rifImageGetInfo(inputImage, RIF_IMAGE_DESC, sizeof(desc), &desc, &retSize);
   status = rifContextCreateImage(context, &desc, nullptr, &outputImage);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }

   //Attach filter and set parameters
   status = rifImageFilterSetParameter1u(filter, "radius", 3);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }
   status = rifImageFilterSetParameter1f(filter, "sigma", 2.0f);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }
   status = rifCommandQueueAttachImageFilter(queue, filter, inputImage, outputImage);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }

   //Execute queue
   status = rifContextExecuteCommandQueue(context, queue, nullptr, nullptr, nullptr);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }

   //Save output image
   ImageTools::SaveImage(outputImage, "out.png");

   //Free resources
   rifCommandQueueDetachImageFilter(queue, filter);
   rifObjectDelete(inputImage);
   rifObjectDelete(outputImage);
   rifObjectDelete(filter);
   rifObjectDelete(queue);
   rifObjectDelete(context);
   return 0;
}