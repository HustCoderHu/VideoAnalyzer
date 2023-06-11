ffmpeg decode video and opengl render AVFrames

# seek 设计

启动时完整扫一遍视频文件，记录下所有 I 帧的信息(frame_id, timestamp 等) 到 map 中

```cpp
std::map<FrameKey, PacketCriticalInfo> gop_map

struct FrameKey {
  int64_t frame_id;
  int64_t timestamp;
  bool operator<(const FrameKey& l, const FrameKey& r)
}
```

执行 seek 操作时就能根据