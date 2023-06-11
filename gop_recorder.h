
#pragma once
#include <stdint.h>
#include <string>
#include <map>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavutil/imgutils.h>
//#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
}

using std::string;
using std::map;

struct FrameKey {
  int64_t frame_id = -1;
  int64_t dts = INT64_MIN;
  int64_t pts = INT64_MIN;

  bool operator<(const FrameKey& r) const {
    if (frame_id != r.frame_id)
      return frame_id < r.frame_id;
    return pts < r.pts;
  }
};

class GopRecorder
{
public:
  GopRecorder();

  void UpdatePacket(int64_t frame_id, const AVPacket* packet);

  const FrameKey* GetGopFirstKeyFrame(int64_t delta_frame_id);
  const FrameKey* GetLastGopKeyFrame();

  string String();
private:
  map<FrameKey, int64_t> keyframe_map;
};

