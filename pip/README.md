# Pip — A Desk Lamp That Grows Its Own Personality 🔆

A virtual desk lamp with its own moods and personality. You can chat with it, give it feedback, and watch its personality shift bit by bit.

---

## Project Files

```
pip-lamp/
├── server.py          # Flask backend (calls the Gemini API)
├── chat.html          # Web chat interface
├── test_client.py     # Terminal test script
└── requirements.txt   # Python dependencies
```

Generated automatically once it's running:

```
pip-lamp/
├── personality.json   # Pip's personality profile
└── history.json       # Conversation memory
```

---

## Step 1: Set Up Your Environment

### 1. Install Python

You need Python 3.10 or higher. Open a terminal and run the following to confirm:

```bash
python3 --version
```

### 2. Get a Gemini API Key

This uses Google's Gemini 2.5 Flash model (available on the free tier).

1. Go to [Google AI Studio](https://aistudio.google.com/apikey) and sign in with your Google account.
2. Click **Create API key** and copy the key it generates.

---

## Step 2: Create the Project Folder

```bash
# Create the folder and enter it
mkdir pip-lamp
cd pip-lamp
```

Put the four files inside. Place them manually under the `pip-lamp/` directory, making sure the structure matches the "Project Files" overview above.

---

## Step 3: Create a Virtual Environment and Install Dependencies

### macOS / Linux

```bash
# Create the virtual environment
python3 -m venv venv

# Activate it (once active, you'll see (venv) appear at the front of your prompt)
source venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

### Windows

```bash
# Create the virtual environment
python -m venv venv

# Activate it
venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt
```

---

## Step 4: Configure Your API Key

Before starting, set your own Gemini API key as an environment variable.

### macOS / Linux

```bash
export GEMINI_API_KEY="paste your key here"
```

### Windows (CMD)

```bash
set GEMINI_API_KEY=paste your key here
```

### Windows (PowerShell)

```powershell
$env:GEMINI_API_KEY="paste your key here"
```

> An environment variable set this way only lasts in the current terminal window. You'll need to set it again after closing the terminal.

---

## Step 5: Start the Server

```bash
python server.py
```

---

## Step 6: Test It

Open your browser and go to:

```
http://127.0.0.1:5001
```

The left side is the chat window; the right side is Pip's personality panel. You can:

- **💬 Chat**: Talk to Pip about anything — it'll respond in its own personality.
- **🛠 Feedback**: Switch to "Feedback" mode and tell Pip what you think of it (e.g. "too wordy," "be a little more playful"), and its personality values will fine-tune accordingly.
- **📡 Sensors**: Drag the temperature, humidity, and light sliders on the right to simulate environmental changes, letting Pip sense them and react.
- **🧹 Clear Memory**: Erase the conversation history while keeping the personality growth record.
- **🔄 Reset Personality**: Return to factory settings and start fresh.

You can also test the whole loop from the terminal without a browser:

```bash
# In a second terminal (with the server still running)
python test_client.py
```

---

### Want to access it from another computer?

Keep the `0.0.0.0` in the startup command unchanged, then use your own LAN IP:

```bash
# Find your LAN IP
# macOS/Linux:
ifconfig | grep "inet "
# Windows:
ipconfig
```

Then visit `http://your-IP:5001` in your browser.

### Want to change the port?

Change `port=5001` on the last line of `server.py` to whatever port you want.
