#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <optional>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include "TypingSession.h"
#include "PracticeMode.h"
#include "TimedChallenge.h"
#include "PassageManager.h"
#include "Statistics.h"
#include "FileManager.h"
#include "Theme.h"
#include "RoundedRect.h"
#include "Widget.h"
#include "Button.h"
#include "StatCard.h"

using namespace std;

// Screen layout of the passage box (in pixels)
const float PASSAGE_X = 60;
const float PASSAGE_Y = 146;
const float PASSAGE_W = 880;
const float PASSAGE_H = 262;
const float TEXT_PADDING = 30;
const float LINE_HEIGHT = 36;
const int MAX_LINES = 6;
const unsigned int PASSAGE_FONT_SIZE = 22;

template <typename T>
string toString(T value, int decimals = 2)
{
    stringstream ss;
    ss << fixed << setprecision(decimals) << value;
    return ss.str();
}

// Loads a font from the assets folder. If it is missing, a Windows font is used.
sf::Font loadFont(string fileName, string windowsFont)
{
    sf::Font font;

    if(font.openFromFile("assets/fonts/" + fileName))
        return font;

    if(font.openFromFile("C:/Windows/Fonts/" + windowsFont))
        return font;

    throw runtime_error("Error: font " + fileName + " not found!");
}

class TypeMasterApp
{
private:
    // ---------- window and fonts (fonts must be loaded before the widgets) ----------
    sf::RenderWindow window;
    sf::Font regularFont;
    sf::Font boldFont;
    sf::Font monoFont;

    // ---------- typing logic ----------
    PassageManager passageManager;
    Statistics statistics;
    FileManager fileManager;
    PracticeMode practice;
    TimedChallenge timed;
    TypingSession* session;     // points to practice or timed (runtime polymorphism)

    // Passage INDEX is zero-based (0 .. count-1)
    // On screen we show passageNumber + 1
    int passageNumber;
    string targetText;
    string userInput;
    string targetDone;          // timed mode: passages that were already finished
    string typedDone;

    bool started;
    bool finished;
    bool timedMode;
    bool newBest;
    float finalTime;
    float bestWPM;

    sf::Clock timer;            // time of the typing test
    sf::Clock frameClock;       // time between two frames (for smooth animations)
    sf::Clock animationClock;   // time since the app was opened

    // ---------- widgets ----------
    Button practiceButton;
    Button timedButton;
    Button restartButton;
    Button nextButton;
    Button saveButton;
    StatCard wpmCard;
    StatCard accuracyCard;
    StatCard timeCard;
    StatCard correctCard;
    StatCard wrongCard;
    vector<Widget*> widgets;    // all widgets, so they can be updated and drawn in one loop

    // ---------- passage layout and animation ----------
    vector<sf::Vector2f> charPositions;
    float charWidth;
    sf::Vector2f caretPosition;
    float lastKeyTime;
    float progress;
    float resultFade;
    string toastText;
    float toastTime;

public:
    TypeMasterApp()
        : regularFont(loadFont("Poppins-Regular.ttf", "segoeui.ttf")),
          boldFont(loadFont("Poppins-SemiBold.ttf", "segoeuib.ttf")),
          monoFont(loadFont("JetBrainsMono-Regular.ttf", "consola.ttf")),
          practice(""),
          timed(""),
          practiceButton(boldFont, "Practice", {694, 34}, {118, 40}),
          timedButton(boldFont, "Timed " + to_string(timed.getTimeLimit()) + "s", {816, 34}, {118, 40}),
          restartButton(boldFont, "Restart", {214, 566}, {160, 50}),
          nextButton(boldFont, "Next Passage", {390, 566}, {190, 50}),
          saveButton(boldFont, "Save Result", {596, 566}, {190, 50}, true),
          wpmCard(regularFont, boldFont, "WPM", Theme::AccentLight, {60, 452}, {164, 92}),
          accuracyCard(regularFont, boldFont, "ACCURACY", Theme::Cyan, {239, 452}, {164, 92}),
          timeCard(regularFont, boldFont, "TIME", Theme::Amber, {418, 452}, {164, 92}),
          correctCard(regularFont, boldFont, "CORRECT", Theme::Green, {597, 452}, {164, 92}),
          wrongCard(regularFont, boldFont, "WRONG", Theme::Red, {776, 452}, {164, 92})
    {
        passageManager.loadPassages("prompts.txt");

        sf::ContextSettings settings;
        settings.antiAliasingLevel = 8;     // smooth round corners

        window.create(sf::VideoMode({1000, 700}),
                      "TypeMaster - Typing Speed & Accuracy Trainer",
                      sf::Style::Titlebar | sf::Style::Close,
                      sf::State::Windowed, settings);
        window.setFramerateLimit(60);

        widgets = {&practiceButton, &timedButton, &restartButton, &nextButton, &saveButton,
                   &wpmCard, &accuracyCard, &timeCard, &correctCard, &wrongCard};

        session = &practice;
        passageNumber = 0;
        charWidth = monoFont.getGlyph('M', PASSAGE_FONT_SIZE, false).advance;
        lastKeyTime = 0;
        resultFade = 0;
        toastTime = 0;

        resetSession(false);
    }

    void run()
    {
        while(window.isOpen())
        {
            while(const optional event = window.pollEvent())
                handleEvent(*event);

            update(frameClock.restart().asSeconds());
            draw();
        }
    }

private:
    // =============================== LOGIC ===============================

    // Gets everything ready for a new test (used by the buttons and TAB)
    void resetSession(bool timedChoice)
    {
        timedMode = timedChoice;
        targetText = passageManager.getPassage(passageNumber);

        if(timedMode)
        {
            timed = TimedChallenge(targetText);
            session = &timed;
        }
        else
        {
            practice = PracticeMode(targetText);
            session = &practice;
        }

        userInput = "";
        targetDone = "";
        typedDone = "";
        started = false;
        finished = false;
        newBest = false;
        finalTime = 0;
        progress = 0;
        statistics = Statistics();
        bestWPM = fileManager.getBestWPM(session->getModeName());

        layoutPassage();
    }

    void startTyping()
    {
        started = true;
        userInput = "";
        session->startSession();
        timer.restart();
    }

    // Ends the test, calculates the final result and saves it (only once)
    void finishSession(float seconds)
    {
        if(seconds <= 0)
            seconds = 0.01f;

        finalTime = seconds;

        statistics.calculate(targetDone + targetText, typedDone + userInput, seconds);

        fileManager.saveResult(
            session->getModeName(),
            statistics.getWPM(),
            statistics.getAccuracy(),
            statistics.getCorrect(),
            statistics.getWrong()
        );

        newBest = statistics.getWPM() > bestWPM;
        if(newBest)
            bestWPM = statistics.getWPM();

        started = false;
        finished = true;
        resultFade = 0;
        showToast("Result saved to user_stats.csv");
    }

    // Timed mode: the passage is done but there is time left, so load the next one.
    // The timer is NOT restarted because Timed Mode is one continuous test.
    void nextTimedPassage()
    {
        targetDone += targetText;
        typedDone += userInput;

        passageNumber = (passageNumber + 1) % passageManager.getCount();
        targetText = passageManager.getPassage(passageNumber);

        timed = TimedChallenge(targetText);
        session = &timed;
        userInput = "";

        layoutPassage();
    }

    void saveResult()
    {
        if(finished)
            showToast("This result is already saved");
        else if(!started || (userInput.empty() && typedDone.empty()))
            showToast("Type something first, then save");
        else
            finishSession(timer.getElapsedTime().asSeconds());
    }

    void showToast(string message)
    {
        toastText = message;
        toastTime = 2.5f;
    }

    // ============================== EVENTS ===============================

    void handleEvent(const sf::Event& event)
    {
        if(event.is<sf::Event::Closed>())
            window.close();

        if(const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>())
        {
            if(mouse->button == sf::Mouse::Button::Left)
                handleClick(sf::Vector2f(mouse->position));
        }

        if(const auto* text = event.getIf<sf::Event::TextEntered>())
            handleTyping(text->unicode);

        if(const auto* key = event.getIf<sf::Event::KeyPressed>())
            handleKey(key->code);
    }

    void handleClick(sf::Vector2f point)
    {
        if(practiceButton.contains(point))
            resetSession(false);
        else if(timedButton.contains(point))
            resetSession(true);
        else if(restartButton.contains(point))
            resetSession(timedMode);
        else if(nextButton.contains(point))
        {
            // Go to next passage, after the last one go back to first
            passageNumber = (passageNumber + 1) % passageManager.getCount();
            resetSession(timedMode);
        }
        else if(saveButton.contains(point))
            saveResult();
    }

    void handleKey(sf::Keyboard::Key code)
    {
        if(code == sf::Keyboard::Key::Enter)
        {
            if(finished)
                resetSession(timedMode);     // try the same passage again

            if(!started)
                startTyping();
        }
        else if(code == sf::Keyboard::Key::Tab)
        {
            resetSession(timedMode);
        }
    }

    void handleTyping(unsigned int unicode)
    {
        if(finished)
            return;

        // typing a letter also starts the test (same as pressing ENTER)
        if(!started && unicode >= 32 && unicode < 127)
            startTyping();

        if(!started)
            return;

        // BACKSPACE
        if(unicode == 8)
        {
            if(!userInput.empty())
                userInput.pop_back();
        }
        // NORMAL CHARACTER
        else if(unicode >= 32 && unicode < 127)
        {
            if(userInput.length() < targetText.length())
                userInput += static_cast<char>(unicode);
        }

        session->setUserInput(userInput);
        lastKeyTime = animationClock.getElapsedTime().asSeconds();

        // whole passage typed
        if(userInput.length() == targetText.length())
        {
            if(timedMode)
                nextTimedPassage();
            else
                finishSession(timer.getElapsedTime().asSeconds());
        }
    }

    // ============================== UPDATE ===============================

    void update(float deltaTime)
    {
        sf::Vector2f mouse(sf::Mouse::getPosition(window));

        for(Widget* widget : widgets)
            widget->update(mouse, deltaTime);

        practiceButton.setActive(!timedMode);
        timedButton.setActive(timedMode);

        float elapsed = started ? timer.getElapsedTime().asSeconds() : finalTime;

        // TIMED MODE - stop when the time limit is reached
        if(started && session->isTimeUp(elapsed))
        {
            finishSession(timed.getTimeLimit());
            elapsed = finalTime;
        }

        // LIVE STATISTICS (at least 1 second, so WPM does not jump at the start)
        if(started)
            statistics.calculate(targetDone + targetText, typedDone + userInput, max(elapsed, 1.0f));

        wpmCard.setValue(toString(statistics.getWPM(), 1));
        accuracyCard.setValue(toString(statistics.getAccuracy(), 1) + "%");
        correctCard.setValue(toString(statistics.getCorrect()));
        wrongCard.setValue(toString(statistics.getWrong()));

        float goal;     // how full the progress bar should be (0 to 1)

        if(timedMode)
        {
            int left = max(0, static_cast<int>(ceil(timed.getTimeLimit() - elapsed)));

            timeCard.setValue(to_string(left) + "s");
            timeCard.setAccent(started && left <= 10 ? Theme::Red : Theme::Amber);
            goal = elapsed / timed.getTimeLimit();
        }
        else
        {
            timeCard.setValue(toString(elapsed, 1) + "s");
            goal = static_cast<float>(userInput.length()) / targetText.length();
        }

        // animations move a little towards their goal every frame
        progress += (goal - progress) * min(1.f, deltaTime * 10);
        caretPosition += (caretTarget() - caretPosition) * min(1.f, deltaTime * 25);

        if(finished)
            resultFade = min(1.f, resultFade + deltaTime * 3);

        if(toastTime > 0)
            toastTime -= deltaTime;
    }

    // Word wrap: works out where every letter of the passage goes,
    // so a word is never cut in half at the end of a line
    void layoutPassage()
    {
        charPositions.clear();

        int maxColumns = static_cast<int>((PASSAGE_W - 2 * TEXT_PADDING) / charWidth);
        int column = 0;
        int line = 0;

        for(size_t i = 0; i < targetText.length(); i++)
        {
            bool wordStart = (i == 0 || targetText[i - 1] == ' ');

            if(wordStart)
            {
                size_t wordEnd = targetText.find(' ', i);

                if(wordEnd == string::npos)
                    wordEnd = targetText.length();

                // the word does not fit on this line, so move it to the next one
                if(column > 0 && column + static_cast<int>(wordEnd - i) > maxColumns)
                {
                    column = 0;
                    line++;
                }
            }

            charPositions.push_back(sf::Vector2f(column * charWidth, line * LINE_HEIGHT));
            column++;
        }

        caretPosition = caretTarget();      // jump there without animation
    }

    int lineOf(size_t index)
    {
        return static_cast<int>(charPositions[index].y / LINE_HEIGHT + 0.5f);
    }

    // First line that is shown. A very long passage scrolls while you type.
    int firstVisibleLine()
    {
        int caretLine = lineOf(min(userInput.length(), targetText.length() - 1));
        int lastLine = lineOf(targetText.length() - 1);

        int first = max(0, caretLine - (MAX_LINES - 2));
        return min(first, max(0, lastLine - MAX_LINES + 1));
    }

    sf::Vector2f textOrigin()
    {
        return sf::Vector2f(PASSAGE_X + TEXT_PADDING,
                            PASSAGE_Y + TEXT_PADDING - firstVisibleLine() * LINE_HEIGHT);
    }

    // Where the caret should be: on the next letter to type
    sf::Vector2f caretTarget()
    {
        if(userInput.length() < charPositions.size())
            return textOrigin() + charPositions[userInput.length()];

        return textOrigin() + charPositions.back() + sf::Vector2f(charWidth, 0);
    }

    // =============================== DRAW ================================

    void draw()
    {
        float time = animationClock.getElapsedTime().asSeconds();

        window.clear(Theme::BackgroundTop);

        drawBackground(time);
        drawHeader();

        // polymorphism: calls Button::draw() or StatCard::draw()
        for(Widget* widget : widgets)
            widget->draw(window);

        drawPassage(time);

        if(finished)
            drawResult();

        drawFooter(time);

        window.display();
    }

    void drawBackground(float time)
    {
        sf::VertexArray background(sf::PrimitiveType::TriangleStrip, 4);

        background[0].position = {0, 0};
        background[1].position = {1000, 0};
        background[2].position = {0, 700};
        background[3].position = {1000, 700};
        background[0].color = Theme::BackgroundTop;
        background[1].color = Theme::BackgroundTop;
        background[2].color = Theme::BackgroundBottom;
        background[3].color = Theme::BackgroundBottom;

        window.draw(background);

        // three big soft lights that slowly float around
        Theme::drawGlow(window, {170 + 60 * sin(time * 0.30f), 110 + 40 * cos(time * 0.40f)},
                        {440, 360}, Theme::fade(Theme::Accent, 0.30f));
        Theme::drawGlow(window, {860 + 50 * cos(time * 0.35f), 640 + 40 * sin(time * 0.30f)},
                        {440, 340}, Theme::fade(Theme::Cyan, 0.15f));
        Theme::drawGlow(window, {720 + 90 * sin(time * 0.20f), 200 + 60 * cos(time * 0.25f)},
                        {320, 260}, Theme::fade(Theme::Pink, 0.12f));
    }

    void drawHeader()
    {
        // logo
        RoundedRect logo({46, 46}, 14);
        logo.setPosition({60, 28});
        logo.setFillColor(Theme::Accent);

        Theme::drawGlow(window, {83, 51}, {60, 60}, Theme::fade(Theme::Accent, 0.6f));
        window.draw(logo);
        drawText("T", boldFont, 26, sf::Color::White, {83, 51});

        // title
        float width = drawText("Type", boldFont, 30, Theme::TextBright, {122, 42}, 0);
        drawText("Master", boldFont, 30, Theme::AccentLight, {124 + width, 42}, 0);
        drawText("Typing Speed & Accuracy Trainer", regularFont, 13, Theme::TextMuted, {123, 68}, 0);

        // dark background behind the two mode buttons
        RoundedRect track({252, 52}, 16);
        track.setPosition({688, 28});
        track.setFillColor(sf::Color(0, 0, 0, 70));
        track.setOutlineThickness(1);
        track.setOutlineColor(Theme::Border);
        window.draw(track);

        // small labels under the title
        string passageInfo = "Passage " + toString(passageNumber + 1) +
                             " of " + toString(passageManager.getCount());
        drawPill(passageInfo, 60, 104, Theme::AccentLight);

        float x = 60 + pillWidth(passageInfo) + 10;

        if(finished)
            drawPill("Finished", x, 104, Theme::AccentLight);
        else if(started)
            drawPill("Typing...", x, 104, Theme::Green);
        else
            drawPill("Ready", x, 104, Theme::Amber);

        string best = session->getModeName() + " best: " +
                      (bestWPM > 0 ? toString(bestWPM, 1) + " WPM" : "no result yet");
        drawPill(best, 940 - pillWidth(best), 104, Theme::Amber);
    }

    void drawPassage(float time)
    {
        RoundedRect card({PASSAGE_W, PASSAGE_H}, 20);
        card.setPosition({PASSAGE_X, PASSAGE_Y});
        card.setFillColor(Theme::Surface);
        card.setOutlineThickness(1);
        card.setOutlineColor(Theme::Border);
        window.draw(card);

        int firstLine = firstVisibleLine();
        sf::Vector2f origin = textOrigin();

        sf::Text letter(monoFont, "", PASSAGE_FONT_SIZE);
        sf::RectangleShape errorBox({charWidth, LINE_HEIGHT - 6});
        errorBox.setFillColor(Theme::fade(Theme::Red, 0.2f));

        for(size_t i = 0; i < targetText.length(); i++)
        {
            int line = lineOf(i);

            if(line < firstLine || line >= firstLine + MAX_LINES)
                continue;

            sf::Vector2f position = origin + charPositions[i];
            sf::Color color = Theme::TextDim;

            if(i < userInput.length())
            {
                if(userInput[i] == targetText[i])
                {
                    color = Theme::TextBright;
                }
                else
                {
                    // red box, so a wrong space is also visible
                    color = Theme::Red;
                    errorBox.setPosition(position);
                    window.draw(errorBox);
                }
            }

            if(targetText[i] != ' ')
            {
                letter.setString(string(1, targetText[i]));
                letter.setFillColor(color);
                letter.setPosition({round(position.x), round(position.y)});
                window.draw(letter);
            }
        }

        // caret: blinks while waiting, stays solid while you type
        if(!finished)
        {
            bool typing = started && time - lastKeyTime < 0.6f;
            float blink = typing ? 1 : 0.5f + 0.5f * cos(time * 5);

            Theme::drawGlow(window, caretPosition + sf::Vector2f(1, 16), {10, 22},
                            Theme::fade(Theme::Cyan, 0.5f * blink));

            RoundedRect caret({3, 28}, 1.5f);
            caret.setPosition(caretPosition + sf::Vector2f(-1, 2));
            caret.setFillColor(Theme::fade(Theme::Cyan, blink));
            window.draw(caret);
        }
    }

    // Result card that appears on top of the passage when the test is over
    void drawResult()
    {
        float a = 1 - pow(1 - resultFade, 3);      // ease out: fast first, then slow
        float top = PASSAGE_Y + (1 - a) * 18;      // slides up a little
        float centerX = PASSAGE_X + PASSAGE_W / 2;

        RoundedRect panel({PASSAGE_W, PASSAGE_H}, 20);
        panel.setPosition({PASSAGE_X, PASSAGE_Y});
        panel.setFillColor(Theme::fade(sf::Color(18, 16, 44), a));
        panel.setOutlineThickness(1);
        panel.setOutlineColor(Theme::fade(Theme::AccentLight, 0.5f * a));
        window.draw(panel);

        Theme::drawGlow(window, {centerX, top + 104}, {230, 110}, Theme::fade(Theme::Accent, 0.45f * a));

        drawText("TEST COMPLETE", boldFont, 13, Theme::fade(Theme::Cyan, a), {centerX, top + 34}, 0.5f, 3);

        drawText(toString(statistics.getWPM(), 1), boldFont, 72, Theme::fade(sf::Color::White, a), {centerX, top + 102});
        drawText("WORDS PER MINUTE", regularFont, 12, Theme::fade(Theme::TextMuted, a), {centerX, top + 150}, 0.5f, 2);

        drawText(toString(statistics.getAccuracy(), 1) + "%", boldFont, 34, Theme::fade(Theme::Cyan, a), {centerX - 270, top + 98});
        drawText("ACCURACY", regularFont, 12, Theme::fade(Theme::TextMuted, a), {centerX - 270, top + 136}, 0.5f, 2);

        drawText(toString(finalTime, 1) + "s", boldFont, 34, Theme::fade(Theme::Amber, a), {centerX + 270, top + 98});
        drawText("TIME", regularFont, 12, Theme::fade(Theme::TextMuted, a), {centerX + 270, top + 136}, 0.5f, 2);

        // rating label, plus a gold label for a new personal best
        string rating = statistics.getRating();
        string bestText = "New personal best!";

        float width = pillWidth(rating);
        if(newBest)
            width += 12 + pillWidth(bestText);

        float x = centerX - width / 2;
        drawPill(rating, x, top + 180, Theme::AccentLight, a);

        if(newBest)
            drawPill(bestText, x + pillWidth(rating) + 12, top + 180, Theme::Amber, a);

        drawText("Press ENTER to try again, or click Next Passage", regularFont, 13,
                 Theme::fade(Theme::TextMuted, a), {centerX, top + 236});
    }

    void drawFooter(float time)
    {
        float barY = PASSAGE_Y + PASSAGE_H + 18;

        if(!started && !finished)
        {
            // "Press ENTER to start" hint, gently pulsing
            float pulse = 0.7f + 0.3f * sin(time * 3);
            string text = timedMode ? "to start  -  type as much as you can before time runs out"
                                    : "to start  -  or just begin typing the passage";

            float width = drawKeyHint("ENTER", text, 0, 0, 0, true);
            drawKeyHint("ENTER", text, 500 - width / 2, barY + 3, pulse);
        }
        else
        {
            // progress bar: passage typed (practice) or time used (timed)
            RoundedRect track({PASSAGE_W, 6}, 3);
            track.setPosition({PASSAGE_X, barY});
            track.setFillColor(sf::Color(255, 255, 255, 18));
            window.draw(track);

            float width = PASSAGE_W * min(progress, 1.f);
            RoundedRect bar({width, 6}, 3);
            bar.setPosition({PASSAGE_X, barY});
            bar.setFillColor(timedMode ? Theme::Amber : Theme::Cyan);
            window.draw(bar);

            Theme::drawGlow(window, {PASSAGE_X + width, barY + 3}, {26, 12},
                            Theme::fade(timedMode ? Theme::Amber : Theme::Cyan, 0.9f));
        }

        // bottom line: a short message after an action, otherwise keyboard hints
        float y = 668;

        if(toastTime > 0)
        {
            float alpha = min(1.f, min(toastTime / 0.4f, (2.5f - toastTime) / 0.15f));
            drawPill(toastText, 500 - pillWidth(toastText) / 2, y - 15, Theme::Green, alpha);
        }
        else
        {
            float first = drawKeyHint("ENTER", "start / try again", 0, 0, 0, true);
            float second = drawKeyHint("TAB", "restart", 0, 0, 0, true);
            float x = 500 - (first + 36 + second) / 2;

            drawKeyHint("ENTER", "start / try again", x, y, 0.8f);
            drawKeyHint("TAB", "restart", x + first + 36, y, 0.8f);
        }
    }

    // ============================ DRAW HELPERS ============================

    // Draws one line of text centred vertically on point.y. Returns its width.
    float drawText(string content, const sf::Font& font, unsigned int size, sf::Color color,
                   sf::Vector2f point, float alignX = 0.5f, float letterSpacing = 1)
    {
        sf::Text text(font, content, size);
        text.setFillColor(color);
        text.setLetterSpacing(letterSpacing);
        Widget::placeText(text, point, alignX);
        window.draw(text);

        return text.getLocalBounds().size.x;
    }

    float textWidth(string content, const sf::Font& font, unsigned int size)
    {
        sf::Text text(font, content, size);
        return text.getLocalBounds().size.x;
    }

    float pillWidth(string content)
    {
        return textWidth(content, regularFont, 13) + 44;
    }

    // Small rounded label with a coloured dot, like "Passage 1 of 4"
    void drawPill(string content, float x, float y, sf::Color color, float alpha = 1)
    {
        RoundedRect pill({pillWidth(content), 30}, 15);
        pill.setPosition({x, y});
        pill.setFillColor(Theme::fade(color, 0.12f * alpha));
        pill.setOutlineThickness(1);
        pill.setOutlineColor(Theme::fade(color, 0.4f * alpha));
        window.draw(pill);

        sf::CircleShape dot(3.5f);
        dot.setFillColor(Theme::fade(color, alpha));
        dot.setPosition({x + 12.5f, y + 11.5f});
        window.draw(dot);

        drawText(content, regularFont, 13, Theme::fade(Theme::TextBright, alpha), {x + 28, y + 15}, 0);
    }

    // Draws a keyboard key like [ENTER] with a short text after it. Returns the width.
    // With measureOnly = true nothing is drawn (used to centre the hints).
    float drawKeyHint(string key, string text, float x, float y, float alpha, bool measureOnly = false)
    {
        float keyWidth = textWidth(key, boldFont, 11) + 18;
        float total = keyWidth + 10 + textWidth(text, regularFont, 13);

        if(measureOnly)
            return total;

        RoundedRect keyCap({keyWidth, 24}, 6);
        keyCap.setPosition({x, y - 12});
        keyCap.setFillColor(Theme::fade(Theme::SurfaceHover, alpha));
        keyCap.setOutlineThickness(1);
        keyCap.setOutlineColor(Theme::fade(Theme::Border, alpha));
        window.draw(keyCap);

        drawText(key, boldFont, 11, Theme::fade(Theme::TextBright, alpha), {x + keyWidth / 2, y});
        drawText(text, regularFont, 13, Theme::fade(Theme::TextMuted, alpha), {x + keyWidth + 10, y}, 0);

        return total;
    }
};

// Work inside the program's own folder, so prompts.txt and assets/ are found
// even when the program is opened by double-clicking it (needed on macOS)
void useProgramFolder(const char* programPath)
{
    try
    {
        filesystem::path folder = filesystem::path(programPath).parent_path();

        if(!folder.empty())
            filesystem::current_path(folder);
    }
    catch(exception &e)
    {
        // keep the current folder
    }
}

int main(int argc, char* argv[])
{
    if(argc > 0)
        useProgramFolder(argv[0]);

    try
    {
        TypeMasterApp app;
        app.run();
    }
    catch(exception &e)
    {
        cout << e.what() << endl;
        cout << "Press Enter to close...";
        cin.get();
    }

    return 0;
}
