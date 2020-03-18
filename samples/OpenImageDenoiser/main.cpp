
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


#define VERBOSE
#include "RadeonImageFilters.h"
#include <iostream>

#include <chrono>

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
#include <thread>

int main(int argc, char* argv[])
{
    for (int i = 0; i < 1/*5*/; ++i)
    {
        rif_int status = RIF_SUCCESS;

        int deviceCount = 0;
        status = rifGetDeviceCount(BACKEND_TYPE,
                                   &deviceCount);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        rif_context context = nullptr;
        if (deviceCount > 0 || status)
        {
            status = rifCreateContext(RIF_API_VERSION,
                                      BACKEND_TYPE,
                                      DEVICE,
                                      nullptr,
                                      &context);
            if (status != RIF_SUCCESS || !context)
            {
                std::cerr << "Error creating context" << std::endl;
                return ERRCODE;
            }
        }



        std::string inputsPath = "images/";

        auto loadImage = [&context](std::string path, int width, int height, int cnum) -> rif_image
        {
            auto img = ImageTools::LoadBinImage(path.c_str(), width, height, cnum, context);
            if (!img)
            {
                std::cerr << "Couldn't load " << path << std::endl;
            }
            return img;
        };

        const int Height = 600;
        const int Width = 800;


        // 4 - Color

        auto colorImg = loadImage(inputsPath + "cam_12_color_spp_8.bin", Width, Height, 3);
        if (!colorImg)
        {
            return ERRCODE;
        }



        // 5 - albedo
        auto albedoImg = loadImage(inputsPath + "cam_12_albedo_spp_8.bin", Width, Height, 3);
        if (!albedoImg)
        {
            return ERRCODE;
        }


        // 6 - normals
        auto normalsImg = loadImage(inputsPath + "cam_12_view_shading_normal.bin", Width, Height, 3);
        if (!normalsImg)
        {
            return ERRCODE;
        }


        rif_command_queue queue = nullptr;
        status = rifContextCreateCommandQueue(context,
                                              &queue);
        if (status != RIF_SUCCESS || !queue)
        {
            return ERRCODE;
        }


        size_t retSize;
        rif_image_desc desc;
        rifImageGetInfo(colorImg,
                        RIF_IMAGE_DESC,
                        sizeof(desc),
                        &desc,
                        &retSize);


        //Create output image
        rif_image outputImage = nullptr;
        status = rifContextCreateImage(context,
                                       &desc,
                                       nullptr,
                                       &outputImage);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }


        rif_image_filter denoiseFilter = nullptr;
        status = rifContextCreateImageFilter(context,
                                             RIF_IMAGE_FILTER_OPENIMAGE_DENOISE,
                                             &denoiseFilter);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // Number of CPU threads used (if = 0, OpenImage Denoise will set it automatically to get the best performance)
        status = rifImageFilterSetParameter1u(denoiseFilter, "numThreads", 0u);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // Bind software theads to hardware threads (=1u) or not (=0u)
        status = rifImageFilterSetParameter1u(denoiseFilter, "threadsBinding", 1u);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // The image is HDR (=1u) or not HDR (=0u)
        status = rifImageFilterSetParameter1u(denoiseFilter, "is_HDR", 1u);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // The image is sRGB (=1u) or not sRGB (=0u)
        status = rifImageFilterSetParameter1u(denoiseFilter, "is_sRGB", 0u);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // 4 - Color
        status = rifImageFilterSetParameterImage(denoiseFilter,
                                                 "colorImg",
            colorImg);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // 5 - albedo
        status = rifImageFilterSetParameterImage(denoiseFilter,
                                                 "albedoImg",
            albedoImg);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // 6 - normals
        status = rifImageFilterSetParameterImage(denoiseFilter,
                                                 "normalsImg",
            normalsImg);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        //Attach filter
        status = rifCommandQueueAttachImageFilter(queue,
                                                  denoiseFilter,
                                                  colorImg,
                                                  outputImage);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        //Warmup
        status = rifContextExecuteCommandQueue(context,
                                               queue,
                                               nullptr,
                                               nullptr,
                                               nullptr);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        //Execute queue
        rifSyncronizeQueue(queue);
        auto  start = std::chrono::high_resolution_clock::now();
#define FILTER_EXEC_COUNT 100
        for (int i = 0; i < FILTER_EXEC_COUNT; ++i)
        {
           status = rifContextExecuteCommandQueue(context,
                                                  queue,
                                                  nullptr,
                                                  nullptr,
                                                  nullptr);
           if (status != RIF_SUCCESS)
           {
               return ERRCODE;
           }
        }
        rifSyncronizeQueue(queue);
        auto end = std::chrono::high_resolution_clock::now();

        rif_image_desc outDesc;
        retSize = 0;
        status = rifImageGetInfo(outputImage,
                                 RIF_IMAGE_DESC,
                                 sizeof(rif_image_desc),
                                 &outDesc,
                                 &retSize);

        auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cerr << "RIF inference time: " << elapsed_ms/FILTER_EXEC_COUNT << " us. Resolution is : "<< outDesc.image_height<<"x"<< outDesc.image_width << std::endl;

        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        // Normalize image data
        float* Pixels = nullptr;
        status = rifImageMap(outputImage,
                             RIF_IMAGE_MAP_READ | RIF_IMAGE_MAP_WRITE,
                             reinterpret_cast<void**>(&Pixels));
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        int input_h = outDesc.image_height;
        int input_w = outDesc.image_width;
        int input_c = outDesc.num_components;
        int input_hStride = input_w * input_c;
        int input_wStride = input_c;
        int input_cStride = 1;
        //int input_cStride = outDesc.image_height * outDesc.image_width;
        //int input_hStride = outDesc.image_width;
        //int input_wStride = 1;

        std::vector<float> val_min(input_c, std::numeric_limits<float>::max());
        std::vector<float> val_max(input_c, -std::numeric_limits<float>::max());
        for(int ih = 0; ih < input_h; ++ih)
        {
            for(int iw = 0; iw < input_w; ++iw)
            {
                for(int ic=0; ic < input_c; ++ic)
                {
                    int index_src = ih * input_hStride + iw * input_wStride + ic * input_cStride;
                    float x = Pixels[index_src];
                    val_min[ic] = (x < val_min[ic]) ? x : val_min[ic];
                    val_max[ic] = (x > val_max[ic]) ? x : val_max[ic];
                }
            }
        }
        std::vector<float> val_range(input_c, 0);
        for(int ic=0; ic < input_c; ++ic)
        {
            val_range[ic] = val_max[ic] - val_min[ic];
            std::cout << "val_min-[" << ic << "]=" << val_min[ic] << std::endl;
            std::cout << "val_max-[" << ic << "]=" << val_max[ic] << std::endl;
            std::cout << "val_range-[" << ic << "]=" << val_range[ic] << std::endl;
        }
        auto outputPath = inputsPath + "output_" + std::to_string(i) + ".ppm";
#ifdef VERBOSE
        std::ofstream output(outputPath, std::ios::out | std::ios::binary);
        output << "P6\n" << input_w << " " << input_h << "\n255\n";
#endif
        for(int ih = 0; ih < input_h; ++ih)
        {
            for(int iw = 0; iw < input_w; ++iw)
            {
                for(int ic=0; ic < input_c; ++ic)
                {
                    int index_src = ih * input_hStride + iw * input_wStride + ic * input_cStride;
                    float x = (Pixels[index_src] - val_min[ic]) / val_range[ic];
                    Pixels[index_src] = x;
#ifdef VERBOSE
                    output << static_cast<unsigned char>(x * 255.);
                    //output << static_cast<unsigned char>(((Pixels[index_src] - val_min[ic]) / val_range[ic]) * 255);
                    //output << static_cast<unsigned char>(std::ceil(((Pixels[index_src] - val_min) / val_range) * 255));
#endif
                }
            }
        }
#ifdef VERBOSE
        output.close();
#endif
        status = rifImageUnmap(outputImage, Pixels);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        outputPath = inputsPath + "output_" + std::to_string(i) + ".hdr";
        std::cout<<"outputPath="<<outputPath<<std::endl;
        //Save output image
        ImageTools::ImageSaveToFile(outputImage, outputPath.c_str());

        //Free resources
        rifCommandQueueDetachImageFilter(queue, denoiseFilter);

        rifObjectDelete(denoiseFilter);

        // 4 - Color
        rifObjectDelete(colorImg);
        // 5 - albedo
        rifObjectDelete(albedoImg);
        // 6 - normals
        rifObjectDelete(normalsImg);
        rifObjectDelete(outputImage);
        rifObjectDelete(queue);
        rifObjectDelete(context);
    }

    return RIF_SUCCESS;
}
