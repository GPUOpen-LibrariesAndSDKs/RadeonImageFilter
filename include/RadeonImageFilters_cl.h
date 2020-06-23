/*****************************************************************************\
*
*  Module Name    RadeonImageFilters_cl.h
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

#ifndef __RADEONIMAGEFILTERS_CL_H
#define __RADEONIMAGEFILTERS_CL_H

#include "RadeonImageFilters.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern RIF_API_ENTRY rif_int rifCreateContextFromOpenClContext(rif_uint64 api_version, void* context, void* device, void* queue, rif_char const * cache_path, rif_context * out_context);

extern RIF_API_ENTRY rif_int rifContextCreateImageFromOpenClMemory(rif_context context, rif_image_desc const * image_desc, void* mem, rif_image * out_image);

#ifdef __cplusplus
}
#endif


#endif // __RADEONIMAGEFILTERS_CL_H
