# -*- coding: utf-8 -*-
"""
Test the whole loop on your computer first, with no hardware (no ESP32 needed).

Usage:
  1. In one terminal, start the server:  python server.py
  2. In this terminal, run it:           python test_client.py
"""

import requests

BASE = "http://127.0.0.1:5001"


def get_story(sensor):
    r = requests.post(f"{BASE}/story", json=sensor).json()
    print("🔆 It says:", r["story"])
    print("   Current personality:", r["traits"])
    return r["story"]


def give_feedback(text, last_story):
    r = requests.post(f"{BASE}/feedback",
                      json={"feedback": text, "last_story": last_story}).json()
    print("🛠  Feedback applied")
    print("   Its mood:", r.get("diary_note"))
    print("   Personality after the nudge:", r["updated_traits"])
    print("   Habits picked up:", r["quirks"])


if __name__ == "__main__":
    # Day 1: a cold, gloomy night
    print("\n=== Day 1 ===")
    s = get_story({"temperature": 14, "humidity": 80, "light": 30, "is_day": 0})

    # You feel it's too bland and want it more playful and more emotional.
    print("\nYou gave feedback: 'Too bland — I'd like you to be more playful and more emotional'")
    give_feedback("Too bland — I'd like you to be more playful and more emotional", s)

    # Day 2: a bright, warm afternoon — let's see whether the personality has already changed
    print("\n=== Day 2 (same routine, but the personality has shifted) ===")
    s = get_story({"temperature": 26, "humidity": 45, "light": 800, "is_day": 1})

    # One more piece of feedback, pointing the opposite way.
    print("\nYou gave feedback: 'This time it's a bit too loud — quieter and more mysterious would be better'")
    give_feedback("This time it's a bit too loud — quieter and more mysterious would be better", s)

    print("\n(Run it a few more times and you'll watch the personality values drift bit by bit — that's it 'growing up'.)")
