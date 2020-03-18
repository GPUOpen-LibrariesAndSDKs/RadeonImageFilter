
/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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

#define ERRCODE -1

#include "../ImageTools/ImageTools.h"

int main()
{
   rif_int status = RIF_SUCCESS;
   rif_context context = nullptr;
   rif_command_queue queue = nullptr;
   rif_image_filter filter = nullptr;

   // First create context and queue
   int deviceCount = 0;
   status = rifGetDeviceCount(BACKEND_TYPE, &deviceCount);
   if (status != RIF_SUCCESS)
   {
      return ERRCODE;
   }

   if (deviceCount > 0 || status)
   {
      status = rifCreateContext(RIF_API_VERSION, BACKEND_TYPE, DEVICE, nullptr, &context);
      if (status != RIF_SUCCESS || !context)
      {
         return ERRCODE;
      }
   }

   rif_image normalsImage = ImageTools::LoadImage("images/normal.exr", context);
   rif_image vNormalsImage = ImageTools::LoadImage("images/v_normal.exr", context);
   rif_image depthImage = ImageTools::LoadImage("images/depth.exr", context);
   rif_image vDepthImage = ImageTools::LoadImage("images/v_depth.exr", context);
   rif_image colorImage = ImageTools::LoadImage("images/color.exr", context);
   rif_image vColorImage = ImageTools::LoadImage("images/v_color.exr", context);
   rif_image transitionImage = ImageTools::LoadImage("images/transition.exr", context);
   rif_image vTransitionImage = ImageTools::LoadImage("images/v_transition.exr", context);
   rif_image outputImage = nullptr;

   if (!(normalsImage && vNormalsImage
      && depthImage && vDepthImage
      && colorImage && vColorImage
      && transitionImage && vTransitionImage))
   {
      return ERRCODE;
   }

   std::vector<rif_image> inputs = { colorImage, normalsImage, depthImage, transitionImage };
   std::vector<float> sigmas = { 0.01f, 0.01f, 0.01f, 0.01f };

   status = rifContextCreateCommandQueue(context, &queue);
   if (status != RIF_SUCCESS || !queue)
   {
      return ERRCODE;
   }

   //Create output image
   rif_image_desc desc;
   size_t retSize;
   rifImageGetInfo(colorImage, RIF_IMAGE_DESC, sizeof(desc), &desc, &retSize);
   desc.type = RIF_COMPONENT_TYPE_UINT8;
   status = rifContextCreateImage(context, &desc, nullptr, &outputImage);
   if (status != RIF_SUCCESS)
   {
      return ERRCODE;
   }

   //Create tone mapping image filter
   std::cout << "Select denoising method:" << std::endl;
   std::cout << "[0] Bilateral" << std::endl;
   std::cout << "[1] Weighted Local Regression" << std::endl;
   std::cout << "[2] Edge Avoided Wavelet" << std::endl;

   int op;
   std::cin >> op;

   switch (op)
   {
   case 0:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_BILATERAL_DENOISE, &filter);
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameterImageArray(filter, "inputs", &inputs[0], inputs.size());
      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      status = rifImageFilterSetParameterFloatArray(filter, "sigmas", &sigmas[0], sigmas.size());

      rifImageFilterSetParameter1u(filter, "radius", 10);
      rifImageFilterSetParameter1u(filter, "inputsNum", inputs.size());

      if (status != RIF_SUCCESS)
      {
         return -1;
      }
      break;
   case 1:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_LWR_DENOISE, &filter);
      if (status != RIF_SUCCESS)
      {
         return ERRCODE;
      }
      rifImageFilterSetParameterImage(filter, "vColorImg", vColorImage);
      rifImageFilterSetParameterImage(filter, "transImg", transitionImage);
      rifImageFilterSetParameterImage(filter, "vTransImg", vTransitionImage);
      rifImageFilterSetParameterImage(filter, "normalsImg", normalsImage);
      rifImageFilterSetParameterImage(filter, "vNormalsImg", vNormalsImage);
      rifImageFilterSetParameterImage(filter, "depthImg", depthImage);
      rifImageFilterSetParameterImage(filter, "vDepthImg", vDepthImage);
      break;
   case 2:
      status = rifContextCreateImageFilter(context, RIF_IMAGE_FILTER_EAW_DENOISE, &filter);
      if (status != RIF_SUCCESS)
      {
         return ERRCODE;
      }
      rifImageFilterSetParameterImage(filter, "normalsImg", normalsImage);
      rifImageFilterSetParameterImage(filter, "depthImg", depthImage);
      rifImageFilterSetParameterImage(filter, "transImg", transitionImage);
      rifImageFilterSetParameterImage(filter, "colorVar", vColorImage);
      break;
   default:
      return ERRCODE;
      break ;
   }
 
   //Attach filter and set parameters
   status = rifCommandQueueAttachImageFilter(queue, filter, colorImage, outputImage);
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
   rifObjectDelete(normalsImage);
   rifObjectDelete(vNormalsImage);
   rifObjectDelete(depthImage);
   rifObjectDelete(vDepthImage);
   rifObjectDelete(colorImage);
   rifObjectDelete(vColorImage);
   rifObjectDelete(transitionImage);
   rifObjectDelete(vTransitionImage);
   rifObjectDelete(outputImage);
   rifObjectDelete(filter);
   rifObjectDelete(queue);
   rifObjectDelete(context);
   return 0;
}