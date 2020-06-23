#pragma once
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#include "Half/half.hpp"
#include <string>
#include <iostream>
#include <fstream>

namespace ImageTools
{


    int LoadEXRLikeTiny(float **out_rgba, int *width, int *height, const char *filename,
        const char **err)
    {
        if (out_rgba == NULL)
        {
            tinyexr::SetErrorMessage("Invalid argument for LoadEXR()", err);
            return TINYEXR_ERROR_INVALID_ARGUMENT;
        }

        EXRVersion exr_version;
        EXRImage exr_image;
        EXRHeader exr_header;
        InitEXRHeader(&exr_header);
        InitEXRImage(&exr_image);

        {
            int ret = ParseEXRVersionFromFile(&exr_version, filename);
            if (ret != TINYEXR_SUCCESS) 
            {
                tinyexr::SetErrorMessage("Invalid EXR header.", err);
                return ret;
            }

            if (exr_version.multipart || exr_version.non_image) 
            {
                tinyexr::SetErrorMessage( "Loading multipart or DeepImage is not supported  in LoadEXR() API", err);
                return TINYEXR_ERROR_INVALID_DATA;  // @fixme.
            }
        }

        {
            int ret = ParseEXRHeaderFromFile(&exr_header, &exr_version, filename, err);
            if (ret != TINYEXR_SUCCESS)
            {
                FreeEXRHeader(&exr_header);
                return ret;
            }
        }

        // Read HALF channel as FLOAT.
        for (int i = 0; i < exr_header.num_channels; i++)
        {
            if (exr_header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF)
            {
                exr_header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
            }
        }


        int ret = LoadEXRImageFromFile(&exr_image, &exr_header, filename, err);
        if (ret != TINYEXR_SUCCESS)
        {
            FreeEXRHeader(&exr_header);
            return ret;
        }


        // RGBA
        int idxR = -1;
        int idxG = -1;
        int idxB = -1;
        int idxA = -1;
        int idxY = -1;

        for (int c = 0; c < exr_header.num_channels; c++)
        {
            if (strcmp(exr_header.channels[c].name, "R") == 0) 
                idxR = c;
            
            else if (strcmp(exr_header.channels[c].name, "G") == 0) 
                idxG = c;
            
            else if (strcmp(exr_header.channels[c].name, "B") == 0) 
                idxB = c;
            
            else if (strcmp(exr_header.channels[c].name, "A") == 0) 
                idxA = c;
            
            else if (strcmp(exr_header.channels[c].name, "Y") == 0) 
                idxY = c;
            
        }

        if (exr_header.num_channels == 1)
        {
            // Alpha channel only.

            if (exr_header.tiled) 
            {
                // todo.implement this
            }

            (*out_rgba) = reinterpret_cast<float *>(
                malloc(4 * sizeof(float) * static_cast<size_t>(exr_image.width) *
                    static_cast<size_t>(exr_image.height)));
          
            for (int i = 0; i < exr_image.width * exr_image.height; i++)
            {
                const float val = reinterpret_cast<float **>(exr_image.images)[0][i];
                (*out_rgba)[4 * i + 0] = val;
                (*out_rgba)[4 * i + 1] = val;
                (*out_rgba)[4 * i + 2] = val;
                (*out_rgba)[4 * i + 3] = val;
            }
        }

        else
        {
            // Assume RGB(A)

            if (idxR == -1)
            {
                tinyexr::SetErrorMessage("R channel not found", err);

                // @todo { free exr_image }
                FreeEXRHeader(&exr_header);
                return TINYEXR_ERROR_INVALID_DATA;
            }

            if (idxG == -1)
            {
                tinyexr::SetErrorMessage("G channel not found", err);
                // @todo { free exr_image }
                FreeEXRHeader(&exr_header);
                return TINYEXR_ERROR_INVALID_DATA;
            }

            if (idxB == -1)
            {
                tinyexr::SetErrorMessage("B channel not found", err);
                // @todo { free exr_image }
                FreeEXRHeader(&exr_header);
                return TINYEXR_ERROR_INVALID_DATA;
            }

            (*out_rgba) = reinterpret_cast<float *>(
                malloc(4 * sizeof(float) * static_cast<size_t>(exr_image.width) *
                    static_cast<size_t>(exr_image.height)));

            if (exr_header.tiled)
            {
                for (int it = 0; it < exr_image.num_tiles; it++) 
                {
                    for (int j = 0; j < exr_header.tile_size_y; j++)
                        for (int i = 0; i < exr_header.tile_size_x; i++) 
                        {
                            const int ii =
                                exr_image.tiles[it].offset_x * exr_header.tile_size_x + i;
                            const int jj =
                                exr_image.tiles[it].offset_y * exr_header.tile_size_y + j;
                            const int idx = ii + jj * exr_image.width;

                            // out of region check.
                            if (ii >= exr_image.width) 
                                continue;
                            
                            if (jj >= exr_image.height) 
                                continue;
                            
                            const int srcIdx = i + j * exr_header.tile_size_x;
                            unsigned char **src = exr_image.tiles[it].images;
                            (*out_rgba)[4 * idx + 0] =
                                reinterpret_cast<float **>(src)[idxR][srcIdx];
                            (*out_rgba)[4 * idx + 1] =
                                reinterpret_cast<float **>(src)[idxG][srcIdx];
                            (*out_rgba)[4 * idx + 2] =
                                reinterpret_cast<float **>(src)[idxB][srcIdx];

                            if (idxA != -1) 
                            {
                                (*out_rgba)[4 * idx + 3] =
                                    reinterpret_cast<float **>(src)[idxA][srcIdx];
                            }
                            else
                            {
                                (*out_rgba)[4 * idx + 3] = 1.0;
                            }
                        }
                }
            }
            else
            {
                for (int i = 0; i < exr_image.width * exr_image.height; i++)
                {
                    (*out_rgba)[4 * i + 0] =
                        reinterpret_cast<float **>(exr_image.images)[idxR][i];
                    (*out_rgba)[4 * i + 1] =
                        reinterpret_cast<float **>(exr_image.images)[idxG][i];
                    (*out_rgba)[4 * i + 2] =
                        reinterpret_cast<float **>(exr_image.images)[idxB][i];

                    if (idxA != -1)
                    {
                        (*out_rgba)[4 * i + 3] =
                            reinterpret_cast<float **>(exr_image.images)[idxA][i];
                    }
                    else 
                    {
                        (*out_rgba)[4 * i + 3] = 1.0;
                    }
                }
            }
        }

        (*width) = exr_image.width;
        (*height) = exr_image.height;

        FreeEXRHeader(&exr_header);
        FreeEXRImage(&exr_image);

        return TINYEXR_SUCCESS;
    }



struct ImageJpg
{
    int Width = 0;
    int Height = 0;
    int BPP = 0;
    std::vector<float> Pixels;
};

void ReadJpg(const std::string & path, ImageJpg & image)
{
    unsigned char* data = stbi_load(path.c_str(), &image.Width, &image.Height, &image.BPP, 3);

    if (data == nullptr)
    {
        // std::exception does not have a constructor that takes a string argument
        // this is an MSVC extension
        //throw std::exception("file not found \n");
        throw std::runtime_error("file not found\n");
    }

    image.Pixels.resize(image.Width * image.Height * image.BPP);

    for (int i = 0; i < image.Width * image.Height * image.BPP; ++i)
    {
        image.Pixels[i] = static_cast<float>(data[i]) / 255.f;
    }

    stbi_image_free(data);
}

void WriteJpg(const std::string & path, ImageJpg & image)
{
    std::vector< unsigned char> data(image.Width * image.Height * image.BPP);
    for (int i = 0; i < image.Width * image.Height * image.BPP; ++i)
    {
        data[i] = image.Pixels[i] * 255;
    }

    stbi_write_jpg(path.c_str(), image.Width, image.Height, 3, data.data(), 0);
}

template <typename dstT, typename srcT>
void CopyAndConvert(dstT* dst, srcT* src, size_t size, dstT factor, bool clamp = false)
{
    for (int i = 0; i < size; ++i)
    {
        float val = (float)src[i] * factor;
        if (clamp)
        {
            val = (val > factor) ? factor : val;
            val = (val < 0) ? 0 : val;
        }
        dst[i] = (dstT)(val);
    }
}

rif_image LoadBinImage(const std::string& path, int width, int height, int cnum, rif_context context, rif_component_type type = RIF_COMPONENT_TYPE_FLOAT32)
{
    rif_image img = nullptr;

    rif_image_desc desc;
    memset(&desc, 0, sizeof(desc));
    desc.type = type;

    std::ifstream input(path, std::ios::binary);
    std::vector<unsigned char> buf(std::istreambuf_iterator<char>(input), {});

    // set image descriptor
    desc.image_width = width;
    desc.image_height = height;
    desc.num_components = cnum;

    // create rifImage
    rif_int status = rifContextCreateImage(context, &desc, buf.data(), &img);
    if (status != RIF_SUCCESS)
    {
        return nullptr;
    }
    else
    {
        return img;
    }
}

rif_image LoadImage(const std::string& path, rif_context context)
{
   rif_image img = nullptr;

   int width, height, num;

   // load reference data and convert format to type format
   std::string ext = path.substr(path.find_last_of(".") + 1);
   std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);


   float* data = nullptr;
   rif_image_desc desc;
   memset(&desc, 0, sizeof(desc));

   if (ext == "hdr" || ext == "exr")
   {
      float* rawData = nullptr;
      if (ext == "hdr")
      {
         rawData = stbi_loadf(path.c_str(), &width, &height, &num, 0);
      }
      else
      {
         const char* err = nullptr;
         int ret = LoadEXRLikeTiny(&rawData, &width, &height, path.c_str(), &err);

         if (ret != TINYEXR_SUCCESS)
         {
            return nullptr;
         }

         num = 4;
         int arraySize = width * height * num;
      }

      if (!rawData)
      {
         return nullptr;
      }
      int arraySize = width * height * num;

      data = new float[arraySize];
      CopyAndConvert<float, float>((float*)data, rawData, arraySize, 1.f);
      desc.type = RIF_COMPONENT_TYPE_FLOAT32;
      if (ext == "exr")
      {
         free(rawData);
         rawData = nullptr;
      }
   }
   else if (ext == "jpg")
   {
      ImageJpg colorImg;

      ReadJpg(path.c_str(), colorImg);
      width = colorImg.Width;
      height = colorImg.Height;
      num = colorImg.BPP;
      int arraySize = width * height;
      data = new float[arraySize * num];
      CopyAndConvert<float, float>((float*)data, &colorImg.Pixels[0], width*height*num, 1.f);
      desc.type = RIF_COMPONENT_TYPE_FLOAT32;
   }
   else if (ext == "bin")
   {
      return nullptr;
   }
   else
   {
      data = reinterpret_cast<float*>(stbi_load(path.c_str(), &width, &height, &num, 0));
      if (!data)
      {
         return nullptr;
      }
      desc.type = RIF_COMPONENT_TYPE_UINT8;
   }

   // set image descriptor
   desc.image_width = width;
   desc.image_height = height;
   desc.num_components = num;

   // create rifImage
   rif_int status = rifContextCreateImage(context, &desc, data, &img);

   if (ext == "exr" || ext == "jpg")
   {
      delete[] data;
      data = nullptr;
   }

   if (status != RIF_SUCCESS)
   {
      return nullptr;
   }
   else
   {
      return img;
   }
}

bool SaveImage(rif_image img, const std::string& path)
{
    rif_image_desc desc;
    size_t retSize;
    rif_int status = rifImageGetInfo(img, RIF_IMAGE_DESC, sizeof(desc), &desc, &retSize);
    if (status != RIF_SUCCESS)
    {
        return false;
    }

    // get image data
    rif_uchar* data;
    status = rifImageMap(img, RIF_IMAGE_MAP_READ, (void**)&data);
    if (status != RIF_SUCCESS || !data)
    {
        return false;
    }

    // write image to disk
    stbi_write_png(path.c_str(), desc.image_width, desc.image_height, desc.num_components, data, 0);
    status = rifImageUnmap(img, data);
    if (status != RIF_SUCCESS)
    {
        return false;
    }

    return true;
}

rif_uint SaveToFile(const char* path,
                    std::string& ext,
                    rif_uint w,
                    rif_uint h,
                    rif_uint n,
                    void* data)
{
    rif_int err = 1;
    if (ext == "png")
        err = stbi_write_png(path, w, h, n, data, 0);
    else if (ext == "bmp")
        err = stbi_write_bmp(path, w, h, n, data);
    else if (ext == "tga")
        err = stbi_write_tga(path, w, h, n, data);
    else if (ext == "hdr")
        err = stbi_write_hdr(path, w, h, n, (float*) data);
    else
        return RIF_ERROR_UNSUPPORTED;

    if (err)
        return RIF_SUCCESS;
    else
        return RIF_ERROR_IO_ERROR;
}

rif_int SaveImageData(void* data,
                      const char* path,
                      rif_uint w,
                      rif_uint h,
                      rif_uint n,
                      rif_component_type type)
{
    rif_int err = 0;
    std::string ext = std::string(path);
    ext = ext.substr(ext.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    size_t arraySize = w * h * n;

    //float type output file
    if (ext == "hdr" || ext == "exr")
    {
        float* rawData = new float[arraySize];
        switch (type)
        {
        case RIF_COMPONENT_TYPE_UINT8:
            CopyAndConvert<float, rif_uchar>(rawData, (rif_uchar*) data, arraySize, 1.f / 255.0f);
            break;
        case RIF_COMPONENT_TYPE_FLOAT32:
            CopyAndConvert<float, float>(rawData, (float*) data, arraySize, 1.f);
            break;
        case RIF_COMPONENT_TYPE_FLOAT16:
            CopyAndConvert<float, half_float::half>(rawData, (half_float::half*) data, arraySize, 1.f);
            break;
        default:
            return RIF_ERROR_UNSUPPORTED;
        }
        err = SaveToFile(path, ext, w, h, n, rawData);
        delete[] rawData;
    }
    else if (ext == "png" || ext == "bmp" || ext == "tga") // unsigned int8  type output file
    {
        rif_uchar* rawData = new rif_uchar[arraySize];
        switch (type)
        {
        case RIF_COMPONENT_TYPE_UINT8:
            CopyAndConvert<rif_uchar, rif_uchar>(rawData, (rif_uchar*) data, arraySize, 1U);
            break;
        case RIF_COMPONENT_TYPE_FLOAT32:
            CopyAndConvert<rif_uchar, float>(rawData, (float*) data, arraySize, 255U, true);
            break;
        case RIF_COMPONENT_TYPE_FLOAT16:
            CopyAndConvert<rif_uchar, half_float::half>(rawData, (half_float::half*) data, arraySize, 255U, true);
            break;
        default:
            return RIF_ERROR_UNSUPPORTED;
        }
        err = SaveToFile(path, ext, w, h, n, rawData);
        delete[] rawData;
    }

    return err;
}

rif_int ImageSaveToFile(rif_image in, const char* path)
{
     rif_int err;
     void * data = nullptr;

     err = rifImageMap(in, RIF_IMAGE_MAP_READ, &data);

     if (err != RIF_SUCCESS)
         return err;
     if (!data)
         return RIF_ERROR_INTERNAL_ERROR;

     rif_image_desc desc;
     size_t retSize = 0;
     err = rifImageGetInfo(in, RIF_IMAGE_DESC, sizeof(rif_image_desc), (void*) &desc, &retSize);
     if (err != RIF_SUCCESS)
         return err;

     int rowPitch = desc.image_row_pitch != 0 ? desc.image_row_pitch : desc.image_width;

     err = SaveImageData(data, path, rowPitch, desc.image_height, desc.num_components, desc.type);
     if (err != RIF_SUCCESS)
         return err;

     err = rifImageUnmap(in, data);
     if (err != RIF_SUCCESS)
         return err;

     return RIF_SUCCESS;
}
}
