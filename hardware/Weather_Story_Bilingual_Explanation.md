# Weather Story / 天气小故事程序说明（中英双语）

## 1. What this program does / 这个程序在做什么

**中文：**  
这个程序把实时天气数据转换成一个简单的“天气情绪”和一段很短的英文小故事。它会连接 Wi-Fi，向 Open-Meteo 天气 API 请求当前天气，然后把温度、湿度、降雨、天气代码、风速、白天/夜晚等数据解析出来。接着，程序会把这些数值判断成几个更容易理解的标签，例如 `cool`、`rainy`、`cloudy`、`windy`、`night`。最后，它根据这些标签生成一句天气总结和一段三句左右的小故事。

**English:**  
This program turns live weather data into a simple “weather mood” and a short English story. It connects to Wi-Fi, requests current weather data from the Open-Meteo API, and parses values such as temperature, humidity, rain, weather code, wind speed, and day/night status. It then converts the raw numbers into readable tags such as `cool`, `rainy`, `cloudy`, `windy`, and `night`. Finally, it uses those tags to generate a weather summary sentence and a tiny three-part story.

---

## 2. Data flow / 数据流程

**中文：**

1. ESP32 启动。
2. 程序打开串口输出，方便调试。
3. 程序连接 Wi-Fi。
4. 程序访问 Open-Meteo API。
5. API 返回 JSON 格式的天气数据。
6. 程序解析 JSON，并保存到 `WeatherData` 结构体。
7. 程序调用 `makeTags()`，把数值天气转成语义标签。
8. 程序调用 `buildWeatherSentence()`，生成一句天气总结。
9. 程序调用 `buildTinyStory()`，生成一段小故事。
10. 程序在 Serial Monitor 中轮流显示：
    - Page 1: 天气摘要和天气句子
    - Page 2: 天气小故事
11. 程序会定期重新请求天气数据。

**English:**

1. The ESP32 starts.
2. The program opens the serial output for debugging.
3. The program connects to Wi-Fi.
4. The program calls the Open-Meteo API.
5. The API returns weather data in JSON format.
6. The program parses the JSON and stores it in the `WeatherData` struct.
7. The program calls `makeTags()` to convert numeric weather data into semantic tags.
8. The program calls `buildWeatherSentence()` to create a short weather summary.
9. The program calls `buildTinyStory()` to create a tiny story.
10. The program alternates between two Serial Monitor pages:
    - Page 1: weather summary and weather sentence
    - Page 2: tiny weather story
11. The program refreshes the weather data periodically.

---

## 3. Main data structures / 主要数据结构

### `WeatherData`

**中文：**  
`WeatherData` 保存 API 返回的原始天气数据。它比较接近“事实数据”。

**English:**  
`WeatherData` stores the raw weather values returned by the API. It is close to “factual data.”

Fields / 字段：

- `time`: API 返回的当前天气时间 / current weather timestamp from the API
- `temperature`: 温度，摄氏度 / temperature in Celsius
- `humidity`: 相对湿度，百分比 / relative humidity in percent
- `precipitation`: 总降水量，毫米 / total precipitation in millimeters
- `rain`: 雨量，毫米 / rain amount in millimeters
- `weatherCode`: WMO 天气代码 / WMO weather code
- `windSpeed`: 10 米高度风速，km/h / wind speed at 10 m, in km/h
- `isDay`: `1` 表示白天，`0` 表示夜晚 / `1` means day, `0` means night

### `WeatherTags`

**中文：**  
`WeatherTags` 保存程序自己推断出来的天气标签。它更接近“解释后的天气状态”。

**English:**  
`WeatherTags` stores the interpreted weather tags created by the program. It is closer to an “interpreted weather state.”

Examples / 例子：

- Temperature / 温度: `hot`, `cold`, `cool`, `mild`
- Air / 空气: `humid`, `dry`, `normal`
- Sky / 天空: `rainy`, `cloudy`, `stormy`, `clear`
- Wind / 风: `windy`, `calm`
- Light / 光线: `day`, `night`

---

## 4. Important functions / 重要函数

### `connectWiFi()`

**中文：**  
连接 Wi-Fi。如果连接成功，会打印 ESP32 的 IP 地址。如果失败，会在串口中显示错误信息。

**English:**  
Connects to Wi-Fi. If the connection succeeds, it prints the ESP32 IP address. If it fails, it prints an error message in the Serial Monitor.

### `buildWeatherUrl()`

**中文：**  
根据经纬度生成 Open-Meteo API 请求网址。这样修改地点时只需要改 `LATITUDE` 和 `LONGITUDE`。

**English:**  
Builds the Open-Meteo API URL from the latitude and longitude. This makes it easy to change the location by editing only `LATITUDE` and `LONGITUDE`.

### `fetchWeather(WeatherData &w)`

**中文：**  
向天气 API 发出 HTTPS 请求，接收 JSON，解析 JSON，并把结果填入 `WeatherData`。成功返回 `true`，失败返回 `false`。

**English:**  
Sends an HTTPS request to the weather API, receives JSON, parses it, and fills a `WeatherData` object. Returns `true` on success and `false` on failure.

### `makeTags(const WeatherData &w)`

**中文：**  
把数值天气转换成语义标签。例如：

- 温度大于等于 30°C → `hot`
- 温度小于等于 5°C → `cold`
- 湿度大于等于 75% → `humid`
- 风速大于等于 20 km/h → `windy`
- 有雨量、降水量或雨类天气代码 → `rainy`

**English:**  
Converts numeric weather values into semantic tags. For example:

- temperature >= 30°C → `hot`
- temperature <= 5°C → `cold`
- humidity >= 75% → `humid`
- wind speed >= 20 km/h → `windy`
- rain amount, precipitation, or rain-related weather code → `rainy`

### `buildWeatherSentence(const WeatherTags &t)`

**中文：**  
根据标签生成一句简单的天气总结，例如：  
`Today feels cool, cloudy, and calm under the night sky.`

**English:**  
Generates a simple weather summary sentence from the tags, for example:  
`Today feels cool, cloudy, and calm under the night sky.`

### `buildTinyStory(const WeatherTags &t)`

**中文：**  
根据天气标签从预设句子库中随机选句，生成一个三句左右的小故事。雨天、阴天、冷空气、有风、夜晚等状态会影响故事内容。

**English:**  
Randomly selects lines from preset text banks to create a tiny three-part story. Rain, clouds, cool air, wind, and night/day status influence the story.

### `printWeatherPage()` and `printStoryPage()`

**中文：**  
这两个函数代替了原来 K10 屏幕显示功能。它们把两页内容打印到 ESP32 的 Serial Monitor。

**English:**  
These two functions replace the original K10 screen display. They print two pages of content to the ESP32 Serial Monitor.

---

## 5. Difference from the original K10 version / 和原 K10 版本的区别

**中文：**

原版本依赖 `unihiker_k10.h`，并使用 K10 的屏幕和画布 API 显示内容。这个 ESP32 版本去掉了所有 K10 相关代码，因此可以在普通 ESP32 开发板上运行。显示方式改为 Serial Monitor 输出。

**English:**

The original version depends on `unihiker_k10.h` and uses the K10 screen/canvas API to display content. This ESP32 version removes all K10-specific code, so it can run on a generic ESP32 board. The display output has been changed to the Serial Monitor.

---

## 6. Required libraries / 所需库

**中文：**

在 Arduino IDE 中需要：

- ESP32 board support package
- `ArduinoJson` library
- ESP32 自带的 `WiFi.h`, `HTTPClient.h`, `WiFiClientSecure.h`

**English:**

In Arduino IDE, you need:

- ESP32 board support package
- `ArduinoJson` library
- ESP32 built-in `WiFi.h`, `HTTPClient.h`, `WiFiClientSecure.h`

---

## 7. How to use / 使用方法

**中文：**

1. 打开 Arduino IDE。
2. 安装 ESP32 board support。
3. 安装 `ArduinoJson`。
4. 新建一个 `.ino` 文件，复制 ESP32 版本代码。
5. 修改：
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
   - `LATITUDE`
   - `LONGITUDE`
6. 选择你的 ESP32 开发板。
7. 上传程序。
8. 打开 Serial Monitor，波特率设为 `115200`。
9. 查看天气摘要和小故事输出。

**English:**

1. Open Arduino IDE.
2. Install ESP32 board support.
3. Install `ArduinoJson`.
4. Create a new `.ino` file and paste the ESP32 version code.
5. Edit:
   - `WIFI_SSID`
   - `WIFI_PASSWORD`
   - `LATITUDE`
   - `LONGITUDE`
6. Select your ESP32 board.
7. Upload the program.
8. Open Serial Monitor and set the baud rate to `115200`.
9. Read the weather summary and tiny story output.

---

## 8. Notes for collaborators / 给协作者的备注

**中文：**
记得打开 USB CDC On Boot
- 这个程序不是在预测天气，而是在读取 API 给出的当前天气。
- 小故事不是 AI 生成的，而是从代码里的预设句子中随机组合出来的。
- 如果要换城市，只需要修改经纬度。
- 如果要扩展故事风格，可以增加更多句子数组。
- 如果要接 OLED、TFT 或其他屏幕，可以保留天气获取和故事生成逻辑，只替换 `printWeatherPage()` / `printStoryPage()` 的显示部分。
- `client.setInsecure()` 会跳过 HTTPS 证书校验，适合课堂/原型阶段；正式项目可以改为使用根证书。

**English:**
remember make USB CDC On Boot
- This program does not forecast weather. It reads the current weather provided by the API.
- The tiny story is not AI-generated. It is randomly assembled from preset sentence arrays inside the code.
- To change the city, edit the latitude and longitude.
- To expand the story style, add more sentence arrays.
- To connect an OLED, TFT, or another display, keep the weather-fetching and story-generation logic and replace only the `printWeatherPage()` / `printStoryPage()` display functions.
- `client.setInsecure()` skips HTTPS certificate verification, which is acceptable for classroom/prototype use. For production projects, use a root certificate instead.
