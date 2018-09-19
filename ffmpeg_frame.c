/*
   This file is part of ffmpeg-php

   Copyright (C) 2004-2008 Todd Kirby (ffmpeg.php AT gmail.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   In addition, as a special exception, the copyright holders of ffmpeg-php
   give you permission to combine ffmpeg-php with code included in the
   standard release of PHP under the PHP license (or modified versions of
   such code, with unchanged license). You may copy and distribute such a
   system following the terms of the GNU GPL for ffmpeg-php and the licenses
   of the other code concerned, provided that you include the source code of
   that other code when and as the GNU GPL requires distribution of source code.

   You must obey the GNU General Public License in all respects for all of the
   code used other than standard release of PHP. If you modify this file, you
   may extend this exception to your version of the file, but you are not
   obligated to do so. If you do not wish to do so, delete this exception
   statement from your version.

 */

#include "php.h"
#include "php_ini.h"
#include "php_globals.h"
#include "ext/standard/info.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_ffmpeg.h"

#include "ffmpeg_frame.h"
#include "ffmpeg_tools.h"

#include <libavutil/imgutils.h>


/* 
   include gd header from local include dir. This is a copy of gd.h that is 
   distributed with php-5.2.5. It is distributed along with ffmpeg-php to
   allow ffmpeg-php to be built without access to the php sources
 */
#if HAVE_LIBGD20
#include "gd.h"
#include "ext/gd/php_gd.h"

#if PHP_MAJOR_VERSION < 7

#define FFMPEG_PHP_FETCH_IMAGE_RESOURCE(gd_img, ret) { \
    ZEND_FETCH_RESOURCE(gd_img, gdImagePtr, ret, -1, "Image", phpi_get_le_gd()); \
}

#else

#define FFMPEG_PHP_FETCH_IMAGE_RESOURCE(gd_img, ret) { \
	if ((gd_img = (gdImagePtr)zend_fetch_resource(Z_RES_P(ret), "Image", phpi_get_le_gd())) == NULL) {\
		RETURN_FALSE;\
	}\
}
#endif



// Borrowed from gd.c
#define gdImageBoundsSafeMacro(im, x, y) (!((((y) < (im)->cy1) || ((y) > (im)->cy2)) || (((x) < (im)->cx1) || ((x) > (im)->cx2))))

#endif // HAVE_LIBGD20

int le_ffmpeg_frame; // not static since it is used in ffmpeg_output_movie

static zend_class_entry *ffmpeg_frame_class_entry_ptr;
zend_class_entry ffmpeg_frame_class_entry;
 
/* {{{ ffmpeg_frame methods[]
    Methods of the ffmpeg_frame class 
*/
zend_function_entry ffmpeg_frame_class_methods[] = {
    
    /* object can't be created from user space so no PHP constructor */
    //PHP_ME(ffmpeg_frame, __construct, NULL, 0)
  
#if HAVE_LIBGD20
    /* gd methods */
    PHP_MALIAS(ffmpeg_frame, togdimage,      toGDImage,     NULL, 0)
#endif // HAVE_LIBGD20

    /* methods */
    PHP_MALIAS(ffmpeg_frame, getwidth,                    getWidth,                   NULL, 0)
    PHP_MALIAS(ffmpeg_frame, getheight,                   getHeight,                  NULL, 0)
    PHP_MALIAS(ffmpeg_frame, iskeyframe,                  isKeyFrame,                 NULL, 0)
    PHP_MALIAS(ffmpeg_frame, getpresentationtimestamp,    getPresentationTimestamp,   NULL, 0)
    PHP_MALIAS(ffmpeg_frame, getpts,                      getPresentationTimestamp,   NULL, 0)
	{NULL, NULL, NULL}
};
/* }}} */


/* {{{ _php_alloc_ff_frame()
 */
static ff_frame_context* _php_alloc_ff_frame()
{
    ff_frame_context *ff_frame = NULL;

    ff_frame = emalloc(sizeof(ff_frame_context));
    
    if (!ff_frame) {
        zend_error(E_ERROR, "Error allocating ffmpeg_frame");
    }

    ff_frame->av_frame = NULL;
    ff_frame->width = 0;
    ff_frame->height = 0;
    ff_frame->pixel_format = 0;

    return ff_frame;
}
/* }}} */


/* {{{ proto object _php_create_ffmpeg_frame() 
   creates an ffmpeg_frame object, adds a ffmpeg_frame resource to the
   object, registers the resource and returns a direct pointer to the 
   resource.
 */
ff_frame_context* _php_create_ffmpeg_frame(INTERNAL_FUNCTION_PARAMETERS)
{
#if PHP_MAJOR_VERSION < 7
    int ret;
#else
    FFMPEG_RESOURCE *ret;
#endif

	ff_frame_context *ff_frame;
    
    ff_frame = _php_alloc_ff_frame();
    
    ret = FFMPEG_REGISTER_RESOURCE(NULL, ff_frame, le_ffmpeg_frame);
    object_init_ex(return_value, ffmpeg_frame_class_entry_ptr);
    add_property_resource(return_value, "ffmpeg_frame", ret);

    return ff_frame;
}
/* }}} */


/* {{{ _php_free_av_frame()
 */
static void _php_free_av_frame(AVFrame *av_frame)
{
    if (av_frame) {
        if (av_frame->data[0]) {
            av_free(av_frame->data[0]);
            av_frame->data[0] = NULL;
        }
        av_free(av_frame);
    }
}
/* }}} */


/* {{{ _php_free_ffmpeg_frame()
 */
static void _php_free_ffmpeg_frame(FFMPEG_RESOURCE *rsrc TSRMLS_DC)
{
    ff_frame_context *ff_frame = (ff_frame_context*)rsrc->ptr;    
    _php_free_av_frame(ff_frame->av_frame);
    efree(ff_frame);
}
/* }}} */


/* {{{ register_ffmpeg_frame_class()
 */
void register_ffmpeg_frame_class(int module_number)
{
    TSRMLS_FETCH();

    le_ffmpeg_frame = zend_register_list_destructors_ex(_php_free_ffmpeg_frame,
            NULL, "ffmpeg_frame", module_number);

    INIT_CLASS_ENTRY(ffmpeg_frame_class_entry, "ffmpeg_frame", 
            ffmpeg_frame_class_methods);

    /* register ffmpeg frame class */
    ffmpeg_frame_class_entry_ptr = 
        zend_register_internal_class(&ffmpeg_frame_class_entry TSRMLS_CC);
}
/* }}} */


/* {{{ _php_convert_frame()
*/
int _php_convert_frame(ff_frame_context *ff_frame_ctx, int dst_fmt) {
    AVFrame *src_frame;
    AVFrame *dst_frame;
    int result = 0;

    if (!ff_frame_ctx->av_frame) {
        return -1;
    }

    src_frame = ff_frame_ctx->av_frame;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
    dst_frame = av_frame_alloc();
#else
    dst_frame = avcodec_alloc_frame();
#endif


#if 0
    avpicture_alloc((AVPicture*)dst_frame, dst_fmt, ff_frame_ctx->width,
            ff_frame_ctx->height);
#else
    int ret = av_image_alloc(dst_frame->data,
            dst_frame->linesize, ff_frame_ctx->width, ff_frame_ctx->height,
            dst_fmt, 1);

        if (ret < 0) {
           memset(dst_frame->data, 0, sizeof(AVPicture));
        }


#endif

    result = ffmpeg_img_convert((AVPicture*)dst_frame, dst_fmt, 
                (AVPicture *)src_frame, 
                ff_frame_ctx->pixel_format, ff_frame_ctx->width, 
                ff_frame_ctx->height);

    if (result) {
        zend_error(E_ERROR, "Error converting frame");
        goto fail;
    }

    ff_frame_ctx->av_frame = dst_frame;
    ff_frame_ctx->pixel_format = dst_fmt;

fail:
    _php_free_av_frame(src_frame);
    return result;
}
/* }}} */

#if HAVE_LIBGD20

/* {{{ _php_get_gd_image()
 */
static long _php_get_gd_image(int w, int h)
{
#if PHP_MAJOR_VERSION < 7
    zval *function_name, *width, *height;
    zval **argv[2];
    zval *retval;
#else
    zval function_name, width, height;
    zval argv[2];
    zval retval;
#endif

    zend_function *func;
    char *function_cname = "imagecreatetruecolor";
    long ret;
    TSRMLS_FETCH();

#if PHP_MAJOR_VERSION < 7   
    if (zend_hash_find(EG(function_table), function_cname, 
                strlen(function_cname) + 1, (void **)&func) == FAILURE) {
        zend_error(E_ERROR, "Error can't find %s function", function_cname);
    }
#else
    if ((func = zend_hash_str_find_ptr(EG(function_table), function_cname,
                strlen(function_cname))) == NULL) {
        zend_error(E_ERROR, "Error can't find %s function", function_cname);
    }
#endif

#if PHP_MAJOR_VERSION < 7
    MAKE_STD_ZVAL(function_name);
    MAKE_STD_ZVAL(width);
    MAKE_STD_ZVAL(height);

    FFMPEG_ZVAL_STRING(function_name, function_cname, 0);
    ZVAL_LONG(width, w);
    ZVAL_LONG(height, h);

    argv[0] = &width;
    argv[1] = &height;

    if (call_user_function_ex(EG(function_table), NULL, function_name, 
                &retval, 2, argv, 0, NULL TSRMLS_CC) == FAILURE) {
        zend_error(E_ERROR, "Error calling %s function", function_cname);
    }

    FREE_ZVAL(function_name); 
    FREE_ZVAL(width); 
    FREE_ZVAL(height); 

    if (!retval || retval->type != IS_RESOURCE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Error creating GD Image");
    }

    ret = retval->value.lval;
    zend_list_addref(ret); 
    if (retval) {
        zval_ptr_dtor(&retval);
    }

#else
    FFMPEG_ZVAL_STRING(&function_name, function_cname, 0);
    ZVAL_LONG(&width, w);
    ZVAL_LONG(&height, h);

    argv[0] = width;
    argv[1] = height;

    if (call_user_function_ex(EG(function_table), NULL, &function_name, 
                &retval, 2, argv, 0, NULL TSRMLS_CC) == FAILURE) {
        zend_error(E_ERROR, "Error calling %s function", function_cname);
    }

    if (Z_TYPE(retval) != IS_RESOURCE) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Error creating GD Image");
    }

    ret = retval.value.lval;
    Z_ADDREF_P(&retval);
    zval_ptr_dtor(&retval);
#endif

    return ret;
}
/* }}} */


/* {{{ _php_avframe_to_gd_image()
 */
static int _php_avframe_to_gd_image(AVFrame *frame, gdImage *dest, int width, 
        int height)
{
    int x, y;
    int *src = (int*)frame->data[0];

	if(width > dest->sx || height > dest->sy){
		return -1;
	}

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /* copy pixel to gdimage buffer zeroing the alpha channel */
			dest->tpixels[y][x] = src[x] & 0x00ffffff;
        }
        src += width;
    }
    return 0;
}
/* }}} */


/* {{{ _php_gd_image_to_avframe()
 */
static int _php_gd_image_to_avframe(gdImage *src, AVFrame *frame, int width, 
        int height) 
{
    int x, y;
    int *dest = (int*)frame->data[0];

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (gdImageBoundsSafeMacro(src, x, y)) {
                dest[x] = src->tpixels[y][x];
            } else {
                return -1;
            }
        }
        dest += width;
    }
    return 0;
}
/* }}} */


/* {{{ proto resource toGDImage()
 */
FFMPEG_PHP_METHOD(ffmpeg_frame, toGDImage)
{
    ff_frame_context *ff_frame;
    gdImage *gd_img;

    GET_FRAME_RESOURCE(getThis(), ff_frame);

    _php_convert_frame(ff_frame, AV_PIX_FMT_RGB32);

    return_value->value.lval = _php_get_gd_image(ff_frame->width, 
            ff_frame->height);

#if PHP_MAJOR_VERSION < 7
    return_value->type = IS_RESOURCE;
    FFMPEG_PHP_FETCH_IMAGE_RESOURCE(gd_img, &return_value);
#else
    FFMPEG_PHP_FETCH_IMAGE_RESOURCE(gd_img, return_value);
#endif


    if (_php_avframe_to_gd_image(ff_frame->av_frame, gd_img,
            ff_frame->width, ff_frame->height)) {
        zend_error(E_ERROR, "failed to convert frame to gd image");
    }

#if PHP_MAJOR_VERSION >= 7
    RETURN_RES(Z_PTR_P(return_value));
#endif

}
/* }}} */


/* {{{ proto object _php_read_frame_from_file(mixed)
 */
/*_php_read_frame_from_file(ff_frame_context *ff_frame, char* filename)
{
    AVFrame *frame = NULL;
    AVFormatContext *ic;
    AVFormatParameters *ap;
    int err;

    // open the input file with generic libav function
    err = av_open_input_file(&ic, filename, NULL, 0, ap);
    if (err < 0) {
        zend_error(E_ERROR, "Can't open image file %d, %d", err, 
        AVERROR_NOFMT);
    }

    
}*/
/* }}} */


/* {{{ proto object ffmpeg_frame(mixed)
 */
FFMPEG_PHP_METHOD(ffmpeg_frame, ffmpeg_frame)
{
#if PHP_MAJOR_VERSION < 7
    zval **argv[1];
    long ret;
#else
    zval argv[1];
    zend_resource *ret;
#endif

    AVFrame *frame;
    gdImage *gd_img;
    ff_frame_context *ff_frame;
    int width, height;

    if (ZEND_NUM_ARGS() != 1) {
        WRONG_PARAM_COUNT;
    }

    /* retrieve argument */
    if (zend_get_parameters_array_ex(ZEND_NUM_ARGS(), argv) != SUCCESS) {
        php_error_docref(NULL TSRMLS_CC, E_ERROR,
                "Error parsing arguments");
    }

    ff_frame = _php_alloc_ff_frame();
    
    ret = FFMPEG_REGISTER_RESOURCE(NULL, ff_frame, le_ffmpeg_frame);
    
#if PHP_MAJOR_VERSION < 7
    object_init_ex(getThis(), ffmpeg_frame_class_entry_ptr);
#endif

    add_property_resource(getThis(), "ffmpeg_frame", ret);

#if PHP_MAJOR_VERSION < 7    
    switch (Z_TYPE_PP(argv[0])) {
#else
    switch (Z_TYPE(argv[0])) {
#endif
        case IS_STRING:
#if PHP_MAJOR_VERSION < 7    
            convert_to_string_ex(argv[0]);
#else
            convert_to_string_ex(&argv[0]);
#endif
            zend_error(E_ERROR, 
                  "Creating an ffmpeg_frame from a file is not implemented\n");
            //_php_read_frame_from_file(ff_frame, Z_STRVAL_PP(argv[0]));
            break;
        case IS_RESOURCE:
#if PHP_MAJOR_VERSION < 7    
            FFMPEG_PHP_FETCH_IMAGE_RESOURCE(gd_img, argv[0]);
#else
            FFMPEG_PHP_FETCH_IMAGE_RESOURCE(gd_img, &argv[0]);
#endif
            if (!gd_img->trueColor) {
                php_error_docref(NULL TSRMLS_CC, E_ERROR,
                        "First parameter must be a truecolor gd image.");
            }

            width = gdImageSX(gd_img);
            height = gdImageSY(gd_img);

            /* create a an av_frame and allocate space for it */
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
            frame = av_frame_alloc();
#else
            frame = avcodec_alloc_frame();
#endif

#if 0
            avpicture_alloc((AVPicture*)frame, AV_PIX_FMT_RGB32, width, height);
#else
            if (av_image_alloc(frame->data, frame->linesize, width,
                             height, AV_PIX_FMT_RGB32, 1) < 0) {

                memset(frame->data, 0, sizeof(AVPicture));

            }

#endif

            /* copy the gd image to the av_frame */
            _php_gd_image_to_avframe(gd_img, frame, width, height);
            
            /* set the ffmepg_frame to point to this av_frame */
            ff_frame->av_frame = frame;
            
            /* set the ffpmeg_frame's properties */
            ff_frame->width = width;
            ff_frame->height = height;
            ff_frame->pixel_format = AV_PIX_FMT_RGB32;
            break;
        default:
            zend_error(E_ERROR, "Invalid argument\n");
    }
}
/* }}} */

#endif /* HAVE_LIBGD20 */


/* {{{ proto int getPresentationTimestamp()
 */
FFMPEG_PHP_METHOD(ffmpeg_frame, getPresentationTimestamp)
{
    ff_frame_context *ff_frame;

    GET_FRAME_RESOURCE(getThis(), ff_frame);
    
    RETURN_DOUBLE((double)ff_frame->pts / AV_TIME_BASE);
}
/* }}} */


/* {{{ proto int isKeyFrame()
 */
FFMPEG_PHP_METHOD(ffmpeg_frame, isKeyFrame)
{
    ff_frame_context *ff_frame;

    GET_FRAME_RESOURCE(getThis(), ff_frame);
    
    RETURN_LONG(ff_frame->keyframe);
}
/* }}} */


/* {{{ proto int getWidth()
 */
FFMPEG_PHP_METHOD(ffmpeg_frame, getWidth)
{
    ff_frame_context *ff_frame;

    GET_FRAME_RESOURCE(getThis(), ff_frame);
    
    RETURN_LONG(ff_frame->width);
}
/* }}} */


/* {{{ proto int getHeight()
 */
FFMPEG_PHP_METHOD(ffmpeg_frame, getHeight)
{
    ff_frame_context *ff_frame;

    GET_FRAME_RESOURCE(getThis(), ff_frame);
    
    RETURN_LONG(ff_frame->height);
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4
 * vim<600: noet sw=4 ts=4
 */
