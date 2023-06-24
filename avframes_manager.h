
#pragma once

#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <thread>

//#include <lz4.h>
//#include <lz4frame.h>

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::shared_ptr;

class AVFrame;

class CompressedBuffer {
public:
  bool Empty() { return len == 0;}
  size_t Size() { return len; }
  shared_ptr<uint8_t> buf;
  size_t len;
};

class MayCompressedFrame {
public:
  void CompressWithLz4() {
    if (tmp_comressed_buffer.Empty()) {
      // alloc
    } else {

    }
//    if ( ! tmp_comressed_buffer.Empty() )
  }

private:
  thread_local static CompressedBuffer tmp_comressed_buffer;

  AVFrame* avframe_;
  shared_ptr<uint8_t> compressed_frame_;
};

class AVFramesManager
{
public:
  AVFramesManager();

  void SetMemLimit(int64_t max_bytes) { max_alloc_bytes_ = max_bytes; }

  void Reserve(uint32_t n);

  size_t DecodedFrames() { return decoded_frames_.size(); }

  AVFrame* GetDecoded(int64_t frame_id);
  vector<pair<int64_t, AVFrame*> > GetDecoded(const vector<int64_t>& frame_ids);
  void PutDecoded(int64_t frame_id, AVFrame* frame);

  AVFrame* GetFree();
  void PutFree(int64_t frame_id, AVFrame *frame);

  string Stat();

private:

  int64_t calFrameBytes(AVFrame *frame);

  map<int64_t, AVFrame*> decoded_frames_; // frameid -> AVFrame*
//  map<uint32_t, AVFrame*>
  vector<AVFrame*> free_frames_;

  map<int64_t, AVFrame*> in_use_decoded_frames_;

  int64_t max_alloc_bytes_ = 0;
  int64_t alloc_bytes_ = 0;

  uint32_t max_alloc_frames_ = 30 * 20;
  uint32_t cur_alloc_frames_ = 0;

  uint32_t frame_id_ = 0;
};

