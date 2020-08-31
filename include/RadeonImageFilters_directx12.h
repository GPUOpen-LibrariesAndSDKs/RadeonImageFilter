/*****************************************************************************\
*
*  Module Name    RadeonImageFilters_directx12.h
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

#ifndef __RADEONIMAGEFILTERS_DIRECTX12_H
#define __RADEONIMAGEFILTERS_DIRECTX12_H

#include "RadeonImageFilters.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
* \brief rifCreateContextFromDirectX12Context
* Creates the new rif_context object associated with DirectX12 device \p device and queue \p queue.
*
* \param api_version - the current RadeonImageFilters version.
* \param device - the DirectX12 device object.
* \param queue - the DirectX12 command queue object.
* \param cache_path - the path to the directory which will contain prebuilded binaries.
* \param out_context - the pointer to rif_context object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_context object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_API_VERSION - if API version is unsupported.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_context is nullptr, or \p device is nullptr.
* \return RIF_ERROR_INTERNAL_ERROR - if an internal RadeonImageFiflters error occurs. Send a bug report if such an error occurs.
*/
extern RIF_API_ENTRY rif_int rifCreateContextFromDirectX12Context(rif_uint64 api_version, void* device, void* queue, rif_char const * cache_path, rif_context * out_context);

/*!
* \brief rifContextCreateImageFromDirectX12Memory
* Creates the new rif_image object with defined by \p image_desc format associated with rif_context, and DirectX12 buffer object containing image data.
*
* \param[in] context - a valid rif_context object.
* \param[in] image_desc - the description of image size and image pixels format.
* \param[in] mem - the DirectX12 buffer with data.
* \param[out] out_image - the pointer to rif_image object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_image object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_CONTEXT - if \p context is not a valid rif_context.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_image is nullptr, or \p image_desc is nullptr, or \p mem is nullptr.
* \return RIF_ERROR_UNSUPPORTED_IMAGE_FORMAT - if image format defined by \p image_desc is currently unsupported.
*/
extern RIF_API_ENTRY rif_int rifContextCreateImageFromDirectX12Memory(rif_context context, rif_image_desc const * image_desc, void* mem, rif_image * out_image);


/*!
* \brief rifCreateOpenCLContextSharedWithDirectX11
* Creates the new rif_context object able to share resources with DirectX11.
*
* \param api_version - the current RadeonImageFilters version.
* \param dx_device - DirectX11 device
* \param cache_path - the path to the directory which will contain prebuilded binaries.
* \param out_context - the pointer to rif_context object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_context object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_API_VERSION - if API version is unsupported.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_context is nullptr, or \p device is nullptr.
* \return RIF_ERROR_INVALID_CONTEXT - if context creation failed.
* \return RIF_ERROR_INTERNAL_ERROR - if an internal RadeonImageFiflters error occurs. Send a bug report if such an error occurs.
*/
extern RIF_API_ENTRY rif_int rifCreateOpenCLContextSharedWithDirectX11(rif_uint64 api_version, void* dx_device, rif_char const * cache_path, rif_context * out_context);

/*!
* \brief rifContextCreateImageFromDirectX11Texture
* Creates the new rif_image object with defined by \p image_desc format associated with rif_context, and DirectX11 texture object containing image data.
*
* \param[in] context - a valid rif_context object.
* \param[in] image_desc - the description of image size and image pixels format.
* \param[in] mem - the DirectX11 texture with data.
* \param[out] out_image - the pointer to rif_image object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_image object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_CONTEXT - if \p context is not a valid rif_context.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_image is nullptr, or \p image_desc is nullptr, or \p mem is nullptr.
* \return RIF_ERROR_UNSUPPORTED_IMAGE_FORMAT - if image format defined by \p image_desc is currently unsupported.
*/
extern RIF_API_ENTRY rif_int rifContextCreateImageFromDirectX11Texture(rif_context context, rif_image_desc const * image_desc, void* mem, rif_image * out_image);

/*!
* \brief rifContextCreateImageFromDirectX11Buffer
* Creates the new rif_image object with defined by \p image_desc format associated with rif_context, and DirectX11 buffer object containing image data.
*
* \param[in] context - a valid rif_context object.
* \param[in] image_desc - the description of image size and image pixels format.
* \param[in] mem - the DirectX11 buffer with data.
* \param[out] out_image - the pointer to rif_image object which will be created, if the function performs successfully.
* \return RIF_SUCCESS - if the rif_image object is created successfully. Otherwise, it returns one of the following errors:
* \return RIF_ERROR_INVALID_CONTEXT - if \p context is not a valid rif_context.
* \return RIF_ERROR_INVALID_PARAMETER - if \p out_image is nullptr, or \p image_desc is nullptr, or \p mem is nullptr.
* \return RIF_ERROR_UNSUPPORTED_IMAGE_FORMAT - if image format defined by \p image_desc is currently unsupported.
*/
extern RIF_API_ENTRY rif_int rifContextCreateImageFromDirectX11Buffer(rif_context context, rif_image_desc const * image_desc, void* mem, rif_image * out_image);

#ifdef __cplusplus
}
#endif


#endif // __RADEONIMAGEFILTERS_DIRECTX12_H
