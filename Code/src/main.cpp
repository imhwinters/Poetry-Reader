#include "esp32-hal-gpio.h"
#include "hal/gpio_types.h"
#include "Arduino.h"
#include "Wire.h"
#include "U8g2lib.h"
#include "LittleFS.h"
#include "pins_arduino.h"
#include <Preferences.h>
#include <cctype>
#include <vector>

#define BTN_POWER D2
#define BTN_SELECT D8
#define BTN_RIGHT D7
#define BTN_DOWN D6
#define BTN_LEFT D0
#define BTN_UP D3

U8G2_SH1107_SEEED_128X128_F_HW_I2C display(U8G2_R2);

Preferences prefs;

struct Author {
    String name;
};

std::vector<Author> authors;
std::vector<int> filteredAuthors;

struct Poem {
    String title;
    String author;
    String file;
};

std::vector<Poem> poems;
std::vector<int> filtered;

enum Screen {
    HOME,
    AUTHORS,
    LIST,
    READER
};

Screen currentScreen = HOME;

int homeIndex = 0;
int listIndex = 0;
int authorIndex = 0;
int currentAuthor = -1;
int readerScroll = 0;
int readerButton = 0;
int poemsPerList = 8;

String poemText = "";
int currentPoem = -1;

const char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

bool pressed(int pin) {
    static uint32_t lastTime[40] = {0};
    uint32_t now = millis();

    if (digitalRead(pin) == LOW) {
        if (now - lastTime[pin] > 180) {
            lastTime[pin] = now;
            return true;
        }
    }

    return false;
}

void loadPoems() {
    File f = LittleFS.open("/index.csv", "r");
    if (!f) return;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.startsWith("title")) continue;

        int a = line.indexOf('~');
        int b = line.indexOf('~', a+1);

        Poem p;
        p.title = line.substring(0, a);
        p.author = line.substring(a + 1, b);
        p.file = line.substring(b + 1);

        poems.push_back(p);
    }

    Serial.print("Poems loaded: ");
    Serial.println(poems.size());

    for (int i = 0; i < poems.size(); i++) {
        bool exists = false;

        for (int j = 0; j < authors.size(); j++) {
            if (authors[j].name == poems[i].author) {
                exists = true;
                break;
            }
        }

        if(!exists) {
            Author a;
            a.name = poems[i].author;
            authors.push_back(a);
        }
    }
}

void filterAuthors(char letter) {
    filteredAuthors.clear();

    for (int i = 0; i < authors.size(); i++) {
        String last =
            authors[i].name.substring( 
                authors[i].name.lastIndexOf(' ') + 1);

        if (toupper(last[0]) == toupper(letter))
                filteredAuthors.push_back(i);
    }

    authorIndex = 0;
    currentScreen = AUTHORS;
}

void filterPoemsByAuthor(String author) {
    filtered.clear();

    for (int i = 0; i < poems.size(); i++) {
        if (poems[i].author == author)
            filtered.push_back(i);
    }

    listIndex = 0;
    currentScreen = LIST;
}

void openPoem(int idx) {
    currentPoem = idx;

    File f = LittleFS.open("/" + poems[idx].file, "r");
    poemText = f.readString();
    f.close();

    readerScroll = 0;
    readerButton = 0;
    currentScreen = READER;
}

void drawHome() {
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);
    
    for (int i = 0; i < 26; i++) {
        int x = (i % 4) * 30 + 8;
        int y = (i / 4) * 16 + 8;

        if (i == homeIndex)
            display.drawBox(x - 4, y - 10, 22, 14);

        display.setDrawColor(i == homeIndex ? 0 : 1);

        if (i < 26)
            display.drawStr(x, y, String(alphabet[i]).c_str());

        display.setDrawColor(1);
    }

    display.sendBuffer();
}

void drawAuthors() {
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);

    int first =
        (authorIndex < 8) ? 0 : authorIndex - 7;

    for (int i = 0; i < 8 && first + i < filteredAuthors.size(); i++) {
        int idx = filteredAuthors[first + i];

        if (first + i == authorIndex)
            display.drawBox(0, i * 15, 128, 14);

        display.setDrawColor(first + i == authorIndex ? 0 : 1);

        display.drawStr(2, i * 15 + 11, authors[idx].name.c_str());

        display.setDrawColor(1);
    }

    display.sendBuffer();
}

void drawList() {
    display.clearBuffer();
    display.setFont(u8g2_font_6x10_tf);

    int firstVisible = max(0, listIndex - poemsPerList + 1);

    for (int i = 0;
         firstVisible + i < filtered.size() && i < poemsPerList; 
         i++) {
        int globalIndex = firstVisible + i;
        
        int idx = filtered[globalIndex];

        if (globalIndex == listIndex)
            display.drawBox(0, i * 15, 128, 14);

        display.setDrawColor(globalIndex == listIndex ? 0 : 1);
        display.drawStr(2, i * 15 + 11, poems[idx].title.c_str());
        display.setDrawColor(1);
    }

    display.sendBuffer();
}

void drawReader() {
    display.clearBuffer();
    display.setFont(u8g2_font_5x8_tf);

    const int maxWidth = 124;
    int y = 10 - readerScroll;

    int paragraphStart = 0;

    while (paragraphStart < poemText.length()) {
        int paragraphEnd = poemText.indexOf('\n', paragraphStart);
        if (paragraphEnd == -1)
            paragraphEnd = poemText.length();

        String paragraph = poemText.substring(paragraphStart, paragraphEnd);

        String line = "";
        int wordStart = 0;
        
        while (wordStart < paragraph.length()) {
            int wordEnd = paragraph.indexOf(' ', wordStart);
            if (wordEnd == -1)
                wordEnd = paragraph.length();

            String word = paragraph.substring(wordStart, wordEnd);

            String testLine = line.length() == 0 ? word : line + " " + word;
            
            if (display.getStrWidth(testLine.c_str()) <= maxWidth) {
                line = testLine;
            } else {
                display.drawStr(0, y, line.c_str());
                y += 8;
                line = word;
            }

            wordStart = wordEnd + 1;
        }


        if (line.length() > 0) {
            display.drawStr(0, y, line.c_str());
            y += 8;
        }

        y += 4;
        paragraphStart = paragraphEnd + 1;
    }

    int by = 118;

    if (readerButton == 0)
        display.drawBox(0, by, 50, 10);

    display.setDrawColor(readerButton == 0 ? 0 : 1);
    display.drawStr(5, 126, "home");
    display.setDrawColor(1);

    if (readerButton == 1)
        display.drawBox(70, by, 50, 10);

    display.setDrawColor(readerButton == 1 ? 0 : 1);

    display.setDrawColor(1);
    display.sendBuffer();
}

void handleHome() {
    if (pressed(BTN_RIGHT)) homeIndex++;
    if (pressed(BTN_LEFT)) homeIndex--;
    if (pressed(BTN_DOWN)) homeIndex += 4;
    if (pressed(BTN_UP)) homeIndex -= 4;

    if (homeIndex < 0) homeIndex = 0;
    if (homeIndex > 25) homeIndex = 25;

    if (pressed(BTN_SELECT)) {
        filterAuthors(alphabet[homeIndex]);
    }
}

void handleAuthors() {
    if (pressed(BTN_DOWN) && authorIndex < filteredAuthors.size() - 1)
        authorIndex++;

    if (pressed(BTN_UP) && authorIndex > 0)
        authorIndex--;

    if (pressed(BTN_LEFT))
        currentScreen = HOME;

    if (pressed(BTN_SELECT)) {
        currentAuthor = filteredAuthors[authorIndex];

        filterPoemsByAuthor(authors[currentAuthor].name);
    }
}

void handleList() {
    if (pressed(BTN_DOWN) && listIndex < filtered.size() - 1)
    {
        listIndex++;
        Serial.println(listIndex);
    }

    if (pressed(BTN_UP) && listIndex > 0)
        listIndex--;

    if (pressed(BTN_LEFT))
        currentScreen = AUTHORS;

    if (pressed(BTN_SELECT))
        openPoem(filtered[listIndex]);
}

void handleReader() {
    if (pressed(BTN_UP))
        readerScroll -= 8;

    if (pressed(BTN_DOWN))
        readerScroll += 8;

    if (readerScroll < 0)
        readerScroll = 0;

    if (pressed(BTN_LEFT))
        readerButton = 0;

    if (pressed(BTN_RIGHT))
        readerButton = 1;

    if (pressed(BTN_SELECT))
        currentScreen = HOME;
}

void setup() {
    Serial.begin(115200);

    if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_GPIO) {
        Serial.println("Woke up!");
    }

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_POWER, INPUT_PULLUP);

    Wire.begin();
    display.begin();
    delay(50);
    display.setPowerSave(0);
    display.clearBuffer();

    LittleFS.begin();
    prefs.begin("poems", false);

    loadPoems();

    while (digitalRead(BTN_POWER) == LOW) {
        delay(10);
    }
}

void enterDeepSleep() {
    display.clearBuffer();
    display.sendBuffer();
    display.setPowerSave(1);

    delay(50);

    while (digitalRead(BTN_POWER) == LOW) {
        delay(10);
    }

    delay(200);

    uint8_t gpio_num = digitalPinToGPIONumber(BTN_POWER);
    esp_deep_sleep_enable_gpio_wakeup((1ULL << gpio_num), ESP_GPIO_WAKEUP_GPIO_LOW);

    delay(50);

    esp_deep_sleep_start();
}

void loop() {
    if (pressed(BTN_POWER))
        enterDeepSleep();

    switch (currentScreen) {
        case HOME:
            handleHome();
            drawHome();
            break;

        case AUTHORS:
            handleAuthors();
            drawAuthors();
            break;

        case LIST:
            handleList();
            drawList();
            break;
        
        case READER:
            handleReader();
            drawReader();
            break;
    }

    if (digitalRead(BTN_POWER) == LOW)
        Serial.println("POWER LOW");

    if (digitalRead(BTN_LEFT) == LOW)
        Serial.println("LEFT LOW");

    delay(20);
}