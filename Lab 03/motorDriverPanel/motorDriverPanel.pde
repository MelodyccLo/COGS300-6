import hypermedia.net.*;

UDP udp;  // The UDP object
String ip = "172.20.10.2";  // YOUR ARDUINO IP
int port = 8888;            // The port we set in Arduino

// UI Variables
int speed = 127;
int sliderX = 320, sliderY = 50, sliderW = 40, sliderH = 300;
char lastCmd = 's';
boolean isMoving = false;

void setup() {
  size(450, 400);
  udp = new UDP(this, 6000); // Local port
  textAlign(CENTER, CENTER);
}

void draw() {
  background(30);
  
  // --- DRAW SLIDER (Speed Control) ---
  fill(50);
  rect(sliderX, sliderY, sliderW, sliderH, 10); // Background of slider
  float handleY = map(speed, 0, 255, sliderY + sliderH, sliderY);
  fill(0, 200, 255);
  rect(sliderX - 5, handleY - 10, sliderW + 10, 20, 5); // Slider handle
  
  fill(255);
  text("SPEED: " + speed, sliderX + 20, sliderY - 20);

  // --- DRAW D-PAD (Visual Feedback) ---
  drawButton(100, 150, "W", lastCmd == 'f');
  drawButton(100, 250, "S", lastCmd == 'b');
  drawButton(30, 250, "A", lastCmd == 'l');
  drawButton(170, 250, "D", lastCmd == 'r');
  
  // --- SLIDER LOGIC ---
  if (mousePressed && mouseX > sliderX && mouseX < sliderX + sliderW) {
    speed = int(constrain(map(mouseY, sliderY + sliderH, sliderY, 0, 255), 0, 255));
  }
}

void drawButton(int x, int y, String label, boolean active) {
  fill(active ? color(0, 255, 100) : 60);
  rect(x, y, 60, 60, 10);
  fill(255);
  textSize(20);
  text(label, x + 30, y + 30);
}

// --- KEYBOARD LOGIC ---
void keyPressed() {
  char newCmd = ' ';
  if (keyCode == UP || key == 'w') newCmd = 'f';
  else if (keyCode == DOWN || key == 's') newCmd = 'b';
  else if (keyCode == LEFT || key == 'a') newCmd = 'l';
  else if (keyCode == RIGHT || key == 'd') newCmd = 'r';

  if (newCmd != ' ' && newCmd != lastCmd) {
    sendCommand(newCmd, speed);
    lastCmd = newCmd;
  }
}

void keyReleased() {
  sendCommand('s', 0);
  lastCmd = 's';
}

// --- MOUSE LOGIC (For clicking buttons) ---
void mousePressed() {
  if (dist(mouseX, mouseY, 130, 180) < 40) sendCommand('f', speed); // Simple hit detection
}

void mouseReleased() {
  if (mouseX < 250) { // If clicking in the D-pad area
    sendCommand('s', 0);
    lastCmd = 's';
  }
}

void sendCommand(char c, int s) {
  String message = c + "," + s;
  udp.send(message, ip, port);
  println("Sent: " + message);
}
