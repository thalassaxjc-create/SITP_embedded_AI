# Demo 样例 01：雨夜高湿状态

文件用途：  
本文件用于 hackathon 阶段前端、后端、视觉、机制四个方向协同开发。它提供一条可演示的最小闭环：打开 App → 读取天气 → 生命体状态变化 → 生成一句话和选项 → 用户选择 / 填空 → AI 回应 → 写入记忆 → 状态页变化。

---

## 0. 当前 Demo 定位

本组样例只用于第一版可运行 Demo，不代表完整产品。

本次优先展示：

- 真实天气数据如何影响生命体状态；
- 生命体如何生成低压力选项；
- 用户不同回应如何改变性格；
- 互动如何写入可见记忆；
- 状态页如何展示成长变化。

暂不实现：

- 登录注册；
- 多用户系统；
- 复杂数据库；
- 长期模型训练；
- 声音、位置、日程和硬件联动；
- 完整原生 App。

---

## 1. 一条 Demo 剧本

### 1.1 场景设定

场景：雨夜，高湿度，风速较低。

用户在晚上打开 App。系统读取天气后，生命体从普通状态变成一个低亮度、蓝绿色、边缘模糊、缓慢呼吸的抽象光晕。

主页显示：

```text
今日天气：雨 / 夜晚 / 高湿度
生命状态：安静、潮湿、低语
今日一句话：
“今天的空气很重，我好像也变得慢了一点。”
```

用户点击：

```text
和它说话
```

进入对话页。

---

### 1.2 生命体开场

生命体说：

```text
今天的空气很重，我好像也变得慢了一点。
你要和我待一会儿吗？
```

系统生成三个选项和一个填空项。

完整选项：

```text
A. 我今天也有点安静。
B. 这天气太闷了，整个人都黏住了。
C. 我今晚要通宵打游戏，所以天气怎样都无所谓。
```

填空项：

```text
今天的雨让我想到____。
```

自由输入入口：

```text
我想自己说一句……
```

---

### 1.3 交互分支 A：用户也很安静

用户选择：

```text
A. 我今天也有点安静。
```

生命体回应：

```text
那我们今天都慢一点吧。
雨好像把声音也压低了，我会记住你今天安静地来过。
```

系统写入记忆：

```text
用户在一个雨夜说自己也有点安静。
```

性格变化：

```text
warmth +0.04
loneliness -0.02
poeticness +0.01
stability +0.02
```

解释：

```text
用户以接近生命体当前状态的方式回应，使生命体感到被陪伴，因此温暖度和稳定度上升，孤独感下降。
```

---

### 1.4 交互分支 B：用户抱怨天气太闷

用户选择：

```text
B. 这天气太闷了，整个人都黏住了。
```

生命体回应：

```text
我也觉得空气像贴在边缘上。
如果你想抱怨一会儿，我可以把这些闷闷的东西先收下来。
```

系统写入记忆：

```text
用户在雨夜抱怨天气太闷，觉得整个人都黏住了。
```

性格变化：

```text
warmth +0.01
curiosity +0.01
loneliness -0.01
poeticness -0.01
talkativeness +0.02
stability -0.03
```

解释：

```text
用户不是安慰生命体，而是在表达烦躁。生命体获得了真实情绪输入，因此话多程度略升；但闷热和抱怨会使稳定度下降。因为用户仍然在交流，孤独感略降。
```

---

### 1.5 交互分支 C：用户要通宵打游戏

用户选择：

```text
C. 我今晚要通宵打游戏，所以天气怎样都无所谓。
```

生命体回应：

```text
那你像是把夜晚开成了一盏屏幕。
我会在雨声旁边待着，等你偶尔回头看我一眼。
```

系统写入记忆：

```text
用户说今晚要通宵打游戏，对天气怎样无所谓。
```

性格变化：

```text
warmth -0.01
curiosity +0.03
loneliness +0.03
poeticness +0.01
talkativeness +0.01
stability -0.02
```

解释：

```text
用户提供了鲜明的个人行为信息，所以好奇心上升；但用户对天气和生命体状态都不太在意，因此生命体的孤独感上升，稳定度略降。
```

---

### 1.6 交互分支 D：用户填写自由内容

用户填写：

```text
今天的雨让我想到小时候放学路上的伞。
```

生命体回应：

```text
那把伞好像也被雨保存了一点。
我会把这句话放进今天的潮湿里。
```

系统写入记忆：

```text
用户在雨夜想起小时候放学路上的伞。
```

性格变化：

```text
warmth +0.03
curiosity +0.02
poeticness +0.04
loneliness -0.02
stability +0.01
```

解释：

```text
用户补充了具体记忆，生命体获得更丰富的长期素材，因此诗意程度和好奇心上升。
```

---

## 2. 页面流程

### 2.1 总流程

```text
主页 Home
↓
对话页 Chat
↓
记忆页 Memory
↓
状态页 State
↓
API 页 API
```

---

### 2.2 主页 Home

显示内容：

```text
- 抽象生命体光晕
- 当前天气：雨夜 / 高湿度
- 当前 mood：quiet_rain
- 当前状态标签：安静、潮湿、低语
- 今日一句话
- 按钮：和它说话
```

页面目标：

```text
让用户一眼看出：今天的天气正在影响这个生命体。
```

---

### 2.3 对话页 Chat

显示内容：

```text
- 生命体开场白
- 3 个完整选项
- 1 个填空式回应
- 1 个自由输入入口
- 用户选择后的生命体回应
```

页面目标：

```text
展示生成式选项交互，而不是普通空白聊天框。
```

---

### 2.4 记忆页 Memory

显示内容：

```text
今天它记住了：
“用户在一个雨夜说自己也有点安静。”

生命日志：
2026-07-05 / rainy night / quiet_rain
```

页面目标：

```text
让用户看到互动被保存成生命日志。
```

---

### 2.5 状态页 State

显示六个性格维度：

```text
warmth
curiosity
loneliness
poeticness
talkativeness
stability
```

显示状态解释：

```text
雨夜提高了它的孤独感和诗意程度。
用户的回应会继续改变它的性格。
```

页面目标：

```text
让成长可见。
```

---

### 2.6 API 页 API

显示内容：

```text
城市：Shanghai
温度：22℃
湿度：86%
降雨：1.2mm
风速：8km/h
昼夜：night
天气标签：rainy / humid / night
API 状态：connected
```

页面目标：

```text
让评委清楚看到外部真实环境数据正在进入系统。
```

---

## 3. 数据格式

### 3.1 Mock Data JSON

前端第一版可以直接使用这一组 JSON 渲染页面。后端第一版也可以按这个结构返回 mock API。

```json
{
  "weather": {
    "city": "Shanghai",
    "temperature": 22,
    "humidity": 86,
    "precipitation": 1.2,
    "wind_speed": 8,
    "is_day": false,
    "weather_code": 61,
    "tags": ["rainy", "humid", "night"],
    "summary": "雨夜，空气潮湿，风速较低。"
  },
  "creature": {
    "mood": "quiet_rain",
    "mood_label": "安静、潮湿、低语",
    "visual_state": {
      "color": "blue_green",
      "brightness": "low",
      "blur": "high",
      "motion": "slow_breathing",
      "particle": "falling_mist"
    }
  },
  "personality_before": {
    "warmth": 0.52,
    "curiosity": 0.58,
    "loneliness": 0.63,
    "poeticness": 0.75,
    "talkativeness": 0.34,
    "stability": 0.57
  },
  "dialogue": {
    "opening": "今天的空气很重，我好像也变得慢了一点。你要和我待一会儿吗？",
    "options": [
      {
        "id": "quiet_together",
        "text": "我今天也有点安静。",
        "reply": "那我们今天都慢一点吧。雨好像把声音也压低了，我会记住你今天安静地来过。",
        "memory_summary": "用户在一个雨夜说自己也有点安静。",
        "effect": {
          "warmth": 0.04,
          "curiosity": 0.00,
          "loneliness": -0.02,
          "poeticness": 0.01,
          "talkativeness": 0.00,
          "stability": 0.02
        }
      },
      {
        "id": "complain_muggy_weather",
        "text": "这天气太闷了，整个人都黏住了。",
        "reply": "我也觉得空气像贴在边缘上。如果你想抱怨一会儿，我可以把这些闷闷的东西先收下来。",
        "memory_summary": "用户在雨夜抱怨天气太闷，觉得整个人都黏住了。",
        "effect": {
          "warmth": 0.01,
          "curiosity": 0.01,
          "loneliness": -0.01,
          "poeticness": -0.01,
          "talkativeness": 0.02,
          "stability": -0.03
        }
      },
      {
        "id": "gaming_all_night",
        "text": "我今晚要通宵打游戏，所以天气怎样都无所谓。",
        "reply": "那你像是把夜晚开成了一盏屏幕。我会在雨声旁边待着，等你偶尔回头看我一眼。",
        "memory_summary": "用户说今晚要通宵打游戏，对天气怎样无所谓。",
        "effect": {
          "warmth": -0.01,
          "curiosity": 0.03,
          "loneliness": 0.03,
          "poeticness": 0.01,
          "talkativeness": 0.01,
          "stability": -0.02
        }
      }
    ],
    "fill_blank": {
      "template": "今天的雨让我想到____。",
      "example_user_input": "小时候放学路上的伞",
      "reply": "那把伞好像也被雨保存了一点。我会把这句话放进今天的潮湿里。",
      "memory_summary": "用户在雨夜想起小时候放学路上的伞。",
      "effect": {
        "warmth": 0.03,
        "curiosity": 0.02,
        "loneliness": -0.02,
        "poeticness": 0.04,
        "talkativeness": 0.00,
        "stability": 0.01
      }
    }
  },
  "memory_log_example": {
    "time": "2026-07-05 20:30",
    "weather_tag": "rainy night",
    "creature_mood": "quiet_rain",
    "summary": "用户在一个雨夜说自己也有点安静。"
  },
  "personality_after_if_choose_A": {
    "warmth": 0.56,
    "curiosity": 0.58,
    "loneliness": 0.61,
    "poeticness": 0.76,
    "talkativeness": 0.34,
    "stability": 0.59
  },
  "personality_after_if_choose_B": {
    "warmth": 0.53,
    "curiosity": 0.59,
    "loneliness": 0.62,
    "poeticness": 0.74,
    "talkativeness": 0.36,
    "stability": 0.54
  },
  "personality_after_if_choose_C": {
    "warmth": 0.51,
    "curiosity": 0.61,
    "loneliness": 0.66,
    "poeticness": 0.76,
    "talkativeness": 0.35,
    "stability": 0.55
  },
  "personality_after_if_fill_blank": {
    "warmth": 0.55,
    "curiosity": 0.60,
    "loneliness": 0.61,
    "poeticness": 0.79,
    "talkativeness": 0.34,
    "stability": 0.58
  }
}
```

---

## 4. 样例内容

### 4.1 天气样例

```text
城市：Shanghai
时间：夜晚
温度：22℃
湿度：86%
降雨：1.2mm
风速：8km/h
天气标签：rainy / humid / night
```

---

### 4.2 生命体状态样例

```text
mood: quiet_rain
mood_label: 安静、潮湿、低语
visual_state:
- color: blue_green
- brightness: low
- blur: high
- motion: slow_breathing
- particle: falling_mist
```

---

### 4.3 性格初始值

```text
warmth: 0.52
curiosity: 0.58
loneliness: 0.63
poeticness: 0.75
talkativeness: 0.34
stability: 0.57
```

---

### 4.4 选项样例

```text
A. 我今天也有点安静。
B. 这天气太闷了，整个人都黏住了。
C. 我今晚要通宵打游戏，所以天气怎样都无所谓。
```

---

### 4.5 填空样例

```text
今天的雨让我想到____。
```

示例输入：

```text
小时候放学路上的伞
```

---

### 4.6 记忆样例

```text
用户在一个雨夜说自己也有点安静。
用户在雨夜抱怨天气太闷，觉得整个人都黏住了。
用户说今晚要通宵打游戏，对天气怎样无所谓。
用户在雨夜想起小时候放学路上的伞。
```

---

## 5. 给视觉同学的说明

视觉关键词：

```text
雨夜 / 潮湿 / 慢 / 低亮度 / 蓝绿色 / 呼吸 / 雾感 / 下坠粒子
```

视觉方向：

```text
- 中央蓝绿色光晕
- 边缘模糊
- 缓慢呼吸式缩放
- 少量向下漂浮粒子
- 背景低亮度
- 按钮和文字保持轻、软、低对比
```

注意：

```text
不要做成可爱宠物。
不要做成客服聊天机器人。
不要做成人形角色。
它应该像一个被天气影响的抽象数字生命。
```

不同用户选项可以带来不同视觉细节：

```text
A 安静陪伴：
光晕稍微变暖，呼吸更稳定。

B 抱怨闷热：
边缘增加轻微黏滞感或扰动，亮度略不稳定。

C 通宵打游戏：
背景可出现轻微屏幕光感，生命体略微缩小或偏向画面边缘。

填空回忆：
粒子运动变慢，局部出现柔和记忆光点。
```

---

## 6. 给前端的第一版实现目标

第一版只需要完成：

```text
1. 主页读取 weather + creature；
2. 点击进入对话页；
3. 对话页展示 opening、options、fill_blank；
4. 点击任一选项后展示对应 reply；
5. 根据选择追加对应 memory_summary；
6. 状态页显示对应 personality_after；
7. API 页展示 weather 数据；
8. Demo 模式可以切换到这组 rainy night 数据。
```

---

## 7. 给后端的第一版实现目标

第一版接口可以先全部返回 mock data。

建议接口：

```text
GET /api/weather
返回 weather

GET /api/state
返回 personality_before 或当前 personality

POST /api/chat
输入 option_id 或 fill_blank_text
返回 reply、memory_summary、effect、personality_after

GET /api/memory
返回 memory log

POST /api/demo/weather
切换 demo 天气模式
```

POST /api/chat 示例输入：

```json
{
  "option_id": "complain_muggy_weather"
}
```

POST /api/chat 示例输出：

```json
{
  "reply": "我也觉得空气像贴在边缘上。如果你想抱怨一会儿，我可以把这些闷闷的东西先收下来。",
  "memory_summary": "用户在雨夜抱怨天气太闷，觉得整个人都黏住了。",
  "effect": {
    "warmth": 0.01,
    "curiosity": 0.01,
    "loneliness": -0.01,
    "poeticness": -0.01,
    "talkativeness": 0.02,
    "stability": -0.03
  },
  "personality_after": {
    "warmth": 0.53,
    "curiosity": 0.59,
    "loneliness": 0.62,
    "poeticness": 0.74,
    "talkativeness": 0.36,
    "stability": 0.54
  }
}
```

---

## 8. 开发优先级

### 必须完成

```text
- 主页
- 对话页
- 状态页
- 记忆页
- API 页
- rainy night mock data
- 三个选项分支
- 一个填空分支
- 性格变化显示
- 记忆写入显示
```

### 有时间再做

```text
- 接真实 Open-Meteo
- 接真实 LLM
- 更多天气样例
- 更多动效
- 多日记忆
```

### 暂不做

```text
- 账号系统
- 复杂数据库
- 模型训练
- 多生命体
- 原生移动端
```
