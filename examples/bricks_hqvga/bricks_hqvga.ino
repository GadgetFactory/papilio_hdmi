/*
  HQVGA Bricks Demo
 
  Classic brick breaker game demonstrating the HQVGA 160x120 pixel 
  framebuffer display with pixel graphics and collision detection.
  
  Original ZPUino VGA version by Alvaro Lopes
  HQVGA port by Jack Gassett, Gadget Factory
  
  Hardware:
  - Papilio Arcade board with ESP32-S3 and FPGA
  - HDMI output (160x120 scaled to 720p)
*/

#include <SPI.h>
#include <HQVGA.h>

// SPI Pin Configuration for ESP32-S3
#define SPI_CLK   12
#define SPI_MOSI  11
#define SPI_MISO  9
#define SPI_CS    10

// Screen dimensions
#define HSIZE 160
#define VSIZE 120

// Block grid: 8 columns x 4 rows
#define BLOCK_COLS 8
#define BLOCK_ROWS 4
#define BLOCK_WIDTH 19
#define BLOCK_HEIGHT 3
#define BLOCK_GAP 1

// Ball and paddle
#define BALL_SIZE 4
#define PADDLE_WIDTH 24
#define PADDLE_HEIGHT 3
#define PADDLE_Y (VSIZE - 10)

// Game state
uint8_t blocks[BLOCK_COLS][BLOCK_ROWS];
int ballX, ballY;
int ballDX, ballDY;
int paddleX;
int score = 0;
int lives = 3;
bool gameOver = false;

// Saved area behind ball for restore
uint8_t savedBall[BALL_SIZE * BALL_SIZE];
int savedBallX = -1, savedBallY = -1;

// Block colors (RGB332)
const uint8_t blockColors[4] = {
  0xE0,  // Red
  0xFC,  // Yellow
  0x1C,  // Green
  0x03   // Blue
};

SPIClass *fpgaSPI = NULL;

void setup() {
  Serial.begin(115200);
  
  Serial.println("HQVGA Bricks Demo");
  Serial.println("Resolution: 160x120 pixels");
  
  // Initialize SPI
  fpgaSPI = new SPIClass(HSPI);
  fpgaSPI->begin(SPI_CLK, SPI_MISO, SPI_MOSI, SPI_CS);
  
  // Initialize VGA - this waits for FPGA bootloader and sets framebuffer mode
  VGA.begin(fpgaSPI, SPI_CS, SPI_CLK, SPI_MOSI, SPI_MISO);
  
  // Show title screen
  VGA.setBackgroundColor(BLACK);
  VGA.clear();
  
  VGA.setColor(RED);
  VGA.printtext(50, 40, "BRICKS", false);
  VGA.setColor(WHITE);
  VGA.printtext(35, 55, "HQVGA DEMO", false);
  VGA.setColor(CYAN);
  VGA.printtext(20, 80, "Press any key", false);
  
  // Wait for keypress or timeout
  unsigned long start = millis();
  while (!Serial.available() && (millis() - start < 5000)) {
    delay(100);
  }
  if (Serial.available()) Serial.read();
  
  initGame();
}

void initGame() {
  // Clear screen
  VGA.clear();
  
  // Initialize blocks
  for (int row = 0; row < BLOCK_ROWS; row++) {
    for (int col = 0; col < BLOCK_COLS; col++) {
      blocks[col][row] = blockColors[row];
    }
  }
  
  // Draw blocks
  drawBlocks();
  
  // Initialize ball
  ballX = HSIZE / 2;
  ballY = VSIZE / 2;
  ballDX = 1;
  ballDY = -1;
  
  // Initialize paddle
  paddleX = (HSIZE - PADDLE_WIDTH) / 2;
  
  // Draw paddle
  drawPaddle();
  
  // Draw score and lives
  drawStatus();
  
  gameOver = false;
  savedBallX = -1;
  savedBallY = -1;
}

void drawBlocks() {
  int startY = 5;
  
  for (int row = 0; row < BLOCK_ROWS; row++) {
    int y = startY + row * (BLOCK_HEIGHT + BLOCK_GAP);
    
    for (int col = 0; col < BLOCK_COLS; col++) {
      int x = col * (BLOCK_WIDTH + BLOCK_GAP);
      
      if (blocks[col][row] != 0) {
        VGA.setColor(blocks[col][row]);
        VGA.drawRect(x, y, BLOCK_WIDTH, BLOCK_HEIGHT);
      }
    }
  }
}

void clearBlock(int col, int row) {
  int startY = 5;
  int x = col * (BLOCK_WIDTH + BLOCK_GAP);
  int y = startY + row * (BLOCK_HEIGHT + BLOCK_GAP);
  
  VGA.setColor(BLACK);
  VGA.drawRect(x, y, BLOCK_WIDTH, BLOCK_HEIGHT);
}

void drawPaddle() {
  // Clear old paddle area (full width)
  VGA.setColor(BLACK);
  VGA.drawRect(0, PADDLE_Y, HSIZE, PADDLE_HEIGHT);
  
  // Draw new paddle
  VGA.setColor(WHITE);
  VGA.drawRect(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT);
}

void drawStatus() {
  char buf[20];
  
  // Clear status area at bottom
  VGA.setColor(BLACK);
  VGA.drawRect(0, VSIZE - 6, HSIZE, 6);
  
  // Draw score
  VGA.setColor(WHITE);
  snprintf(buf, sizeof(buf), "S:%d", score);
  VGA.printtext(2, VSIZE - 6, buf, true);
  
  // Draw lives
  snprintf(buf, sizeof(buf), "L:%d", lives);
  VGA.printtext(HSIZE - 24, VSIZE - 6, buf, true);
}

void saveBallArea(int x, int y) {
  int idx = 0;
  for (int dy = 0; dy < BALL_SIZE; dy++) {
    for (int dx = 0; dx < BALL_SIZE; dx++) {
      savedBall[idx++] = VGA.getPixel(x + dx, y + dy);
    }
  }
  savedBallX = x;
  savedBallY = y;
}

void restoreBallArea() {
  if (savedBallX < 0) return;
  
  int idx = 0;
  for (int dy = 0; dy < BALL_SIZE; dy++) {
    for (int dx = 0; dx < BALL_SIZE; dx++) {
      VGA.putPixel(savedBallX + dx, savedBallY + dy, savedBall[idx++]);
    }
  }
}

void drawBall(int x, int y) {
  // Draw a small filled ball
  VGA.setColor(YELLOW);
  VGA.drawRect(x, y, BALL_SIZE, BALL_SIZE);
}

bool checkBlockCollision(int x, int y) {
  int startY = 5;
  int blockAreaHeight = BLOCK_ROWS * (BLOCK_HEIGHT + BLOCK_GAP);
  
  // Only check if ball is in block area
  if (y > startY + blockAreaHeight) return false;
  
  // Check each corner of the ball
  for (int corner = 0; corner < 4; corner++) {
    int checkX = x + (corner & 1) * (BALL_SIZE - 1);
    int checkY = y + (corner >> 1) * (BALL_SIZE - 1);
    
    int col = checkX / (BLOCK_WIDTH + BLOCK_GAP);
    int row = (checkY - startY) / (BLOCK_HEIGHT + BLOCK_GAP);
    
    if (col >= 0 && col < BLOCK_COLS && row >= 0 && row < BLOCK_ROWS) {
      if (blocks[col][row] != 0) {
        // Hit a block!
        blocks[col][row] = 0;
        clearBlock(col, row);
        score += 10;
        drawStatus();
        return true;
      }
    }
  }
  return false;
}

bool checkPaddleCollision(int x, int y) {
  // Check if ball hits paddle
  if (y + BALL_SIZE >= PADDLE_Y && y + BALL_SIZE <= PADDLE_Y + PADDLE_HEIGHT) {
    if (x + BALL_SIZE >= paddleX && x <= paddleX + PADDLE_WIDTH) {
      // Adjust angle based on where ball hits paddle
      int hitPos = (x + BALL_SIZE/2) - (paddleX + PADDLE_WIDTH/2);
      if (hitPos < -PADDLE_WIDTH/3) {
        ballDX = -2;
      } else if (hitPos > PADDLE_WIDTH/3) {
        ballDX = 2;
      } else {
        ballDX = (ballDX > 0) ? 1 : -1;
      }
      return true;
    }
  }
  return false;
}

void updateBall() {
  // Calculate new position
  int newX = ballX + ballDX;
  int newY = ballY + ballDY;
  
  // Wall collisions
  if (newX <= 0) {
    newX = 0;
    ballDX = -ballDX;
  }
  if (newX >= HSIZE - BALL_SIZE) {
    newX = HSIZE - BALL_SIZE;
    ballDX = -ballDX;
  }
  if (newY <= 0) {
    newY = 0;
    ballDY = -ballDY;
  }
  
  // Bottom - lose life
  if (newY >= VSIZE - BALL_SIZE) {
    lives--;
    drawStatus();
    
    if (lives <= 0) {
      gameOver = true;
      return;
    }
    
    // Reset ball
    delay(500);
    restoreBallArea();
    ballX = HSIZE / 2;
    ballY = VSIZE / 2;
    ballDX = 1;
    ballDY = -1;
    savedBallX = -1;
    return;
  }
  
  // Block collision
  if (checkBlockCollision(newX, newY)) {
    ballDY = -ballDY;
    newY = ballY + ballDY;
  }
  
  // Paddle collision
  if (checkPaddleCollision(newX, newY)) {
    ballDY = -abs(ballDY);  // Always bounce up
    newY = PADDLE_Y - BALL_SIZE;
  }
  
  // Restore old ball area, save new area, draw ball
  restoreBallArea();
  saveBallArea(newX, newY);
  drawBall(newX, newY);
  
  ballX = newX;
  ballY = newY;
}

void updatePaddle() {
  // Simple AI - follow the ball
  int paddleCenter = paddleX + PADDLE_WIDTH / 2;
  int ballCenter = ballX + BALL_SIZE / 2;
  
  if (paddleCenter < ballCenter - 2) {
    paddleX += 2;
    if (paddleX > HSIZE - PADDLE_WIDTH) paddleX = HSIZE - PADDLE_WIDTH;
  } else if (paddleCenter > ballCenter + 2) {
    paddleX -= 2;
    if (paddleX < 0) paddleX = 0;
  }
  
  drawPaddle();
}

void checkWin() {
  // Check if all blocks destroyed
  for (int row = 0; row < BLOCK_ROWS; row++) {
    for (int col = 0; col < BLOCK_COLS; col++) {
      if (blocks[col][row] != 0) return;
    }
  }
  
  // Win!
  VGA.setColor(GREEN);
  VGA.printtext(45, 55, "YOU WIN!", false);
  delay(3000);
  
  score += 100;
  initGame();
}

void showGameOver() {
  VGA.setColor(RED);
  VGA.printtext(40, 50, "GAME OVER", false);
  
  char buf[20];
  snprintf(buf, sizeof(buf), "Score: %d", score);
  VGA.setColor(WHITE);
  VGA.printtext(40, 65, buf, false);
  
  delay(3000);
  
  score = 0;
  lives = 3;
  initGame();
}

// Scrolling text
const char *scrollText = "    Welcome to HQVGA Bricks! A ZPUino classic ported to ESP32-S3...    ";
int scrollOffset = 0;
int scrollCounter = 0;

void drawScrollingText() {
  scrollCounter++;
  if (scrollCounter < 8) return;
  scrollCounter = 0;
  
  // Draw scrolling text at bottom of block area
  int textY = 22;
  const char *text = &scrollText[scrollOffset >> 3];
  
  if (!*text) {
    scrollOffset = 0;
    text = scrollText;
  }
  
  int x = -(scrollOffset & 0x7);
  VGA.setColor(CYAN);
  
  // Clear text line
  VGA.setColor(BLACK);
  VGA.drawRect(0, textY, HSIZE, 8);
  
  VGA.setColor(CYAN);
  for (int i = 0; i <= HSIZE/8 + 1; i++) {
    if (*text) {
      VGA.printchar(x, textY, *text, true);
      text++;
    }
    x += 8;
  }
  
  scrollOffset++;
}

void loop() {
  if (gameOver) {
    showGameOver();
    return;
  }
  
  updateBall();
  updatePaddle();
  drawScrollingText();
  checkWin();
  
  delay(15);  // ~60fps
}
