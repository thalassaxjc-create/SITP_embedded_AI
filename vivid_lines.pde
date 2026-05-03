// Global variables
ArrayList<FloatingChar> chars;
String targetSentence = "Hello, Creative World!"; // Target sentence, can be modified
int state = 0; // 0: waiting for click, 1: animation in progress, 2: complete
int animStartTime;
int animDuration = 3000; // 3 seconds

void setup() {
  size(800, 600);
  textAlign(CENTER, CENTER);
  textSize(32);
  chars = new ArrayList<FloatingChar>();
  println("================================================");
  println("  Ready. Click the canvas to start animation.");
  println("================================================");
}

void draw() {
  background(20);
  
  if (state == 0) {
    // Waiting state: display prompt text
    fill(150);
    text("Click anywhere to start", width/2, height/2);
  } else if (state == 1) {
    // Animation state
    int elapsed = millis() - animStartTime;
    float progress = constrain(elapsed / float(animDuration), 0.0, 1.0);
    
    if (progress >= 1.0) {
      // Animation complete, output sentence to console and display final result
      state = 2;
      println("------------------------------------------------");
      println("  Animation Complete!");
      println("  Assembled Sentence: " + targetSentence);
      println("------------------------------------------------");
    }
    
    // Update and display each character
    for (int i = 0; i < chars.size(); i++) {
      FloatingChar c = chars.get(i);
      c.update(progress);
      c.display();
    }
  } else if (state == 2) {
    // Animation complete state: draw complete sentence at the top
    fill(255);
    float totalWidth = textWidth(targetSentence);
    float xStart = (width - totalWidth) / 2 + textWidth(targetSentence.substring(0,1))/2;
    
    for (int i = 0; i < targetSentence.length(); i++) {
      char letter = targetSentence.charAt(i);
      float x = xStart + textWidth(targetSentence.substring(0, i)) + textWidth(letter)/2;
      float y = 80; // Top position
      fill(255);
      text(letter, x, y);
    }
    
    // Display complete sentence at the bottom for reference
    fill(200);
    textSize(16);
    text("Sentence: " + targetSentence, width/2, height - 50);
    textSize(32);
  }
}

void mousePressed() {
  // Reset animation
  state = 1;
  animStartTime = millis();
  
  // Generate random floating characters
  chars.clear();
  for (int i = 0; i < targetSentence.length(); i++) {
    // Each character spawns randomly from the screen center
    float startX = width/2 + random(-100, 100);
    float startY = height/2 + random(-100, 100);
    float startRotation = random(TWO_PI);
    float startVelX = random(-3, 3);
    float startVelY = random(-5, -1); // Slight upward initial velocity
    
    chars.add(new FloatingChar(
      targetSentence.charAt(i),
      startX, startY,
      startVelX, startVelY,
      startRotation,
      i // Target index for calculating final position
    ));
  }
  
  println("New animation started...");
}

// FloatingChar class
class FloatingChar {
  char letter;
  float x, y;
  float startX, startY;
  float velX, velY;
  float rotation;
  float rotSpeed;
  float targetX, targetY;
  int targetIndex;
  
  FloatingChar(char _letter, float _x, float _y, float _vx, float _vy, float _rot, int _index) {
    letter = _letter;
    x = _x;
    y = _y;
    startX = _x;
    startY = _y;
    velX = _vx;
    velY = _vy;
    rotation = _rot;
    rotSpeed = random(-0.05, 0.05);
    targetIndex = _index;
    
    // Calculate target position (top center)
    float totalWidth = textWidth(targetSentence);
    float xStart = (width - totalWidth) / 2;
    targetX = xStart + textWidth(targetSentence.substring(0, targetIndex)) + textWidth(letter)/2;
    targetY = 80;
  }
  
  void update(float progress) {
    // Use easing function for more natural animation
    float easeProgress = easeInOutCubic(progress);
    
    // Position interpolation: move from start to target, retaining some randomness
    x = lerp(startX, targetX, easeProgress);
    y = lerp(startY, targetY, easeProgress);
    
    // Add floating effect: sinusoidal wave offset during movement
    if (progress < 0.8) {
      float floatOffset = sin(progress * PI * 4 + targetIndex) * 15 * (1 - progress);
      x += floatOffset * cos(targetIndex);
      y += floatOffset * sin(targetIndex);
    }
    
    // Rotation gradually returns to zero
    rotation = lerp(rotation, 0, easeProgress);
  }
  
  void display() {
    pushMatrix();
    translate(x, y);
    rotate(rotation);
    fill(255, 220, 100 + targetIndex * 5 % 155);
    text(letter, 0, 0);
    popMatrix();
  }
  
  float easeInOutCubic(float t) {
    return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
  }
}
