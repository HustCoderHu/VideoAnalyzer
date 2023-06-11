#include <sstream>
#include "gop_recorder.h"
#include "mylog.h"

using std::ostringstream;

extern bool PacketIsKeyFrame(const AVPacket* packet);

GopRecorder::GopRecorder()
{

}

void GopRecorder::UpdatePacket(int64_t frame_id, const AVPacket* packet)
{
  FrameKey fk;
  fk.frame_id = frame_id;
  fk.dts = packet->dts;
  fk.pts = packet->pts;

  if (keyframe_map.empty() && ! PacketIsKeyFrame(packet)) {
    LOG << "delta frame pts:" << packet->pts;
  }

  if (PacketIsKeyFrame(packet))
    keyframe_map.emplace(fk, fk.dts);
}

const FrameKey* GopRecorder::GetGopFirstKeyFrame(int64_t frame_id)
{
  FrameKey k;
  k.frame_id = frame_id;
  auto it = keyframe_map.lower_bound(k);
  const FrameKey& fk = it->first;
  if (fk.frame_id == frame_id)
    return &fk;

  if (it == keyframe_map.begin()) {
    // delta frame id is smaller than first gop key frame id
    return nullptr;
  }

  it--;
  return &(it->first);
}

const FrameKey* GopRecorder::GetLastGopKeyFrame()
{
  if (keyframe_map.empty())
    return nullptr;
  return &(keyframe_map.rbegin()->first);
}

string GopRecorder::String()
{
  ostringstream os;
  os << '[';
  for (auto &p : keyframe_map) {
    const FrameKey& fk = p.first;
    os << "id:" << fk.frame_id << " pts:" << fk.pts << ", ";
  }
  os << ']';
  return os.str();
}

