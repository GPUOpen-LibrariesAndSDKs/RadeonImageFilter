
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

#include "../Utils/cmd_parser.h"
#include "AppSettings.h"

AppCliParser::AppCliParser(int argc, char * argv[])
    : m_cmd_parser(argc, argv)
{   }


AppSettings AppCliParser::Parse()
{
    AppSettings s;
    s.path = m_cmd_parser.GetOption("-p", s.path);

    s.colorImg = m_cmd_parser.GetOption("-c", s.colorImg);
    s.albedoImg = m_cmd_parser.GetOption("-a", s.albedoImg);
    s.depthImg = m_cmd_parser.GetOption("-d", s.depthImg);
    s.normalImg = m_cmd_parser.GetOption("-n", s.normalImg);

    if (m_cmd_parser.OptionExists("-o"))
    {
        s.UseColorOnly = true;
    }

    if (m_cmd_parser.OptionExists("-t"))
    {
        s.UseToneMapper = true;
    }


    return s;
}

AppSettings::AppSettings()
{}

int main(int argc, char* argv[])
{
    long a = 0;
    size_t sizea = sizeof(a);

    AppCliParser cliparser(argc, argv);
    AppSettings settings = cliparser.Parse();

    bool useColorOnly = settings.UseColorOnly;

    for (int i = 0; i < 1; ++i)
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



        std::string inputsPath = settings.path;
        auto loadImage = [&context](std::string path, int width, int height, int cnum) -> rif_image
        {

            std::string ext = path.substr(path.find_last_of(".") + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

            rif_image img = nullptr;
            if (ext == "bin")
                img = ImageTools::LoadBinImage(path.c_str(), width, height, cnum, context);
            else
                img = ImageTools::LoadImage(path.c_str(), context);

            if (!img)
            {
                std::cerr << "Couldn't load " << path << std::endl;
            }
            return img;
        };

        const int Height = 600;
        const int Width = 800;


        auto colorImg = loadImage(inputsPath + settings.colorImg, Width, Height, 3);
        if (!colorImg)
        {
            return ERRCODE;
        }

       
        auto normalsImg = loadImage(inputsPath + settings.normalImg, Width, Height, 3);
        if (!normalsImg && !useColorOnly)
        {
            return ERRCODE;
        }
        auto depthImg = loadImage(inputsPath + settings.depthImg, Width, Height, 1);
        if (!depthImg && !useColorOnly)
        {
            return ERRCODE;
        }
        auto albedoImg = loadImage(inputsPath + settings.albedoImg, Width, Height, 3);
        if (!albedoImg && !useColorOnly)
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
        desc.num_components = 3;
        desc.type = RIF_COMPONENT_TYPE_FLOAT32;
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
            RIF_IMAGE_FILTER_AI_DENOISE,
            &denoiseFilter);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        status = rifImageFilterSetParameterString(denoiseFilter, "modelPath", "./models");
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        rif_image_filter remapNormalsFilter = nullptr;
        status = rifContextCreateImageFilter(context,
            RIF_IMAGE_FILTER_REMAP_RANGE,
            &remapNormalsFilter);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }
        rif_image_filter remapDepthFilter = nullptr;
        status = rifContextCreateImageFilter(context,
            RIF_IMAGE_FILTER_REMAP_RANGE,
            &remapDepthFilter);
        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        rif_image_filter toneMapFilter = nullptr;
        rif_image tempToneMapImage = nullptr;

        status = rifImageFilterSetParameterImage(denoiseFilter,
            "colorImg",
            colorImg);


        if (status != RIF_SUCCESS)
        {
            return ERRCODE;
        }

        if (!useColorOnly)
        {
            status = rifImageFilterSetParameterImage(denoiseFilter,
                "normalsImg",
                normalsImg);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameterImage(denoiseFilter,
                "depthImg",
                depthImg);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameterImage(denoiseFilter,
                "albedoImg",
                albedoImg);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameter1f(remapNormalsFilter, "dstLo", -0.0f);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameter1f(remapNormalsFilter, "dstHi", +1.0f);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifCommandQueueAttachImageFilter(queue,
                remapNormalsFilter,
                normalsImg,
                normalsImg);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameter1f(remapDepthFilter, "dstLo", 0.0f);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameter1f(remapDepthFilter, "dstHi", 1.0f);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifCommandQueueAttachImageFilter(queue,
                remapDepthFilter,
                depthImg,
                depthImg);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }
        }///end ColorOnly

        //applying tonemapper before can give better result in some cases
        if (settings.UseToneMapper)
        {
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

            rifImageGetInfo(colorImg, RIF_IMAGE_DESC, sizeof(desc), &desc, &retSize);
            desc.type = RIF_COMPONENT_TYPE_FLOAT32;
            status = rifContextCreateImage(context, &desc, nullptr, &tempToneMapImage);
            if (status != RIF_SUCCESS)
            {
                return -1;
            }

            //Attach filter and set parameters
            status = rifCommandQueueAttachImageFilter(queue, toneMapFilter, colorImg, tempToneMapImage);
            if (status != RIF_SUCCESS)
            {
                return -1;
            }

            status = rifImageFilterSetParameterImage(denoiseFilter,
                "colorImg",
                tempToneMapImage);


            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifCommandQueueAttachImageFilter(queue,
                denoiseFilter,
                tempToneMapImage,
                outputImage);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

        }//end tonemapper

        if (!settings.UseToneMapper)
        {
            status = rifImageFilterSetParameter1u(denoiseFilter, "useHDR", 1);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifImageFilterSetParameterImage(denoiseFilter,
                "colorImg",
                colorImg);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }

            status = rifCommandQueueAttachImageFilter(queue,
                denoiseFilter,
                colorImg,
                outputImage);
            if (status != RIF_SUCCESS)
            {
                return ERRCODE;
            }
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
#define FILTER_EXEC_COUNT 1
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


       auto outputPath = inputsPath + "output_";

        if (settings.UseColorOnly)
            outputPath += "conly_";
        if (settings.UseToneMapper)
            outputPath += "tone_";



        outputPath+= std::to_string(i) + ".hdr";
        std::cout<<"outputPath="<<outputPath<<std::endl;
        //Save output image
        ImageTools::ImageSaveToFile(outputImage, outputPath.c_str());

        //Free resources
        if (!settings.UseColorOnly)
        {
            rifCommandQueueDetachImageFilter(queue, remapNormalsFilter);
            rifCommandQueueDetachImageFilter(queue, remapDepthFilter);
        }
        rifCommandQueueDetachImageFilter(queue, denoiseFilter);

        rifObjectDelete(denoiseFilter);
        rifObjectDelete(remapNormalsFilter);
        rifObjectDelete(remapDepthFilter);
        if (toneMapFilter)
            rifObjectDelete(toneMapFilter);

        if(tempToneMapImage)
            rifObjectDelete(tempToneMapImage);

        rifObjectDelete(colorImg);
        rifObjectDelete(normalsImg);
        rifObjectDelete(depthImg);
        rifObjectDelete(albedoImg);
        rifObjectDelete(outputImage);
        rifObjectDelete(queue);
        rifObjectDelete(context);
    }

    return RIF_SUCCESS;
}
