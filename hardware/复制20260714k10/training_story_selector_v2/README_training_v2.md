# Weather Story Tiny Selector v2 训练说明

本目录独立于旧训练文件，保持固件现有定义：14D 顺序为 10 个 weather 特征后接 `calm/weird/warm/lonely`，mood id 顺序为 `quiet/warm/strange/lonely`（0/1/2/3）。ESP32 只执行 inference，不在设备上训练。

## 环境

建议使用 Python 3.11 的独立虚拟环境。系统缺少 TensorFlow 时先安装：

```powershell
py -3.11 -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

TensorFlow 安装受操作系统与 CPU 支持影响；如果 `tensorflow` wheel 不适配当前平台，请在兼容的 Python 3.11/TensorFlow 环境执行这些脚本，不要改成 ESP32 端训练。

## 完整流程

```powershell
cd training_story_selector_v2
python generate_training_data_v2.py
python train_selector_v2.py
python sensitivity_test.py
python ablation_test.py
python convert_to_tflite_v2.py
python export_model_header_v2.py
```

数据生成使用题目给定的四组 score：天气项与相应 personality 项共同决定最大分标签，personality 权重明确为 `0.70`；小幅 score jitter 避免边界过于机械。随机样本按标签平衡，同时加入五种固定天气 × 四种 personality 的 controlled contrast groups。CSV 的 label 仍是最高 score 的离散 mood；模型训练将同一组 scores 以 temperature `0.25` 平滑成 soft targets，以便输出概率表达 score 强弱并避免 one-hot softmax 饱和。

两个模型均为 `Dense(16, relu) -> Dense(8, relu) -> Dense(4, softmax)`：full model 输入 14D，weather-only ablation 输入前 10D。训练与测试使用同一固定随机划分，便于公平比较。

## Arduino 替换方式

- `model_v2.h` 使用新数组名 `story_mood_model_v2_tflite` / `_len`，接入时需同步修改 `StoryMoodModel.cpp` 对数组名的引用。
- `model_v2_compatible.h` 保留固件当前使用的 `story_mood_model_tflite` / `_len`。验证通过后，可把它复制为 Arduino 工程中的 `model.h`，无需改运行逻辑。
- 当前 `WeatherStory_TinySelector/model.h` 不会被脚本覆盖，旧模型始终保留，便于回退。

替换前应确认 `sensitivity_report_v2.md` 和 `ablation_report_v2.md` 均通过，并在目标 ESP32/TFLite Micro 运行环境做一次模型初始化与 14D 输入 smoke test。
