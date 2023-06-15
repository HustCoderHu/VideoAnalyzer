#include <stddef.h>
#include <sstream>
#include "avframes_manager.h"
#include "mylog.h"

extern "C" {
#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
}

using std::ostringstream;

AVFramesManager::AVFramesManager() {}

void AVFramesManager::Reserve(uint32_t n)
{
  if (n <= free_frames_.size())
    return;

  if (n > max_alloc_frames_ - cur_alloc_frames_) {
    n = max_alloc_frames_ - cur_alloc_frames_;
  }
  AVFrame* frame = NULL;
  for (uint32_t i = 0; i < n; ++i, ++cur_alloc_frames_) {
    frame = av_frame_alloc();
    if (!frame) {
      LOG << "alloc frames failed, cur: " << cur_alloc_frames_
          << " max: " << max_alloc_frames_
          << " ENOMEM: " << ENOMEM;
      return;
    }
    free_frames_.push_back(frame);
  }
}

AVFrame* AVFramesManager::GetDecoded(int64_t frame_id)
{
  if (decoded_frames_.empty())
    return NULL;
  auto itr = decoded_frames_.find(frame_id);
  if (itr == decoded_frames_.end()) {
    LOG << "ERR: frame not found, frame_id: " << frame_id;
    return NULL;
  }
//  auto itr = decoded_frames_.begin();
//  frame_id = itr->first;
  AVFrame *frame = itr->second;
  decoded_frames_.erase(itr);
  in_use_decoded_frames_.emplace(frame_id, frame);
  return frame;
}

vector<pair<int64_t, AVFrame *> > AVFramesManager::GetDecoded(const vector<int64_t> &frame_ids)
{
  vector<pair<int64_t, AVFrame *> frames;
  frames.reserve(frame_ids.size());
  for (int64_t fid : frame_ids) {
    auto itr = decoded_frames_.find(fid);
    if (itr == decoded_frames_.end()) {
      LOG << "WARNING, frame not found, frame_id: " << fid;
      continue;
    }
    frames.push_back({fid, itr->second});
  }
  return frames;
}

void AVFramesManager::PutDecoded(int64_t frame_id, AVFrame* frame)
{
  alloc_bytes_ += calFrameBytes(frame);
  decoded_frames_.emplace(frame_id_++, frame);
  return;
}

AVFrame* AVFramesManager::GetFree()
{
  if (free_frames_.empty()) {
    if (alloc_bytes_ >= max_alloc_bytes_) {
      LOG << "ERR: can not alloc more AVFrame,"
             " because already alloc too much memory: " << (alloc_bytes_ >> 20)
          << " MB, max: " << (max_alloc_bytes_ >> 20) << " MB";
      return;
    }

//    if (cur_alloc_frames_ == max_alloc_frames_) {
//      LOG << "already alloc max frames: " << max_alloc_frames_;
//      return NULL;
//    }
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
      LOG << "av_frame_alloc NULL, total alloced: " << (alloc_bytes_ >> 20)
          << " MB, max: " << (max_alloc_bytes_ >> 20) << " MB, ENOMEM: " << ENOMEM;
      return NULL;
    }
//    ++cur_alloc_frames_;
    free_frames_.push_back(frame);
  }
  AVFrame *frame = free_frames_.back();
  frame->format = AV_PIX_FMT_NONE;
  free_frames_.pop_back();
  return frame;
}

void AVFramesManager::PutFree(int64_t frame_id, AVFrame* frame)
{
  auto itr = in_use_decoded_frames_.find(frame_id);
  if (itr == in_use_decoded_frames_.end()) {
    LOG << "ERR: frame not found in in_use_decoded_frames, frame_id: "
        << frame_id << " ptr: " << frame;
  }

  free_frames_.push_back(frame);
  LOG << "decoded:" << decoded_frames_.size()
     << " free:" << free_frames_.size()
     << " cur_alloc:" << cur_alloc_frames_
     << " max:" << max_alloc_frames_;
}

string AVFramesManager::Stat()
{
  ostringstream os;
  os << "decoded:" << decoded_frames_.size()
     << " free:" << free_frames_.size()
     << " cur_alloc:" << cur_alloc_frames_
     << " max:" << max_alloc_frames_;
  return os.str();
}

int64_t AVFramesManager::calFrameBytes(AVFrame *frame)
{
  int64_t bytes = 0;
  switch (frame->format) {
  case AV_PIX_FMT_YUV420P:
    {
      bytes = frame->linesize[0] * frame->height +
              (frame->linesize[1] + frame->linesize[2]) * (frame->height >> 1);
    }
    break;
  case AV_PIX_FMT_NV12:
    {
      bytes = frame->linesize[0] * frame->height +
              frame->linesize[1] * (frame->height >> 1);
    }
    break;
  }
  return bytes;
}
