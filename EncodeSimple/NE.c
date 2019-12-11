#include <stdint.h>
#include <stdio.h>
#include "x264.h"
#include <stdlib.h>

int main( int argc, char **argv )
{
    int width = 640;
    int height= 360;
    x264_param_t param;
    x264_picture_t pic;
    x264_picture_t pic_out;
    x264_t *h;
    int i_frame = 0;
    int i_frame_size;
    x264_nal_t *nal;
    int i_nal;

    FILE* fp_src  = fopen("demo_640x360_yuv420p.yuv", "rb");
	FILE* fp_dst = fopen("out.h264", "wb");

    /* Get default params for preset/tuning */
    if( x264_param_default_preset( &param, "medium", NULL ) < 0 ){
        return -1;
    }
        
    /* Configure non-default params */
    param.i_bitdepth = 8;
    param.i_csp = X264_CSP_I420;
    param.i_width  = width;
    param.i_height = height;
    param.b_vfr_input = 0;
    param.b_repeat_headers = 1;
    param.b_annexb = 1;
    
    /* Apply profile restrictions. */
    if( x264_param_apply_profile( &param, "baseline" ) < 0 ){
        return -1;
    }

    if( x264_picture_alloc( &pic, param.i_csp, param.i_width, param.i_height ) < 0 ){
        return -1;
    }
        
  

    h = x264_encoder_open( &param );
    if( !h ){
        x264_picture_clean( &pic );
    }
       



    int luma_size = width * height;
    int chroma_size = luma_size / 4;
    /* Encode frames */
    for( ;; i_frame++ )
    {
        /* Read input frame */
        if( fread( pic.img.plane[0], 1, luma_size, fp_src ) != luma_size )
            break;
        if( fread( pic.img.plane[1], 1, chroma_size, fp_src ) != chroma_size )
            break;
        if( fread( pic.img.plane[2], 1, chroma_size, fp_src ) != chroma_size )
            break;

        pic.i_pts = i_frame;
        i_frame_size = x264_encoder_encode( h, &nal, &i_nal, &pic, &pic_out );
        if( i_frame_size < 0 ){
            x264_encoder_close( h );
        }
        else if( i_frame_size )
        {
            printf("\n---------------进入编码的输出----------------\n");
            if( !fwrite( nal->p_payload, i_frame_size, 1, fp_dst ) ){
                x264_encoder_close( h );
            }
        }
    }
    /* Flush delayed frames */
    while( x264_encoder_delayed_frames( h ) )
    {
        i_frame_size = x264_encoder_encode( h, &nal, &i_nal, NULL, &pic_out );
        if( i_frame_size < 0 ){
            x264_encoder_close( h );
        }
        else if( i_frame_size )
        {
            if( !fwrite( nal->p_payload, i_frame_size, 1, fp_dst ) ){
                x264_encoder_close( h );
            }
        }
    }

    x264_encoder_close( h );
    x264_picture_clean( &pic );

    fclose(fp_src);
    fclose(fp_dst);

    return 0;
}
