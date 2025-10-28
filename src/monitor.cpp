#include "monitor.h"
#include "meme_bitmap.h"
#include <Wire.h>
#include <U8g2lib.h>
#include <FluxGarage_RoboEyes.h>

// Global display objects
static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// U8g2 for UTF-8 support (same I2C pins, address 0x3C)
static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// RoboEyes instance (template class for Adafruit_SSD1306) - pass by reference
static RoboEyes<Adafruit_SSD1306> roboEyes(display);

// Banner messages for idle screen - motivational quotes
static const char* bannerMessages[] = {
  "The secret of getting ahead is getting started - Mark Twain",
  "Don't watch the clock; do what it does. Keep going - Sam Levenson",
  "The way to get started is to quit talking and begin doing - Walt Disney",
  "It always seems impossible until it's done - Nelson Mandela",
  "Success is not final, failure is not fatal - Winston Churchill",
  "Believe you can and you're halfway there - Theodore Roosevelt",
  "Action is the foundational key to all success - Pablo Picasso",
  "Small progress is still progress",
  "Focus on being productive instead of busy",
  "Great things never come from comfort zones"
};
static const int bannerMessageCount = sizeof(bannerMessages) / sizeof(bannerMessages[0]);
static int currentBannerIndex = 0;
static unsigned long lastBannerRotation = 0;
static const unsigned long BANNER_ROTATION_INTERVAL = 5000; // Rotate every 5 seconds

// Continuous scrolling transition variables
static int scrollOffset = 0;
static unsigned long lastScrollUpdate = 0;
static const unsigned long SCROLL_DELAY = 50; // Milliseconds between scroll updates
static const int SCROLL_SPEED = 2; // Pixels to scroll per update
static bool scrollInitialized = false;
static unsigned long scrollPauseStart = 0;
static const unsigned long SCROLL_PAUSE_DURATION = 2000; // Pause to read the message (2 seconds)
static int currentBannerDisplayIndex = 0; // Which message is currently being displayed

// Initialize the monitor
bool monitor_init(int sda_pin, int scl_pin) {
  // Initialize I2C
  Wire.begin(sda_pin, scl_pin);

  // Initialize Adafruit display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return false;
  }

  // Disable text wrapping to prevent line breaks
  display.setTextWrap(false);

  // Initialize U8g2 (for UTF-8 emoticons)
  u8g2.begin();
  u8g2.enableUTF8Print();

  Serial.println("Monitor initialized successfully");
  return true;
}

// Draw continuous scrolling banner text with transitions
void monitor_draw_banner(int y) {
  unsigned long currentTime = millis();
  display.setTextSize(1);

  // Initialize on first call
  if (!scrollInitialized) {
    scrollInitialized = true;
    scrollOffset = 0;
    scrollPauseStart = currentTime;
    lastScrollUpdate = currentTime;
    currentBannerDisplayIndex = currentBannerIndex;
  }

  const char* currentMessage = bannerMessages[currentBannerDisplayIndex];
  int nextIndex = (currentBannerDisplayIndex + 1) % bannerMessageCount;
  const char* nextMessage = bannerMessages[nextIndex];

  // Get text bounds for current message
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(currentMessage, 0, 0, &x1, &y1, &w, &h);
  int currentMessageWidth = w;

  // Get text bounds for next message
  display.getTextBounds(nextMessage, 0, 0, &x1, &y1, &w, &h);
  int nextMessageWidth = w;

  // Handle pause at the beginning
  if (scrollPauseStart > 0) {
    if (currentTime - scrollPauseStart >= SCROLL_PAUSE_DURATION) {
      // Pause is over, start scrolling
      scrollPauseStart = 0;
      lastScrollUpdate = currentTime;
    } else {
      // Still in pause, display message statically
      if (currentMessageWidth <= SCREEN_WIDTH) {
        // Center short messages
        int centerX = (SCREEN_WIDTH - currentMessageWidth) / 2;
        display.setCursor(centerX, y);
      } else {
        // Long messages start from left
        display.setCursor(0, y);
      }
      display.print(currentMessage);
      return;
    }
  }

  // Update scroll offset
  if (currentTime - lastScrollUpdate >= SCROLL_DELAY) {
    scrollOffset += SCROLL_SPEED;
    lastScrollUpdate = currentTime;
  }

  // Calculate total transition width (current message scrolls out, next slides in)
  int totalTransitionWidth = currentMessageWidth + SCREEN_WIDTH + nextMessageWidth;

  // Check if we've completed the full transition
  if (scrollOffset >= totalTransitionWidth) {
    // Move to next message
    currentBannerDisplayIndex = nextIndex;
    scrollOffset = 0;
    scrollPauseStart = currentTime;
    lastScrollUpdate = currentTime;
    return;
  }

  // Draw the scrolling transition
  // Current message scrolls from left to off-screen left
  int currentX = -scrollOffset;

  // Next message appears from right when current message width + spacing has scrolled
  int nextX = currentMessageWidth + SCREEN_WIDTH - scrollOffset;

  // Draw current message if still visible
  if (currentX + currentMessageWidth > 0) {
    display.setCursor(currentX, y);
    display.print(currentMessage);
  }

  // Draw next message if it has started appearing
  if (nextX < SCREEN_WIDTH) {
    display.setCursor(nextX, y);
    display.print(nextMessage);
  }
}

// Show boot screen
void monitor_show_boot_screen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Pomodoro");
  display.println("Timer");
  display.display();
}

// Show idle screen
void monitor_show_idle_screen(IdleMode selectedMode, int completedCount) {
  display.clearDisplay();
  display.setTextSize(1);

  // Top section - Mode selection
  // WORK text
  display.setCursor(10, 2);
  if (selectedMode == MODE_WORK) {
    // Inverted for selected
    display.fillRect(8, 0, 40, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.println("WORK");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.println("WORK");
  }

  // BREAK text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(70, 2);
  if (selectedMode == MODE_BREAK) {
    // Inverted for selected
    display.fillRect(68, 0, 50, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.println("BREAK");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.println("BREAK");
  }

  display.setTextColor(SSD1306_WHITE);
  display.drawLine(0, 14, 127, 14, SSD1306_WHITE);

  // Timer section - show 00:00
  display.setTextSize(3);
  display.setCursor(15, 22);
  display.println("00:00");

  display.drawLine(0, 48, 127, 48, SSD1306_WHITE);

  // Bottom section - Static banner message with quotes
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  // Center text vertically in banner area (48-64 = 16px height, text is 8px)
  // y = 48 + (16-8)/2 = 48 + 4 = 52, but adjust to 54 for better visual centering
  monitor_draw_banner(54);

  display.display();
}

// Show running screen
void monitor_show_running_screen(PomodoroState state, unsigned long timeRemaining, int completedCount) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Top section - Mode display
  // WORK text
  display.setCursor(10, 2);
  if (state == POMODORO_WORK) {
    // Inverted for active
    display.fillRect(8, 0, 40, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.println("WORK");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.println("WORK");
  }

  // BREAK text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(70, 2);
  if (state == POMODORO_SHORT_BREAK || state == POMODORO_LONG_BREAK) {
    // Inverted for active
    display.fillRect(68, 0, 50, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.println("BREAK");
  } else {
    display.setTextColor(SSD1306_WHITE);
    display.println("BREAK");
  }

  display.setTextColor(SSD1306_WHITE);
  display.drawLine(0, 14, 127, 14, SSD1306_WHITE);

  // Timer section
  display.setTextSize(3);
  display.setCursor(15, 22);
  String timeStr = monitor_format_time(timeRemaining);
  display.println(timeStr);

  // Show paused indicator if paused
  if (state == POMODORO_PAUSED) {
    display.setTextSize(1);
    display.setCursor(95, 30);
    display.println("||");
  }

  display.drawLine(0, 48, 127, 48, SSD1306_WHITE);

  // Bottom section - Completed rounds
  display.setTextSize(1);
  display.setCursor(0, 52);
  display.print("Completed: ");
  display.setTextSize(1);
  display.setCursor(85, 52);
  display.print(completedCount);

  display.display();
}

// Show finished screen
void monitor_show_finished_screen(int completedCount) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(10, 10);
  display.println("FINISHED!");

  display.setTextSize(1);
  display.setCursor(0, 35);
  display.println("Press BTN1 to continue");

  display.setCursor(0, 50);
  display.print("Completed: ");
  display.print(completedCount);

  display.display();
}

// Format time as MM:SS
String monitor_format_time(unsigned long seconds) {
  unsigned long minutes = seconds / 60;
  unsigned long secs = seconds % 60;

  String timeStr = "";
  if (minutes < 10) timeStr += "0";
  timeStr += String(minutes);
  timeStr += ":";
  if (secs < 10) timeStr += "0";
  timeStr += String(secs);

  return timeStr;
}

// Show meme image
void monitor_show_meme() {
  display.clearDisplay();
  display.drawBitmap(0, 0, meme_bitmap, MEME_WIDTH, MEME_HEIGHT, SSD1306_WHITE);
  display.display();
}

// Initialize RoboEyes
void monitor_roboeyes_init() {
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 20);  // 128x64, 20fps
}

// Show happy expression
void monitor_roboeyes_show_happy() {
  roboEyes.setMood(HAPPY);
}

// Show sad expression
void monitor_roboeyes_show_sad() {
  roboEyes.setMood(TIRED);  // Use TIRED for sad/sleepy look
}

// Animate eyes for a duration
void monitor_roboeyes_animate(int duration_ms) {
  unsigned long startTime = millis();

  while (millis() - startTime < duration_ms) {
    display.clearDisplay();
    roboEyes.update();  // This calls drawEyes() internally
    display.display();
    delay(50);  // ~20fps
  }
}

// Show init expression - eyes look left and right
void monitor_roboeyes_show_init() {
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(true);  // Makes outer eye larger when looking sideways
  
  // Animate looking left and right
  unsigned long startTime = millis();
  int lookDuration = 1000;  // Look each direction for 1 second
  
  // Look left
  roboEyes.setPosition(W);  // West - look left
  while (millis() - startTime < lookDuration) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }
  
  // Look center briefly
  startTime = millis();
  roboEyes.setPosition(DEFAULT);  // Center
  while (millis() - startTime < 500) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }
  
  // Look right
  startTime = millis();
  roboEyes.setPosition(E);  // East - look right
  while (millis() - startTime < lookDuration) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }
  
  // Return to center
  roboEyes.setPosition(DEFAULT);
  roboEyes.setCuriosity(false);

  // Trigger blink
  roboEyes.blink();

  // Quick blink animation (shorter duration to transition faster)
  startTime = millis();
  while (millis() - startTime < 50) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }

  // Transition to happy mood immediately after blink
  roboEyes.setMood(HAPPY);

  // Typing animation for "Happy Learning!" text
  const char* message = "Happy Learning!";
  int messageLen = strlen(message);
  int textY = 52; // Position slightly higher from bottom

  display.setTextSize(1);

  // Calculate center position for the full text
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
  int centerX = (SCREEN_WIDTH - w) / 2;

  // Typing effect - reveal one character at a time
  int charDelay = 80; // Delay between each character (ms)

  for (int i = 0; i <= messageLen; i++) {
    display.clearDisplay();
    roboEyes.update();

    // Draw the text up to current character
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(centerX, textY);

    for (int j = 0; j < i; j++) {
      display.print(message[j]);
    }

    display.display();
    delay(charDelay);
  }
  delay(1000); // Hold the final message for a moment
}

// Show lost expression - searching eyes then sad with "Where are you?"
void monitor_roboeyes_show_lost() {
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(true);  // Makes outer eye larger when looking

  // Searching animation - look around in different directions
  unsigned long startTime = millis();
  int lookDuration = 600;  // Look each direction for 600ms

  // Look left
  roboEyes.setPosition(W);  // West - look left
  while (millis() - startTime < lookDuration) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }

  // Look right
  startTime = millis();
  roboEyes.setPosition(E);  // East - look right
  while (millis() - startTime < lookDuration) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }

  // Look up
  startTime = millis();
  roboEyes.setPosition(N);  // North - look up
  while (millis() - startTime < lookDuration) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }

  // Look down
  startTime = millis();
  roboEyes.setPosition(S);  // South - look down
  while (millis() - startTime < lookDuration) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }

  // Return to center and turn off curiosity
  roboEyes.setPosition(DEFAULT);
  roboEyes.setCuriosity(false);

  // Switch to sad mood
  roboEyes.setMood(TIRED);  // TIRED for sad look

  // Display sad eyes with message
  const char* message = "Where are you?";
  int messageLen = strlen(message);
  int textY = 52;

  display.setTextSize(1);

  // Calculate center position for the text
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
  int centerX = (SCREEN_WIDTH - w) / 2;

  // Typing effect - reveal one character at a time
  int charDelay = 80; // Delay between each character (ms)

  for (int i = 0; i <= messageLen; i++) {
    display.clearDisplay();
    roboEyes.update();

    // Draw the text up to current character
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(centerX, textY);

    for (int j = 0; j < i; j++) {
      display.print(message[j]);
    }

    display.display();
    delay(charDelay);
  }

  delay(2000); // Hold the final message for 2 seconds
}

// Show return expression - sad, blink, then happy with "You are Back!"
void monitor_roboeyes_show_return() {
  // Start with sad mood
  roboEyes.setMood(TIRED);  // TIRED for sad look
  roboEyes.setPosition(DEFAULT);  // Center position

  // Show sad eyes for a moment
  unsigned long startTime = millis();
  while (millis() - startTime < 1000) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(100);
  }

  // Trigger blink
  roboEyes.blink();

  // Blink animation
  startTime = millis();
  while (millis() - startTime < 300) {
    display.clearDisplay();
    roboEyes.update();
    display.display();
    delay(50);
  }

  // Transition to happy mood
  roboEyes.setMood(HAPPY);

  // Display happy eyes with message
  const char* message = "You are Back!";
  int messageLen = strlen(message);
  int textY = 52;

  display.setTextSize(1);

  // Calculate center position for the text
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
  int centerX = (SCREEN_WIDTH - w) / 2;

  // Typing effect - reveal one character at a time
  int charDelay = 80; // Delay between each character (ms)

  for (int i = 0; i <= messageLen; i++) {
    display.clearDisplay();
    roboEyes.update();

    // Draw the text up to current character
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(centerX, textY);

    for (int j = 0; j < i; j++) {
      display.print(message[j]);
    }

    display.display();
    delay(charDelay);
  }

  delay(2000); // Hold the final message for 2 seconds
}

