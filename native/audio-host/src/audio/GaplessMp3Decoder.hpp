#pragma once
#include "miniaudio.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#undef DR_MP3_IMPLEMENTATION
#include <new>

namespace easyknack {
// Import-only backend: upstream dr_mp3 removes tagged encoder delay/padding;
// miniaudio retains responsibility for format conversion and resampling.
struct GaplessMp3Decoder {
    ma_data_source_base base{};
    drmp3 decoder{};
    static ma_result read(ma_data_source* source,void* output,ma_uint64 count,ma_uint64* readCount){
        auto* self=static_cast<GaplessMp3Decoder*>(source);
        auto frames=drmp3_read_pcm_frames_f32(&self->decoder,count,static_cast<float*>(output));
        if(readCount)*readCount=frames;
        return frames?MA_SUCCESS:MA_AT_END;
    }
    static ma_result seek(ma_data_source* source,ma_uint64 frame){return drmp3_seek_to_pcm_frame(&static_cast<GaplessMp3Decoder*>(source)->decoder,frame)?MA_SUCCESS:MA_BAD_SEEK;}
    static ma_result format(ma_data_source* source,ma_format* format,ma_uint32* channels,ma_uint32* rate,ma_channel* map,size_t capacity){
        auto& decoder=static_cast<GaplessMp3Decoder*>(source)->decoder;
        if(format)*format=ma_format_f32;if(channels)*channels=decoder.channels;if(rate)*rate=decoder.sampleRate;
        if(map)ma_channel_map_init_standard(ma_standard_channel_map_default,map,capacity,decoder.channels);
        return MA_SUCCESS;
    }
    static ma_result cursor(ma_data_source* source,ma_uint64* position){
        auto& decoder=static_cast<GaplessMp3Decoder*>(source)->decoder;
        *position=decoder.currentPCMFrame>decoder.delayInPCMFrames?decoder.currentPCMFrame-decoder.delayInPCMFrames:0;
        return MA_SUCCESS;
    }
    static ma_result length(ma_data_source* source,ma_uint64* frames){*frames=drmp3_get_pcm_frame_count(&static_cast<GaplessMp3Decoder*>(source)->decoder);return MA_SUCCESS;}
    inline static const ma_data_source_vtable sourceTable{read,seek,format,cursor,length,nullptr,0};
    static ma_result openFile(void*,const char* path,const ma_decoding_backend_config*,const ma_allocation_callbacks*,ma_data_source** output){
        auto* self=new(std::nothrow) GaplessMp3Decoder;
        if(!self)return MA_OUT_OF_MEMORY;
        auto config=ma_data_source_config_init();config.vtable=&sourceTable;
        auto result=ma_data_source_init(&config,&self->base);
        if(result!=MA_SUCCESS){delete self;return result;}
        if(!drmp3_init_file(&self->decoder,path,nullptr)){ma_data_source_uninit(&self->base);delete self;return MA_INVALID_FILE;}
        *output=self;return MA_SUCCESS;
    }
    static ma_result openWide(void*,const wchar_t* path,const ma_decoding_backend_config*,const ma_allocation_callbacks*,ma_data_source** output){
        auto* self=new(std::nothrow) GaplessMp3Decoder;
        if(!self)return MA_OUT_OF_MEMORY;
        auto config=ma_data_source_config_init();config.vtable=&sourceTable;
        auto result=ma_data_source_init(&config,&self->base);
        if(result!=MA_SUCCESS){delete self;return result;}
        if(!drmp3_init_file_w(&self->decoder,path,nullptr)){ma_data_source_uninit(&self->base);delete self;return MA_INVALID_FILE;}
        *output=self;return MA_SUCCESS;
    }
    static void close(void*,ma_data_source* source,const ma_allocation_callbacks*){
        auto* self=static_cast<GaplessMp3Decoder*>(source);drmp3_uninit(&self->decoder);ma_data_source_uninit(&self->base);delete self;
    }
    inline static ma_decoding_backend_vtable backend{nullptr,openFile,openWide,nullptr,close};
};
}
