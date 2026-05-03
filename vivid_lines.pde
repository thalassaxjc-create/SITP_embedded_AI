// Global variables
ArrayList<FloatingWord> words;
String targetSentence = "Hello, Creative World!"; // Target sentence, can be modified
String[] targetWords;
int state = 0; // 0: waiting for click, 1: animation in progress, 2: complete
int animStartTime;
int animDuration = 3000; // 3 seconds

void setup() {
  size(800, 600);
  textAlign(CENTER, CENTER);
  textSize(32);
  words = new ArrayList<FloatingWord>();
  targetWords = splitTokens(targetSentence, " ");
  println("================================================");
  println("  Ready. Click the canvas to start animation.");
  println("================================================");
}

void draw() {
  background(20);
  
  if (state == 0) {
    fill(150);
    text("Click anywhere to start", width/2, height/2);
  } else if (state == 1) {
    int elapsed = millis() - animStartTime;
    float progress = constrain(elapsed / float(animDuration), 0.0, 1.0);
    
    if (progress >= 1.0) {
      state = 2;
      println("------------------------------------------------");
      println("  Animation Complete!");
      println("  Assembled Sentence: " + targetSentence);
      println("------------------------------------------------");
    }
    
    // Update and display each word
    for (int i = 0; i < words.size(); i++) {
      FloatingWord w = words.get(i);
      w.update(progress);
      w.display();
    }
  } else if (state == 2) {
    drawSentenceByWords();

    fill(200);
    textSize(16);
    text("Sentence: " + targetSentence, width/2, height - 50);
    textSize(32);
  }
}

void mousePressed() {
  state = 1;
  animStartTime = millis();
  
  words.clear();
  for (int i = 0; i < targetWords.length; i++) {
    float startX = width/2 + random(-100, 100);
    float startY = height/2 + random(-100, 100);
    float startRotation = random(TWO_PI);
    float startVelX = random(-3, 3);
    float startVelY = random(-5, -1);
    
    words.add(new FloatingWord(
      targetWords[i],
      startX, startY,
      startVelX, startVelY,
      startRotation,
      i
    ));
  }
  
  println("New animation started...");
}

void drawSentenceByWords() {
  fill(255);
  float spacing = textWidth(" ");
  float totalWidth = 0;

  for (int i = 0; i < targetWords.length; i++) {
    totalWidth += textWidth(targetWords[i]);
    if (i < targetWords.length - 1) {
      totalWidth += spacing;
    }
  }

  float xCursor = (width - totalWidth) / 2;
  float y = 80;

  for (int i = 0; i < targetWords.length; i++) {
    String word = targetWords[i];
    float w = textWidth(word);
    text(word, xCursor + w / 2, y);
    xCursor += w + spacing;
  }
}

class FloatingWord {
  String word;
  float x, y;
  float startX, startY;
  float velX, velY;
  float rotation;
  float rotSpeed;
  float targetX, targetY;
  int targetIndex;
  
  FloatingWord(String _word, float _x, float _y, float _vx, float _vy, float _rot, int _index) {
    word = _word;
    x = _x;
    y = _y;
    startX = _x;
    startY = _y;
    velX = _vx;
    velY = _vy;
    rotation = _rot;
    rotSpeed = random(-0.05, 0.05);
    targetIndex = _index;
    
    float spacing = textWidth(" ");
    float totalWidth = 0;
    for (int i = 0; i < targetWords.length; i++) {
      totalWidth += textWidth(targetWords[i]);
      if (i < targetWords.length - 1) {
        totalWidth += spacing;
      }
    }

    float xStart = (width - totalWidth) / 2;
    for (int i = 0; i < targetIndex; i++) {
      xStart += textWidth(targetWords[i]) + spacing;
    }

    targetX = xStart + textWidth(word) / 2;
    targetY = 80;
  }
  
  void update(float progress) {
    float easeProgress = easeInOutCubic(progress);
    
    x = lerp(startX, targetX, easeProgress);
    y = lerp(startY, targetY, easeProgress);
    
    if (progress < 0.8) {
      float floatOffset = sin(progress * PI * 4 + targetIndex) * 15 * (1 - progress);
      x += floatOffset * cos(targetIndex);
      y += floatOffset * sin(targetIndex);
    }
    
    rotation = lerp(rotation, 0, easeProgress);
  }
  
  void display() {
    pushMatrix();
    translate(x, y);
    rotate(rotation);
    fill(255, 220, 100 + targetIndex * 20 % 155);
    text(word, 0, 0);
    popMatrix();
  }
  
  float easeInOutCubic(float t) {
    return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
  }
}
