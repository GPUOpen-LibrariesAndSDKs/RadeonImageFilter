/*****************************************************************************\
*
*  Module Name    RadeonImageFilters_metals.h
*  Project        RadeonImageFilters
*
*  Description    Radeon Image Filters Interface header
*
*  Copyright 2019 Advanced Micro Devices, Inc.
*
*  All rights reserved. This notice is intended as a precaution against
*  inadvertent publication and does not imply publication or any waiver
*  of confidentiality. The year included in the foregoing notice is the
*  year of creation of the work.
*
\*****************************************************************************/
#pragma once

#ifndef __RADEONIMAGEFILTERS_METAL_H
#define __RADEONIMAGEFILTERS_METAL_H

#include "RadeonImageFilters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
* \brief rifCreateContextFromMetalContext
* Creates the new rif_context object associated with Metal device \p device and queue \p queue.
*
* \param api_version - the current RadeonImageFilters version.
* \param device - the MTLDevice object.
* \param queue - the MTLCommandQueue object.
* \param cache_path - the path to the directory which will contain prebuilded binaries.
* \param out_context - the pointer to rif_context object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_context object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_API_VERSION - if API version is unsupported.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_context is nullptr, or \p device is nullptr.
* \return RIF_ERROR_INVALID_CONTEXT - if context creation failed.
* \return RIF_ERROR_INTERNAL_ERROR - if an internal RadeonImageFiflters error occurs. Send a bug report if such an error occurs.
*/
extern RIF_API_ENTRY rif_int rifCreateContextFromMetalContext(rif_uint64 api_version, void* device, void* queue, rif_char const * cache_path, rif_context * out_context);

/*!
* \brief rifContextCreateImageFromMetalMemory
* Creates the new rif_image object with defined by \p image_desc format associated with rif_context, and MTLBuffer object containing image data.
*
* \param[in] context - a valid rif_context object.
* \param[in] image_desc - the description of image size and image pixels format.
* \param[in] mem - the MTLBuffer with data.
* \param[in] size - the size of \mem in bytes.
* \param[out] out_image - the pointer to rif_image object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_image object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_CONTEXT - if \p context is not a valid rif_context.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_image is nullptr, or \p image_desc is nullptr, or \p mem is nullptr.
* \return RIF_ERROR_UNSUPPORTED_IMAGE_FORMAT - if image format defined by \p image_desc is currently unsupported.
*/
extern RIF_API_ENTRY rif_int rifContextCreateImageFromMetalMemory(rif_context context, rif_image_desc const * image_desc, void* mem, rif_longlong size, rif_image * out_image);


typedef struct _MTLDevice *  MTLDevice;
typedef struct _MTLCommandQueue *  MTLCommandQueue;

#ifdef __cplusplus
}
#endif


#endif // __RADEONIMAGEFILTERS_METAL_H
