/*
  HQVGA Space Invaders Demo
 
  Demonstrates the HQVGA 160x120 pixel framebuffer display with a
  simple space invaders animation using pixel graphics.
  
  Original concept from JO3RI's LCD demo
  HQVGA graphics version by Jack Gassett, Gadget Factory
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - VGA output (160x120 scaled to 800x600@72Hz)
*/

#include <SPI.h>
#include <HQVGA.h>

// SPI Pin Configuration for ESP32-S3
#define SPI_CLK   12
#define SPI_MOSI  11
#define SPI_MISO  9
#define SPI_CS    10

// Space invader sprite data (8x8 pixels each)
// Frame 1 - legs out
const uint8_t invader1[8] = {
  0b00100100,  //   .  .
  0b00011000,  //    ..
  0b00111100,  //   ....
  0b01101110,  //  .. ...
  0b11111111,  // ........
  0b10111101,  // . .... .
  0b10100101,  // . .  . .
  0b00011000   //    ..
};

// Frame 2 - legs in  
const uint8_t invader2[8] = {
  0b00100100,  //   .  .
  0b00011000,  //    ..
  0b00111100,  //   ....
  0b01101110,  //  .. ...
  0b11111111,  // ........
  0b00111100,  //   ....
  0b01000010,  //  .    .
  0b10000001   // .      .
};

// Player ship sprite (8x5)
const uint8_t player[5] = {
  0b00011000,  //    ..
  0b00111100,  //   ....
  0b01111110,  //  ......
  0b11111111,  // ........
  0b11111111   // ........
};

// Bullet
const uint8_t bulletHeight = 4;

// Game state
int invaderX = 10;
int invaderY = 20;
int invaderDir = 1;  // 1 = right, -1 = left
int invaderFrame = 0;
int playerX = 76;    // Center of 160 pixel width
bool bulletActive = false;
int bulletX, bulletY;
int score = 0;
unsigned long lastFrameTime = 0;
const int frameDelay = 100;  // ms between frames

SPIClass *fpgaSPI = NULL;

void setup() {
  Serial.begin(115200);
  
  Serial.println("HQVGA Space Invaders Demo");
  Serial.println("Resolution: 160x120 pixels");
  
  // Initialize SPI
  fpgaSPI = new SPIClass(HSPI);
  fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  
  // Initialize VGA
  VGA.begin(fpgaSPI, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  
  // Clear screen to black
  VGA.setBackgroundColor(BLACK);
  VGA.clear();
  
  // Draw title
  VGA.setColor(GREEN);
  VGA.printtext(30, 5, "SPACE INVADERS", false);
  VGA.setColor(WHITE);
  VGA.printtext(45, 15, "HQVGA DEMO", false);
  
  delay(2000);
  
  // Clear and start game
  VGA.clear();
  
  Serial.println("Game started!");
  Serial.println("Controls: Press any key to fire");
}

void drawSprite(int x, int y, const uint8_t* sprite, int height, VGA_class::pixel_t color) {
  for (int row = 0; row < height; row++) {
    uint8_t rowData = sprite[row];
    for (int col = 0; col < 8; col++) {
      if (rowData & (0x80 >> col)) {
        VGA.putPixel(x + col, y + row, color);
      }
    }
  }
}

void clearSprite(int x, int y, int width, int height) {
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      VGA.putPixel(x + col, y + row, BLACK);
    }
  }
}

void drawInvader(int x, int y, int frame) {
  const uint8_t* sprite = (frame == 0) ? invader1 : invader2;
  drawSprite(x, y, sprite, 8, GREEN);
}

void drawPlayer(int x, int y) {
  drawSprite(x, y, player, 5, CYAN);
}

void drawBullet(int x, int y) {
  for (int i = 0; i < bulletHeight; i++) {
    VGA.putPixel(x, y + i, YELLOW);
  }
}

void clearBullet(int x, int y) {
  for (int i = 0; i < bulletHeight; i++) {
    VGA.putPixel(x, y + i, BLACK);
  }
}

void drawScore() {
  // Draw score at top of screen
  char scoreStr[16];
  snprintf(scoreStr, sizeof(scoreStr), "SCORE:%d", score);
  VGA.setColor(WHITE);
  VGA.printtext(2, 2, scoreStr, false);
}

void drawGround() {
  // Draw ground line
  VGA.setColor(GREEN);
  for (int x = 0; x < 160; x++) {
    VGA.putPixel(x, 115, GREEN);
  }
}

bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
  return !(x1 + w1 < x2 || x2 + w2 < x1 || y1 + h1 < y2 || y2 + h2 < y1);
}

void fireBullet() {
  if (!bulletActive) {
    bulletActive = true;
    bulletX = playerX + 3;  // Center of player
    bulletY = 105;          // Just above player
    Serial.println("Fire!");
  }
}

void gameOver() {
  VGA.setColor(RED);
  VGA.printtext(50, 50, "GAME OVER!", false);
  VGA.setColor(WHITE);
  char finalScore[20];
  snprintf(finalScore, sizeof(finalScore), "SCORE: %d", score);
  VGA.printtext(45, 65, finalScore, false);
  
  Serial.println("Game Over!");
  Serial.printf("Final Score: %d\n", score);
  
  delay(3000);
  
  // Reset game
  score = 0;
  invaderX = 10;
  invaderY = 20;
  invaderDir = 1;
  bulletActive = false;
  VGA.clear();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Frame rate limiting
  if (currentTime - lastFrameTime < frameDelay) {
    return;
  }
  lastFrameTime = currentTime;
  
  // Check for fire command
  if (Serial.available()) {
    Serial.read();
    fireBullet();
  }
  
  // Clear old positions
  clearSprite(invaderX, invaderY, 8, 8);
  clearSprite(playerX, 110, 8, 5);
  if (bulletActive) {
    clearBullet(bulletX, bulletY);
  }
  
  // Update invader position
  invaderX += invaderDir * 2;
  
  // Bounce off walls
  if (invaderX >= 150) {
    invaderDir = -1;
    invaderY += 4;  // Move down
  } else if (invaderX <= 2) {
    invaderDir = 1;
    invaderY += 4;  // Move down
  }
  
  // Toggle animation frame
  invaderFrame = 1 - invaderFrame;
  
  // Check if invader reached the bottom
  if (invaderY >= 100) {
    gameOver();
    return;
  }
  
  // Update bullet
  if (bulletActive) {
    bulletY -= 4;  // Move up
    
    // Check if bullet went off screen
    if (bulletY < 10) {
      bulletActive = false;
    }
    
    // Check collision with invader
    if (checkCollision(bulletX, bulletY, 1, bulletHeight, invaderX, invaderY, 8, 8)) {
      // Hit!
      Serial.println("HIT!");
      score += 100;
      bulletActive = false;
      
      // Reset invader position
      invaderX = 10 + (random(140));
      invaderY = 20;
      
      // Flash effect
      VGA.setColor(WHITE);
      VGA.drawRect(invaderX - 2, invaderY - 2, 12, 12);
      delay(50);
      clearSprite(invaderX - 2, invaderY - 2, 12, 12);
    }
  }
  
  // Simple player AI - follow invader
  if (playerX + 4 < invaderX + 4) {
    playerX += 2;
    if (playerX > 150) playerX = 150;
  } else if (playerX + 4 > invaderX + 4) {
    playerX -= 2;
    if (playerX < 2) playerX = 2;
  }
  
  // Auto-fire occasionally
  if (!bulletActive && random(10) < 2) {
    fireBullet();
  }
  
  // Draw everything
  drawGround();
  drawScore();
  drawInvader(invaderX, invaderY, invaderFrame);
  drawPlayer(playerX, 110);
  if (bulletActive) {
    drawBullet(bulletX, bulletY);
  }
}
