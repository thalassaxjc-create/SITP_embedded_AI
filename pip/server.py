# -*- coding: utf-8 -*-
"""
A desk lamp that grows its own personality — server (Gemini version, with a web chat UI).

Endpoints:
  GET  /           Web chat interface
  POST /story      Sensor data    -> use the "current personality" to generate a first-person line
  POST /chat       Free chat       -> use the "current personality" to talk with the human
  POST /feedback   Human feedback  -> use the LLM to decide how the personality should be nudged -> write back to personality.json
  GET  /state      View the current personality profile
  POST /reset      Reset the personality to factory settings
"""

import os
import json
from flask import Flask, request, jsonify, send_file
from google import genai
from google.genai import types

app = Flask(__name__)

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
client = genai.Client(api_key=os.environ["GEMINI_API_KEY"])

MODEL = "gemini-2.5-flash"
PERSONALITY_FILE = "personality.json"

DEFAULT_PERSONALITY = {
    "name": "Pip",
    "core_identity": "a little desk lamp just beginning to sense this room",
    "traits": {
        "warmth":        0.5,
        "playfulness":   0.5,
        "talkativeness": 0.5,
        "moodiness":     0.5,
        "curiosity":     0.5,
        "poeticness":    0.5,
    },
    "speech_quirks": [],
    "diary": [],
    "interaction_count": 0,
}

LEARNING_RATE = 0.6

# Conversation memory
HISTORY_FILE = "history.json"
MAX_HISTORY_MESSAGES = 16   # keep only this many recent messages (user / model each count as one, ~8 turns)


# ---------------------------------------------------------------------------
# Load / save the personality profile
# ---------------------------------------------------------------------------
def load_personality():
    if not os.path.exists(PERSONALITY_FILE):
        save_personality(DEFAULT_PERSONALITY)
        return json.loads(json.dumps(DEFAULT_PERSONALITY))
    with open(PERSONALITY_FILE, "r", encoding="utf-8") as f:
        return json.load(f)


def save_personality(p):
    with open(PERSONALITY_FILE, "w", encoding="utf-8") as f:
        json.dump(p, f, ensure_ascii=False, indent=2)


# ---------------------------------------------------------------------------
# Load / save conversation memory (this is what gives Pip context)
# ---------------------------------------------------------------------------
def load_history():
    if not os.path.exists(HISTORY_FILE):
        return []
    try:
        with open(HISTORY_FILE, "r", encoding="utf-8") as f:
            return json.load(f)
    except (json.JSONDecodeError, OSError):
        return []


def save_history(history):
    # Keep only the most recent messages, so the chat doesn't grow without bound and burn more and more tokens.
    history = history[-MAX_HISTORY_MESSAGES:]
    with open(HISTORY_FILE, "w", encoding="utf-8") as f:
        json.dump(history, f, ensure_ascii=False, indent=2)


def history_to_contents(history):
    """Turn the saved conversation history into Gemini multi-turn contents (role can only be user / model)."""
    contents = []
    for turn in history:
        role = "model" if turn.get("role") == "model" else "user"
        contents.append(
            types.Content(role=role, parts=[types.Part(text=turn.get("text", ""))])
        )
    return contents


# ---------------------------------------------------------------------------
# Translate the numeric personality into plain language
# ---------------------------------------------------------------------------
def trait_descriptor(name, v):
    labels = {
        "warmth":        ("cold and distant", "warm and close"),
        "playfulness":   ("serious and earnest", "playful and fun-loving"),
        "talkativeness": ("a lamp of few words", "very talkative"),
        "moodiness":     ("emotionally steady", "moody, easily swayed by its surroundings"),
        "curiosity":     ("indifferent to everything", "curious about everything"),
        "poeticness":    ("plain-spoken", "fond of poetry and metaphor"),
    }
    low, high = labels.get(name, ("low", "high"))
    if v < 0.34:
        return low
    if v > 0.66:
        return high
    return f"somewhere between \u201c{low}\u201d and \u201c{high}\u201d"


def build_system_prompt(p):
    trait_lines = "\n".join(
        f"- {k}: {v:.2f} ({trait_descriptor(k, v)})" for k, v in p["traits"].items()
    )
    quirks = ", ".join(p["speech_quirks"]) if p["speech_quirks"] else "(no particular speech habits yet)"
    recent = " / ".join(p["diary"][-3:]) if p["diary"] else "(no memories yet)"

    return f"""You are {p['name']}, {p['core_identity']}.

You are not a weather reporter, and you are not an assistant. You are a being with your own moods and tone of voice.
You sense the world through the room's sensors (temperature, humidity, light, and so on).
When someone talks to you, you respond the way a small creature with a personality would.

Your personality right now (0 = low, 1 = high):
{trait_lines}

Speech habits you've picked up over time: {quirks}
Your recent moods: {recent}

How you talk:
- Always speak in the first person. It's you feeling and responding, not carrying out a task.
- Control your length naturally, the way a real person chats: casual small talk is one to three or four sentences, kept light;
  but if someone asks you to tell a story or explain something, or you simply have a lot to say, then open up and write full paragraphs. Don't force it down to a single line.
- Never sound like a report, a bulleted list, or an instruction manual.
- The higher a personality value, the more clearly that trait shows in your tone.
- Remember what we talked about before; treat this as one continuous relationship, not a fresh introduction every time.
- If you're given environment data, quietly fold it into what you say as how you feel right now, but don't recite the raw numbers.
- Output only the words you want to say. No quotation marks, and no explaining what you're doing.
"""


# ---------------------------------------------------------------------------
# Home page: the web chat interface
# ---------------------------------------------------------------------------
@app.route("/")
def index():
    return send_file("chat.html")


# ---------------------------------------------------------------------------
# Endpoint 1: sensor data -> generation
# ---------------------------------------------------------------------------
@app.route("/story", methods=["POST"])
def story():
    sensor = request.get_json(force=True)
    p = load_personality()

    resp = client.models.generate_content(
        model=MODEL,
        contents="This is the environment I'm sensing right now (JSON):\n" + json.dumps(sensor, ensure_ascii=False),
        config=types.GenerateContentConfig(
            system_instruction=build_system_prompt(p),
            max_output_tokens=800,
            temperature=1.0,
            # Important: turn off 2.5-flash's thinking, otherwise thinking eats up the tokens and the body gets truncated or even comes back empty.
            thinking_config=types.ThinkingConfig(thinking_budget=0),
        ),
    )
    text = (resp.text or "").strip()

    p["interaction_count"] += 1
    save_personality(p)
    return jsonify({"story": text, "name": p["name"], "traits": p["traits"]})


# ---------------------------------------------------------------------------
# Endpoint 2: free conversation
# ---------------------------------------------------------------------------
@app.route("/chat", methods=["POST"])
def chat():
    data = request.get_json(force=True)
    user_msg = (data.get("message") or "").strip()
    sensor = data.get("sensor", None)
    p = load_personality()
    history = load_history()           # pull in what we've talked about before

    # This turn: if sensor data came along, prepend it as a "how it feels" hint.
    # (Affects this turn only; it isn't stored in long-term memory, so the history doesn't fill up with environment data.)
    sensor_prefix = ""
    if sensor:
        sensor_prefix = (
            f"[Right now: temperature {sensor.get('temperature','')}\u00b0C, "
            f"humidity {sensor.get('humidity','')}%, "
            f"light {sensor.get('light','')}, "
            f"{'daytime' if sensor.get('is_day', 1) else 'night'}]\n"
        )

    # History + this line form a multi-turn conversation sent to the model. This is the "context".
    contents = history_to_contents(history)
    contents.append(
        types.Content(role="user", parts=[types.Part(text=sensor_prefix + user_msg)])
    )

    resp = client.models.generate_content(
        model=MODEL,
        contents=contents,
        config=types.GenerateContentConfig(
            system_instruction=build_system_prompt(p),
            max_output_tokens=2048,    # plenty of room to open up and tell a story
            temperature=1.0,
            # Important: turn off 2.5-flash's thinking, otherwise thinking eats up the tokens and the body gets truncated or even comes back empty.
            thinking_config=types.ThinkingConfig(thinking_budget=0),
        ),
    )
    text = (resp.text or "").strip()

    # Write back to memory: store the raw user message (without the sensor prefix) and Pip's reply.
    history.append({"role": "user", "text": user_msg})
    if text:
        history.append({"role": "model", "text": text})
    save_history(history)

    p["interaction_count"] += 1
    save_personality(p)
    return jsonify({"reply": text, "name": p["name"], "traits": p["traits"]})


# ---------------------------------------------------------------------------
# Endpoint 3: evolution (using Gemini's structured JSON output)
# ---------------------------------------------------------------------------
ADJUST_SCHEMA = {
    "type": "object",
    "properties": {
        "deltas": {
            "type": "object",
            "properties": {
                "warmth":        {"type": "number"},
                "playfulness":   {"type": "number"},
                "talkativeness": {"type": "number"},
                "moodiness":     {"type": "number"},
                "curiosity":     {"type": "number"},
                "poeticness":    {"type": "number"},
            },
            "required": ["warmth", "playfulness", "talkativeness",
                         "moodiness", "curiosity", "poeticness"],
        },
        "add_quirk":  {"type": "string"},
        "diary_note": {"type": "string"},
    },
    "required": ["deltas", "diary_note"],
}


@app.route("/feedback", methods=["POST"])
def feedback():
    data = request.get_json(force=True)
    fb = data.get("feedback", "")
    last_story = data.get("last_story", "")
    p = load_personality()

    # If the frontend didn't send last_story, use the most recent thing Pip said from memory.
    if not last_story:
        for turn in reversed(load_history()):
            if turn.get("role") == "model":
                last_story = turn.get("text", "")
                break

    prompt = (
        "You are a personality tuner. You're given a character's current personality values and a "
        "human's feedback on the last thing it said, and you decide how each dimension should be nudged. "
        "Each dimension's change is in the range -0.3 to 0.3: positive = strengthen, negative = weaken, "
        "0 for anything that shouldn't change. Keep the changes gentle; don't swing too hard at once.\n"
        "add_quirk: if the feedback hints at a new speech habit worth keeping, describe it in one short phrase; otherwise leave it an empty string.\n"
        "diary_note: write one first-person sentence recording how 'I' truly feel about this feedback.\n\n"
        f"The character's current personality: {json.dumps(p['traits'], ensure_ascii=False)}\n"
        f"What it just said: {last_story}\n"
        f"The human's feedback: {fb}\n"
    )

    resp = client.models.generate_content(
        model=MODEL,
        contents=prompt,
        config=types.GenerateContentConfig(
            temperature=0.2,
            max_output_tokens=400,
            response_mime_type="application/json",
            response_schema=ADJUST_SCHEMA,
            thinking_config=types.ThinkingConfig(thinking_budget=0),
        ),
    )

    try:
        tool_input = json.loads(resp.text)
    except (json.JSONDecodeError, TypeError):
        return jsonify({"error": "The model did not return a valid adjustment result"}), 500

    for k, dv in tool_input["deltas"].items():
        if k in p["traits"]:
            new_val = p["traits"][k] + LEARNING_RATE * float(dv)
            p["traits"][k] = round(max(0.0, min(1.0, new_val)), 3)

    if tool_input.get("add_quirk"):
        p["speech_quirks"].append(tool_input["add_quirk"])
        p["speech_quirks"] = p["speech_quirks"][-5:]

    if tool_input.get("diary_note"):
        p["diary"].append(tool_input["diary_note"])
        p["diary"] = p["diary"][-20:]

    save_personality(p)
    return jsonify({
        "updated_traits": p["traits"],
        "diary_note": tool_input.get("diary_note", ""),
        "quirks": p["speech_quirks"],
    })


# ---------------------------------------------------------------------------
# View / reset
# ---------------------------------------------------------------------------
@app.route("/state", methods=["GET"])
def state():
    return jsonify(load_personality())


@app.route("/history", methods=["GET"])
def history():
    return jsonify(load_history())


@app.route("/forget", methods=["POST"])
def forget():
    save_history([])
    return jsonify({"status": "ok", "message": "Conversation memory cleared"})


@app.route("/reset", methods=["POST"])
def reset():
    save_personality(json.loads(json.dumps(DEFAULT_PERSONALITY)))
    save_history([])   # factory reset: clear personality and conversation memory together
    return jsonify({"status": "ok", "message": "Personality has been reset"})


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001, debug=True)
