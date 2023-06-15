// http://ffmpeg.org/doxygen/trunk/decode_video_8c-example.html
// http://ffmpeg.org/doxygen/trunk/demux_decode_8c-example.html
// http://ffmpeg.org/doxygen/trunk/hw_decode_8c-example.html
#include <sstream>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
}

#include "decoder_thread.h"

#include "mylog.h"

#define __STDC_CONSTANT_MACROS

using std::ostringstream;

static uint8_t *video_dst_data[4] = {NULL};
static int video_dst_linesize[4];
static int video_dst_bufsize;

static enum AVPixelFormat hw_pix_fmt;

//static AVFrame *frame = NULL;
static int video_frame_count = 0;

DecoderThread::DecoderThread()
{
  if (!avpacket_)
    avpacket_ = av_packet_alloc();
  if (!avpacket_) {
    LOG << "Could not allocate packet ENOMEM: " << ENOMEM;
    //    fprintf(stderr, "Could not allocate packet\n");
    //    ret = AVERROR(ENOMEM);
    //    goto end;
  }
  avframes_mgr.Reserve(30 * 5);
}

void DecoderThread::Init(const char* file)
{
  video_file_ = file;
  int ret = 0;

  int argc = 4;
  char *argv[] = {};
  if (argc != 4) {
    fprintf(stderr,
            "usage: %s  input_file video_output_file audio_output_file\n"
            "API example program to show how to read frames from an input file.\n"
            "This program reads frames from a file, decodes them, and writes decoded\n"
            "video frames to a rawvideo file named video_output_file, and decoded\n"
            "audio frames to a rawaudio file named audio_output_file.\n",
            argv[0]);
    exit(1);
  }

  /* open input file, and allocate format context */
  if (avformat_open_input(&fmt_ctx_, video_file_, NULL, NULL) < 0) {
    fprintf(stderr, "Could not open source file %s\n", video_file_);
    exit(1);
  }

  /* retrieve stream information */
  if (avformat_find_stream_info(fmt_ctx_, NULL) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    exit(1);
  }

  LOG << "open open_codec_context video";
  if (open_codec_context(&video_stream_idx_, &video_codec_ctx_, AVMEDIA_TYPE_VIDEO) >= 0) {
    video_stream_ = fmt_ctx_->streams[video_stream_idx_];

    /* allocate image where the decoded image will be put */
    width_ = video_codec_ctx_->width;
    height_ = video_codec_ctx_->height;
    pix_fmt_ = video_codec_ctx_->pix_fmt;
    ret = av_image_alloc(video_dst_data, video_dst_linesize, width_, height_, pix_fmt_, 1);
    if (ret < 0) {
      fprintf(stderr, "Could not allocate raw video buffer\n");
      goto end;
    }
    video_dst_bufsize = ret;
  }

  if (!video_stream_) {
    fprintf(stderr, "Could not find video stream in the input, aborting\n");
    ret = 1;
    goto end;
  }

  /* flush the decoders */
  //  if (video_codec_ctx_)
  //    decode_packet(video_codec_ctx_, NULL);

  InitGopRecorder();
  printf("Demuxing succeeded.\n");

//    if (video_stream_) {
//        printf("Play the output video file with the command:\n"
//               "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
//               av_get_pix_fmt_name(pix_fmt_), width_, height_,
//               video_dst_filename);
//    }

end:
  //  avcodec_free_context(&video_codec_ctx_);
  //  avformat_close_input(&fmt_ctx_);
  //  //    if (video_dst_file)
  //  //        fclose(video_dst_file);
  //  av_packet_free(&pkt);
  //  av_frame_free(&frame);
  //  av_free(video_dst_data[0]);

  LOG << "ret:" << ret;
}

void DecoderThread::InitGopRecorder()
{
  int64_t frame_id = 0;
  while (av_read_frame(fmt_ctx_, avpacket_) >= 0) {
    if (avpacket_->stream_index == video_stream_idx_) {
      gop_recorder_.UpdatePacket(frame_id++, avpacket_);
    }
    av_packet_unref(avpacket_);
  }

  // reset fptr to begining
  char av_err_str[AV_ERROR_MAX_STRING_SIZE];
  const FrameKey *fk = gop_recorder_.GetGopFirstKeyFrame(0);
  if (fk != nullptr) {
    int ret = avformat_seek_file(fmt_ctx_,
                                 video_stream_idx_,
                                 INT64_MIN,
                                 fk->pts,
                                 fk->pts + 4000,
                                 0);
    if (ret < 0) {
      av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
      LOG << "avformat_seek_file error: " << av_err_str;
    } else {
      LOG << "avformat_seek_file ok";
    }
  }
  LOG << "gop_recorder_: " << gop_recorder_.String().c_str();
}


int DecoderThread::output_video_frame(AVFrame *frame)
{
  if (frame->width != width_ || frame->height != height_ || frame->format != pix_fmt_) {
    /* To handle this change, one could call av_image_alloc again and
         * decode the following frames into another rawvideo file. */
    LOG << "Error: Width, height and pixel format have to be "
        << "constant in a rawvideo file, but the width, height or "
        << "pixel format of the input video changed: " << width_ << '*' << height_ << ' '
        << av_get_pix_fmt_name(pix_fmt_) << " -> " << frame->width << '*' << frame->height << ' '
        << av_get_pix_fmt_name((AVPixelFormat) frame->format);
    //        LOG << "old: width=" << width_ << ", height=" << height_ << av_get_pix_fmt_name((AVPixelFormat)frame->format);
    //            << ", format=" << av_get_pix_fmt_name(pix_fmt_);
    //        LOG << "new: width=" << frame->width << ", height=" << frame->height
    //            << ", format=" << av_get_pix_fmt_name((AVPixelFormat)frame->format);
    //        return -1;
  }

  LOG << "video frame n:" << video_frame_count++
      << " " << frame->width << '*' << frame->height
      << " " << av_get_pix_fmt_name((AVPixelFormat) frame->format)
      << " dts:" << frame->pkt_dts;
  //    printf("video_frame n:%d\n", video_frame_count++);
  return 0;

  /* copy decoded frame to destination buffer:
     * this is required since rawvideo expects non aligned data */
  av_image_copy(video_dst_data,
                video_dst_linesize,
                (const uint8_t **) (frame->data),
                frame->linesize,
                pix_fmt_,
                width_,
                height_);

  /* write to rawvideo file */
  //    fwrite(video_dst_data[0], 1, video_dst_bufsize, video_dst_file);
  return 0;
}

int DecoderThread::decode_packet(AVCodecContext *dec, const AVPacket *pkt)
{
  int ret = 0;
  char av_err_str[AV_ERROR_MAX_STRING_SIZE];

  AVFrame *sw_frame = NULL;

  // submit the packet to the decoder
  ret = avcodec_send_packet(dec, pkt);
  if (ret < 0) {
    av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
    fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err_str);
    return ret;
  }

  // get all the available frames from the decoder
  while (ret >= 0) {
    AVFrame *pframe = av_frame_alloc();
    ret = avcodec_receive_frame(dec, pframe);
    if (ret < 0) {
      // those two return values are special and mean there is no output
      // frame available, but there were no errors during decoding
      if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
        return 0;

      av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
      fprintf(stderr, "Error during decoding (%s)\n", av_err_str);
      return ret;
    }

    // write the frame data to output file
    if (dec->codec->type == AVMEDIA_TYPE_VIDEO) {
//      ret = output_video_frame(pframe);
      if (true || pframe->width == 1920) {
//        emit signal_on_avframe(pframe);
//        msleep(3600 * 1000);
      }

      if (false && hw_decode_ && pframe->format == hw_pix_fmt_) {
        // retrieve data from GPU to CPU
        ret = av_hwframe_map(sw_frame, pframe, 0);
        if (ret < 0) {
          av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
          LOG << "av_hwframe_map err:" << av_err_str;
//        if ((ret = av_hwframe_transfer_data(sw_frame, pframe, 0)) < 0) {
//          fprintf(stderr, "Error transferring the data to system memory\n");
          goto fail;
        }
      }
      av_frame_free(&pframe);
    }
    //        av_frame_unref(frame);
  }
fail:
//  av_frame_unref(pframe);
  return 0;
}

int DecoderThread::hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
{
  int err = 0;
  err = av_hwdevice_ctx_create(&hw_device_ctx_, type, NULL, NULL, 0);
  if (err < 0) {
    LOG << "Failed to create specified HW device.";
    return err;
  }
  ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx_);
  return err;
}

static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *pix_fmts)
{
  const enum AVPixelFormat *p = NULL;

  LOG << "hw_pix_fmt: " << av_get_pix_fmt_name((AVPixelFormat)hw_pix_fmt);

  for (p = pix_fmts; *p != -1; p++) {
    LOG << av_get_pix_fmt_name((AVPixelFormat)*p);
    if (*p == hw_pix_fmt) {
      break;
    }
  }

  if (p != NULL)
    return *p;

  LOG << "Failed to get HW surface format";
  return AV_PIX_FMT_NONE;
}

int DecoderThread::open_codec_context(int *stream_idx,
                                      AVCodecContext **codec_ctx,
                                      enum AVMediaType type)
{
  int ret, stream_index;
  AVStream *avstream;
  const AVCodec *avcodec = NULL;

  AVHWDeviceType device_type;
  const char* hw_device_type_name = "dxva2";
//  hw_device_type_name = "d3d11va";
  if (hw_decode_) {
    device_type = av_hwdevice_find_type_by_name(hw_device_type_name);
    if (AV_HWDEVICE_TYPE_NONE == device_type) {
      LOG << "Device type " << hw_device_type_name << " is not supported.";
      ostringstream os;
      while((device_type = av_hwdevice_iterate_types(device_type)) != AV_HWDEVICE_TYPE_NONE)
        os << " " << av_hwdevice_get_type_name(device_type);
      LOG << "Available device types:" << os.str().c_str();
      return -1;
    }
  }

  ret = av_find_best_stream(fmt_ctx_, type, -1, -1, NULL, 0);
  if (ret < 0) {
    LOG << "could not find " << av_get_media_type_string(type)
        << " stream in input file " << video_file_;
    return ret;
  } else {
    stream_index = ret;
    avstream = fmt_ctx_->streams[stream_index];

    /* find decoder for the stream */
    avcodec = avcodec_find_decoder(avstream->codecpar->codec_id);
    //        dec = avcodec_find_decoder_by_name("dxva2");
    if (!avcodec) {
      LOG << "failed to find " << av_get_media_type_string(type) << " codec";
      return AVERROR(EINVAL);
    }

    if (hw_decode_) {
      int index = 0;
      for (index = 0;; ++index) {
        const AVCodecHWConfig *config = avcodec_get_hw_config(avcodec, index);
        if (NULL == config) {
          LOG << "avcodec_get_hw_config failed, index:" << index;
        }
        if (!config) {
          LOG << "Decoder [" << avcodec->name << "]"
              << " does not support device type:" << av_hwdevice_get_type_name(device_type);
        }
        if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX
            && config->device_type == device_type) {
          hw_pix_fmt = hw_pix_fmt_ = config->pix_fmt;
          LOG << "found:" << av_get_pix_fmt_name((AVPixelFormat)hw_pix_fmt) << ", break";
          break;
        }
      }
//      (*codec_ctx)->get_format = std::bind(&DecoderThread::get_hw_format, this,
//                                           std::placeholders::_1,
//                                           std::placeholders::_2);
    }

    /* Allocate a codec context for the decoder */
    *codec_ctx = avcodec_alloc_context3(avcodec);
    if (!*codec_ctx) {
      fprintf(stderr, "Failed to allocate the %s codec context\n", av_get_media_type_string(type));
      return AVERROR(ENOMEM);
    }

    /* Copy codec parameters from input stream to output codec context */
    if ((ret = avcodec_parameters_to_context(*codec_ctx, avstream->codecpar)) < 0) {
      LOG << "failed to copy " << av_get_media_type_string(type)
          << " codec parameters to decoder context";
      return ret;
    }

    if (hw_decode_) {
      (*codec_ctx)->get_format = get_hw_format;
      if (hw_decoder_init(*codec_ctx, device_type) < 0) {
        LOG << "hw_decoder_init < 0";
        return -1;
      }
    }

    /* Init the decoders */
    if ((ret = avcodec_open2(*codec_ctx, avcodec, NULL)) < 0) {
      LOG << "failed to open " << av_get_media_type_string(type) << " codec";
      return ret;
    }
    *stream_idx = stream_index;
  }

  return 0;
}

int DecoderThread::get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt)
{
  int i;
  struct sample_fmt_entry
  {
    enum AVSampleFormat sample_fmt;
    const char *fmt_be, *fmt_le;
  } sample_fmt_entries[] = {
      {AV_SAMPLE_FMT_U8, "u8", "u8"},
      {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
      {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
      {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
      {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
  };
  *fmt = NULL;

  for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
    struct sample_fmt_entry *entry = &sample_fmt_entries[i];
    if (sample_fmt == entry->sample_fmt) {
      *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
      return 0;
    }
  }
  LOG << "sample format " << av_get_sample_fmt_name(sample_fmt)
      << " is not supported as output format";
  return -1;
}

uint32_t DecoderThread::emit_frames(const vector<int64_t>& frame_ids)
{
  vector<pair<int64_t, AVFrame *> frames = avframes_mgr.GetDecoded(frame_ids);
  for (auto& p : frames) {
    int64_t frame_id = p.first;
    AVFrame* frame = p.second;
    emit signal_frame_decoded(frame_id, frame, video_stream_->time_base);
  }
}

bool DecoderThread::emit_frame(int64_t frame_id)
{
  AVFrame* frame = avframes_mgr.GetDecoded(frame_id);
  if (NULL == frame) {
    LOG << "ERR: GetDecoded failed, frame_id: " << frame_id;
    return false;
  }
  emit signal_frame_decoded(frame_id, frame, video_stream_->time_base);
  return true;
}

uint32_t DecoderThread::emit_frames(uint32_t n)
{
  uint32_t frame_id;
  AVFrame *frame = NULL;
  uint32_t sent = 0;
  for (; sent < n; ++sent) {
    frame = avframes_mgr.GetDecoded(frame_id);
    if (!frame) {
      LOG << "avframes_mgr.GetDecoded return NULL";
      break;
    }
    emit signal_frame_decoded(frame_id, frame, video_stream_->time_base);
  }
  return sent;
}

static int sent_packets = 0;

uint32_t DecoderThread::decode_and_cache(uint32_t max_cache)
{
  if (hw_decode_)
    return hw_decode_and_cache(max_cache);

  int ret = 0;
  AVFrame *frame = NULL;
//  LOG << "max_cache: " << max_cache;
  uint32_t cur_decoded_frames = avframes_mgr.DecodedFrames();
  if (avframes_mgr.DecodedFrames() >= max_cache) {
    goto end;
  }
  char av_err_str[AV_ERROR_MAX_STRING_SIZE];

  frame = avframes_mgr.GetFree();
  if (NULL == frame) {
    LOG << "avframes_mgr.GetFree() NULL";
    goto end;
  }
  // read frames from the file
  while (av_read_frame(fmt_ctx_, avpacket_) >= 0) {
    if (avpacket_->stream_index != video_stream_idx_) {
      av_packet_unref(avpacket_);
      continue;
    }
//    LOG << "sent_packets:" << ++sent_packets;
    ret = avcodec_send_packet(video_codec_ctx_, avpacket_);
    if (ret < 0) {
      av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
      LOG << "avcodec_send_packet error: " << av_err_str;
      av_packet_unref(avpacket_);
      continue;
    }

    // get all the available frames from the decoder
    while (ret >= 0 && frame != NULL && avframes_mgr.DecodedFrames() < max_cache) {
      ret = avcodec_receive_frame(video_codec_ctx_, frame);
//      LOG << "sent_packets:" << ++sent_packets
//          << " avcodec_receive_frame ret:" << ret;
      if (ret < 0) {
        // those two return values are special and mean there is no output
        // frame available, but there were no errors during decoding
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN) )
          break;

        av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
        LOG << "avcodec_receive_frame error: " << av_err_str;
        goto end;
      }
      LOG << "PutDecoded(frame): " << ++sent_packets
          << " format: " << av_get_pix_fmt_name((AVPixelFormat)frame->format)
          << " pts; " << frame->pts;
      video_codec_ctx_->frame_number
      avframes_mgr.PutDecoded(frame);
      frame = avframes_mgr.GetFree();
    }
    av_packet_unref(avpacket_);
    if (avframes_mgr.DecodedFrames() >= max_cache)
      break;
  } // end  while (av_read_frame
end:
  if (frame)
    avframes_mgr.PutFree(frame);
  if (cur_decoded_frames < avframes_mgr.DecodedFrames())
    LOG << "decoded frames: "
        << cur_decoded_frames << " -> " << avframes_mgr.DecodedFrames();
  return avframes_mgr.DecodedFrames();
}

int DecoderThread::hw_decode_and_cache(uint32_t max_cache)
{
  int ret = 0;
  AVFrame *frame = NULL, *sw_frame = NULL;
  //  LOG << "max_cache: " << max_cache;
  uint32_t cur_decoded_frames = avframes_mgr.DecodedFrames();
  if (avframes_mgr.DecodedFrames() >= max_cache) {
    goto end;
  }
  char av_err_str[AV_ERROR_MAX_STRING_SIZE];

  frame = avframes_mgr.GetFree();
  sw_frame = avframes_mgr.GetFree();
  if (NULL == frame || NULL == sw_frame)
    goto end;

  // read frames from the file
  while (av_read_frame(fmt_ctx_, avpacket_) >= 0) {
    if (avpacket_->stream_index != video_stream_idx_) {
      av_packet_unref(avpacket_);
      continue;
    }
    //    LOG << "sent_packets:" << ++sent_packets;
    ret = avcodec_send_packet(video_codec_ctx_, avpacket_);
    if (ret < 0) {
      av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
      LOG << "avcodec_send_packet error: " << av_err_str;
      av_packet_unref(avpacket_);
      continue;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
      ret = avcodec_receive_frame(video_codec_ctx_, frame);
      // those two return values are special and mean there is no output
      // frame available, but there were no errors during decoding
      if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN) )
        break;
      else if (ret < 0) {
        av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
        LOG << "avcodec_receive_frame error: " << av_err_str;
        goto end;
      }

      if (frame->format == hw_pix_fmt_) {
        // retrieve data from GPU to CPU
        //        ret = av_hwframe_map(sw_frame, frame, 0);
        if ((ret = av_hwframe_transfer_data(sw_frame, frame, 0)) < 0) {
          av_make_error_string(av_err_str, AV_ERROR_MAX_STRING_SIZE, ret);
          LOG << "av_hwframe_transfer_data err:" << av_err_str;
          //          fprintf(stderr, "Error transferring the data to system memory\n");
          continue;
        }
//        else {
//          LOG << "av_hwframe_transfer_data success: " << ret;
//        }
        //        LOG << "sent_packets:" << ++sent_packets << " PutDecoded(sw_frame)";
        sw_frame->pts = frame->pts;
        LOG << "PutDecoded(sw_frame): " << ++sent_packets
            << " format: " << av_get_pix_fmt_name((AVPixelFormat)sw_frame->format)
            << " pts; " << sw_frame->pts;
        avframes_mgr.PutDecoded(sw_frame);
        if ((sw_frame = avframes_mgr.GetFree()) == NULL) {
          LOG << "sw_frame = avframes_mgr.GetFree() NULL";
          break;
        }
      } else {
        LOG << "PutDecoded(frame): " << ++sent_packets
            << " format: " << av_get_pix_fmt_name((AVPixelFormat)frame->format)
            << " pts: " << frame->pts;
        avframes_mgr.PutDecoded(frame);
        if ((frame = avframes_mgr.GetFree()) == NULL) {
          LOG << "frame = avframes_mgr.GetFree() NULL";
          break;
        }
      }
      if (avframes_mgr.DecodedFrames() >= max_cache)
        break;
    }
    av_packet_unref(avpacket_);
    if (avframes_mgr.DecodedFrames() >= max_cache)
      break;
    if (ret < 0 && ret != AVERROR_EOF && ret != AVERROR(EAGAIN) )
      break;
  } // end  while (av_read_frame
end:
  if (frame)
    avframes_mgr.PutFree(frame);
  if (sw_frame)
    avframes_mgr.PutFree(sw_frame);
  if (cur_decoded_frames < avframes_mgr.DecodedFrames())
    LOG << "decoded frames: "
        << cur_decoded_frames << " -> " << avframes_mgr.DecodedFrames();
  return avframes_mgr.DecodedFrames();
}

void DecoderThread::on_require_frames(uint32_t n)
{
//  return;
  uint32_t sent = emit_frames(n);
  if (sent == n)
    return;

  uint32_t need_decode_frames = n - sent;
  avframes_mgr.Reserve(need_decode_frames);

  decode_and_cache(need_decode_frames);

  emit_frames(need_decode_frames);
}

void DecoderThread::on_frame_consumed(uint32_t frame_id, AVFrame* avframe)
{
//  LOG << avframe;
  av_frame_unref(avframe);
  avframes_mgr.PutFree(avframe);
}

void DecoderThread::on_timer()
{
  LOG << avframes_mgr.Stat().c_str();
  decode_and_cache(max_cache_decoded_frames_);
}

void DecoderThread::on_signal_exit()
{
  avcodec_free_context(&video_codec_ctx_);
  avformat_close_input(&fmt_ctx_);
  //    if (video_dst_file)
  //        fclose(video_dst_file);
  av_packet_free(&avpacket_);
//  av_frame_free(&frame);
  av_free(video_dst_data[0]);
}
