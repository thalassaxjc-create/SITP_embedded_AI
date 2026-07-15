#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\PersonalityState.h"
#ifndef PERSONALITY_STATE_H
#define PERSONALITY_STATE_H

// PersonalityState = 设备的长期偏好。
// 第一阶段先用固定值；之后可以根据用户反馈更新，并保存到 Flash。
struct PersonalityState {
  float calm;
  float weird;
  float warm;
  float lonely;
};

inline PersonalityState makeDefaultPersonalityState() {
  PersonalityState p;
  p.calm = 0.55;
  p.weird = 0.30;
  p.warm = 0.40;
  p.lonely = 0.25;
  return p;
}

#endif
