#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#define _CRT_SECURE_NO_WARNINGS

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 800
#define NUM_CARDS 3
#define TOTAL_CARDS 22
#define MAX_LINES 8
#define CARD_WIDTH 350      // ปรับขนาดกว้างลดลง
#define CARD_HEIGHT 550     // ปรับความสูงลดลง
#define DESC_FONT_SIZE 20
#define LINE_SPACING 26

typedef enum { START_SCREEN, SELECTING, REVEALING, RESULT } GameState;

typedef struct {
    Rectangle bounds;
    const char *name;
    const char *description;
    bool revealed;
    Color color;
} Card;

Card cards[NUM_CARDS];
GameState gameState = START_SCREEN;
int currentCard = 0;

const char *cardNames[TOTAL_CARDS] = {
    "The Fool", "The Magician", "The High Priestess", "The Empress", "The Emperor",
    "The Hierophant", "The Lovers", "The Chariot", "Strength", "The Hermit",
    "Wheel of Fortune", "Justice", "The Hanged Man", "Death", "Temperance",
    "The Devil", "The Tower", "The Star", "The Moon", "The Sun",
    "Judgement", "The World"
};

const char *descriptions[TOTAL_CARDS] = {
    /* 0 */ "Embracing new beginnings with childlike wonder\nTrusting the universe's plan without fear\nA clean slate awaits - take the first step\nPotential is limitless when heart remains open",
    /* 1 */ "Channeling creative energy to manifest dreams\nMastering the elements: earth, air, fire, water\nTools for success are already in your hands\nStrategic action brings desired outcomes",
    /* 2 */ "Mysteries unfolding in divine timing\nPay attention to dreams and synchronicities\nSacred knowledge revealed through intuition\nA spiritual awakening is imminent",
    /* 3 */ "Nurturing creativity and abundance\nFertility in ideas and relationships\nMotherly protection guides your path\nManifest prosperity through compassion",
    /* 4 */ "Establishing order through disciplined leadership\nSetting clear boundaries and structure\nProtecting what matters most\nBalancing power with responsibility",
    /* 5 */ "Seeking traditional wisdom\nLearning from established institutions\nFinding mentors\nFollowing conventional paths",
    /* 6 */ "Harmonious partnerships\nAlignment of values\nImportant choices\nFollowing heart's true desire",
    /* 7 */ "Victory through determination\nOvercoming obstacles\nFocused willpower\nStrategic decision making",
    /* 8 */ "Taming inner beasts with gentle compassion\nTrue strength flows from self-acceptance\nVulnerability becomes your greatest power\nBalance ferocity and tenderness within",
    /* 9 */ "Soul-searching journey\nInner wisdom discovery\nWithdrawing for reflection\nSeeking deeper truths",
    /* 10 */ "Cycles of destiny\nEmbracing changes\nCosmic alignment\nNew opportunities arising",
    /* 11 */ "Karmic justice restored\nTruth revealed\nLegal matters resolved\nEthical decisions",
    /* 12 */ "Sacrifice for enlightenment\nNew perspectives\nSurrendering control\nSpiritual awakening",
    /* 13 */ "Transformational endings\nRebirth through release\nLetting go\nNew beginnings",
    /* 14 */ "Blending opposites harmoniously\nModeration succeeds\nPatience rewarded\nBalanced approach",
    /* 15 */ "Breaking material chains\nOvercoming addictions\nSelf-liberation\nRecognizing illusions",
    /* 16 */ "Foundations shaken\nTruth revealed through chaos\nEssential destruction\nPhoenix rising from ashes",
    /* 17 */ "Renewed hope\nSpiritual guidance\nHealing energy\nDivine inspiration",
    /* 18 */ "Navigating illusions\nTrusting intuition\nDeciphering mysteries\nShadow work",
    /* 19 */ "Radiant success\nVitality and joy\nConfidence shines\nCelebrating achievements",
    /* 20 */ "Spiritual awakening\nSoul's calling\nJudgment day\nRebirth process",
    /* 21 */ "Cycle completion\nIntegration of experiences\nWholeness achieved\nNew chapter begins"
};

typedef struct {
    char lines[MAX_LINES][256];
    int count;
} WrappedText;

WrappedText WrapText(const char *text, float fontSize, int maxWidth) {
    WrappedText result = {0};
    if(!text) return result;

    char buffer[1024] = {0};
    strncpy(buffer, text, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    char *paragraph = strtok(buffer, "\n");
    while(paragraph && result.count < MAX_LINES) {
        int paragraphLength = (int)strlen(paragraph);
        int start = 0;
        
        while(start < paragraphLength && result.count < MAX_LINES) {
            int end = paragraphLength;
            int lastSpace = -1;
            
            for(int i = start; i <= paragraphLength; i++) {
                if(paragraph[i] == ' ' || paragraph[i] == '\0') {
                    char temp = paragraph[i];
                    paragraph[i] = '\0';
                    int width = MeasureText(&paragraph[start], fontSize);
                    paragraph[i] = temp;
                    
                    if(width > maxWidth && lastSpace != -1) {
                        end = lastSpace;
                        break;
                    }
                    lastSpace = i;
                }
            }
            
            int copyLength = end - start;
            if(copyLength > 0) {
                strncpy(result.lines[result.count], &paragraph[start], copyLength);
                result.lines[result.count][copyLength] = '\0';
                result.count++;
            }
            start = (end == paragraphLength) ? end : end + 1;
        }
        paragraph = strtok(NULL, "\n");
    }
    return result;
}

void InitCards() {
    srand((unsigned int)time(NULL));
    float spacing = 30.0f;  // ระยะห่างระหว่างการ์ดลดลง
    float totalWidth = NUM_CARDS * CARD_WIDTH + (NUM_CARDS - 1) * spacing;
    float startX = (SCREEN_WIDTH - totalWidth) / 2.0f;
    float startY = (SCREEN_HEIGHT - CARD_HEIGHT) * 0.4f; // ปรับตำแหน่งแนวตั้ง
    
    for (int i = 0; i < NUM_CARDS; i++) {
        cards[i].bounds = (Rectangle){
            startX + (CARD_WIDTH + spacing) * (float)i,
            startY,
            (float)CARD_WIDTH,
            (float)CARD_HEIGHT
        };
        cards[i].revealed = false;
        cards[i].color = DARKGRAY;
    }
}

int RandomCardIndex() {
    return rand() % TOTAL_CARDS;
}

void UpdateApp() {
    if (gameState == START_SCREEN) {
        Rectangle startButton = {
            SCREEN_WIDTH/2.0f - 110.0f,
            SCREEN_HEIGHT/2.0f + 100.0f,
            220.0f,
            60.0f
        };
        
        if (GuiButton(startButton, "Begin Reading")) {
            gameState = SELECTING;
            InitCards();
            currentCard = 0;
        }
    }
    else if (gameState == SELECTING || gameState == REVEALING) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < NUM_CARDS; i++) {
                if (CheckCollisionPointRec(GetMousePosition(), cards[i].bounds)) {
                    if (!cards[i].revealed && currentCard == i) {
                        int idx = RandomCardIndex();
                        if(idx >= 0 && idx < TOTAL_CARDS) {
                            cards[i].name = cardNames[idx];
                            cards[i].description = descriptions[idx];
                            cards[i].revealed = true;
                            cards[i].color = (Color){
                                (unsigned char)(rand()%200 + 55),
                                (unsigned char)(rand()%200 + 55),
                                (unsigned char)(rand()%200 + 55),
                                255
                            };
                            currentCard++;
                            if (currentCard >= NUM_CARDS) gameState = RESULT;
                        }
                    }
                }
            }
        }
    }
}

void DrawApp() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (gameState == START_SCREEN) {
        DrawText("Mystic Tarot Reader", 
            (int)(SCREEN_WIDTH - MeasureText("Mystic Tarot Reader", 70))/2, 
            100, 70, DARKPURPLE);
        
        DrawText("Reveal Your Destiny", 
            (int)(SCREEN_WIDTH - MeasureText("Reveal Your Destiny", 35))/2, 
            200, 35, DARKGRAY);
    }
    else {
        for (int i = 0; i < NUM_CARDS; i++) {
            DrawRectangleRounded(cards[i].bounds, 0.12f, 12, cards[i].color);
            
            if (cards[i].revealed && cards[i].description) {
                DrawText(cards[i].name, 
                    (int)cards[i].bounds.x + 20,  // ปรับระยะห่างขอบ
                    (int)cards[i].bounds.y + 20, 
                    28, BLACK);
                
                WrappedText wrapped = WrapText(cards[i].description, DESC_FONT_SIZE, CARD_WIDTH - 40);
                float y = cards[i].bounds.y + 70.0f;
                
                for (int j = 0; j < wrapped.count; j++) {
                    DrawText(wrapped.lines[j], 
                        (int)cards[i].bounds.x + 20, 
                        (int)y, 
                        DESC_FONT_SIZE, DARKBROWN);
                    y += LINE_SPACING;
                }
            } else {
                const char* num = TextFormat("%d", i+1);
                int textWidth = MeasureText(num, 50);
                DrawText(num,
                    (int)(cards[i].bounds.x + (CARD_WIDTH - textWidth)/2),
                    (int)(cards[i].bounds.y + (CARD_HEIGHT - 50)/2),
                    50, WHITE);
            }
        }

        const char* labels[] = {"Past", "Present", "Future"};
        for (int i = 0; i < NUM_CARDS; i++) {
            int textWidth = MeasureText(labels[i], 26); // ปรับขนาดฟอนต์
            DrawText(labels[i],
                (int)(cards[i].bounds.x + (CARD_WIDTH - textWidth)/2),
                (int)cards[i].bounds.y - 40, // ปรับตำแหน่งป้าย
                26, DARKGRAY);
        }

        if (gameState == RESULT) {
            DrawRectangle(0, SCREEN_HEIGHT - 80, SCREEN_WIDTH, 80, Fade(SKYBLUE, 0.9f));
            DrawText("Final Destiny Reading:", 30, SCREEN_HEIGHT - 65, 26, DARKBLUE);
            DrawText(TextFormat("Past: %s", cards[0].name), 30, SCREEN_HEIGHT - 40, 22, DARKBLUE);
            DrawText(TextFormat("Present: %s", cards[1].name), 350, SCREEN_HEIGHT - 40, 22, DARKBLUE);
            DrawText(TextFormat("Future: %s", cards[2].name), 670, SCREEN_HEIGHT - 40, 22, DARKBLUE);
        }
    }

    EndDrawing();
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mystic Tarot Reader");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateApp();
        DrawApp();
    }

    CloseWindow();
    return 0;
}