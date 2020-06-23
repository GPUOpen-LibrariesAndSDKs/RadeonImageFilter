
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
#include <iostream>

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
   rif_image_filter toneMapFilter = nullptr;
   rif_image_filter gammaFilter = nullptr;
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

   //Create tone mapping image filter
   std::cout << "Select tone mapping operator:" << std::endl;
   std::cout << "[0] Linear tone mapping" << std::endl;
   std::cout << "[1] Exponential tone mapping" << std::endl;
   std::cout << "[2] Reinhard tone mapping" << std::endl;
   std::cout << "[3] Drago tone mapping" << std::endl;
   std::cout << "[4] Filmic tone mapping" << std::endl;

   int op;
   std::cin >> op;

   switch (op)
   {
   case 0:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_LINEAR_TONEMAP, &toneMapFilter);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "cKey", 0.15f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      break;
   case 1:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_EXPONENTIAL_TONEMAP, &toneMapFilter);
      status = rifImageFilterSetParameter1f(toneMapFilter, "cExposure", 0.15f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "cIntensity", 0.15f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      break;
   case 2:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_REINHARD02_TONEMAP, &toneMapFilter);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "preScale", 1.0f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "postScale", 1.2f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "burn", 3.75f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      break;
   case 3:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_DRAGO_TONEMAP, &toneMapFilter);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "avLum", 0.15f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "maxLum", 10.0f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "cBias", 0.0f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      break;
   case 4:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_FILMIC_TONEMAP, &toneMapFilter);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "cExposure", 0.15f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1f(toneMapFilter, "cContrast", 10.0f);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameter1u(toneMapFilter, "cApplyToneMap", 1);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      break;
   default:
      break;
   }

   //Create gamma correction filter
   status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_GAMMA_CORRECTION, &gammaFilter);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }
   status = rifImageFilterSetParameter1f(gammaFilter, "cGamma", 2.2f);
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
   desc.type = RIF_COMPONENT_TYPE_UINT8;
   status = rifContextCreateImage(context, &desc, nullptr, &outputImage);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }

   //Attach filter and set parameters
   status = rifCommandQueueAttachImageFilter(queue, toneMapFilter, inputImage, outputImage);
   if (status != RIF_SUCCESS)
   {
      return -1;
   }

   status = rifCommandQueueAttachImageFilter(queue, gammaFilter, outputImage, outputImage);
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
   rifCommandQueueDetachImageFilter(queue, toneMapFilter);
   rifObjectDelete(inputImage);
   rifObjectDelete(outputImage);
   rifObjectDelete(toneMapFilter);
   rifObjectDelete(queue);
   rifObjectDelete(context);
   return 0;
}
