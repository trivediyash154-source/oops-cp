#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <optional>
#include <vector>
#include <stdexcept>
#include "TypingSession.h"
#include "PracticeMode.h"
#include "TimedChallenge.h"
#include "PassageManager.h"
#include "Statistics.h"
#include "FileManager.h"

using namespace std;

template <typename T>
string toString(T value)
{
    stringstream ss;
    ss << fixed << setprecision(2) << value;
    return ss.str();
}

int main()
{
    try
    {
        PassageManager passageManager;
        passageManager.loadPassages("prompts.txt");

        sf::RenderWindow window(
            sf::VideoMode({1000, 700}),
            "TypeMaster - Typing Speed & Accuracy Trainer"
        );

        sf::Font font;

        if(!font.openFromFile("C:/Windows/Fonts/segoeui.ttf"))
            throw runtime_error("Error: segoeui.ttf not found!");

        // Passage INDEX is zero-based (0 .. count-1)
        // On screen we show passageNumber + 1
        int passageNumber = 0;

        string targetText = passageManager.getPassage(passageNumber);

        PracticeMode practice(targetText);
        TimedChallenge timed(targetText, 60);

        TypingSession* session = &practice;

        Statistics statistics;
        FileManager fileManager;

        string userInput = "";

        bool started = false;
        bool finished = false;
        bool timedMode = false;

        float finalTime = 0;

        sf::Clock timer;

        while(window.isOpen())
        {
            while(const optional event = window.pollEvent())
            {
                if(event->is<sf::Event::Closed>())
                {
                    window.close();
                }

                if(const auto* mouse =
                    event->getIf<sf::Event::MouseButtonPressed>())
                {
                    int mouseX = mouse->position.x;
                    int mouseY = mouse->position.y;

                    if(mouseY >= 570 && mouseY <= 630)
                    {
                        // PRACTICE BUTTON
                        if(mouseX >= 100 && mouseX <= 250)
                        {
                            timedMode = false;

                            targetText =
                                passageManager.getPassage(passageNumber);

                            practice = PracticeMode(targetText);
                            session = &practice;

                            userInput = "";
                            started = false;
                            finished = false;
                            finalTime = 0;

                            statistics = Statistics();
                        }

                        // TIMED BUTTON
                        if(mouseX >= 280 && mouseX <= 430)
                        {
                            timedMode = true;

                            targetText =
                                passageManager.getPassage(passageNumber);

                            timed = TimedChallenge(targetText, 60);
                            session = &timed;

                            userInput = "";
                            started = false;
                            finished = false;
                            finalTime = 0;

                            statistics = Statistics();
                        }

                        // NEXT BUTTON
                        if(mouseX >= 460 && mouseX <= 610)
                        {
                            // Go to next passage, after the last one go back to first
                            passageNumber =
                                (passageNumber + 1) % passageManager.getCount();

                            targetText =
                                passageManager.getPassage(passageNumber);

                            if(timedMode)
                            {
                                timed = TimedChallenge(targetText, 60);
                                session = &timed;
                            }
                            else
                            {
                                practice = PracticeMode(targetText);
                                session = &practice;
                            }

                            userInput = "";
                            started = false;
                            finished = false;
                            finalTime = 0;

                            statistics = Statistics();
                        }

                        // SAVE RESULT BUTTON
                        if(mouseX >= 640 && mouseX <= 800)
                        {
                            if(started || finished)
                            {
                                float seconds = finalTime;

                                if(started && !finished)
                                    seconds =
                                        timer.getElapsedTime().asSeconds();

                                if(seconds <= 0)
                                    seconds = 0.01f;

                                statistics.calculate(
                                    targetText,
                                    userInput,
                                    seconds
                                );

                                fileManager.saveResult(
                                    timedMode ? "Timed" : "Practice",
                                    statistics.getWPM(),
                                    statistics.getAccuracy(),
                                    statistics.getCorrect(),
                                    statistics.getWrong()
                                );

                                finished = true;
                                started = false;
                            }
                        }
                    }
                }

                // TEXT INPUT
                if(const auto* textEvent =
                    event->getIf<sf::Event::TextEntered>())
                {
                    if(started && !finished)
                    {
                        unsigned int unicode = textEvent->unicode;

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

                        // PRACTICE COMPLETED
                        if(!timedMode && userInput == targetText)
                        {
                            finalTime =
                                timer.getElapsedTime().asSeconds();

                            if(finalTime <= 0)
                                finalTime = 0.01f;

                            statistics.calculate(
                                targetText,
                                userInput,
                                finalTime
                            );

                            fileManager.saveResult(
                                "Practice",
                                statistics.getWPM(),
                                statistics.getAccuracy(),
                                statistics.getCorrect(),
                                statistics.getWrong()
                            );

                            finished = true;
                            started = false;
                        }

                        // TIMED MODE - MOVE TO NEXT PASSAGE
                        if(timedMode && userInput == targetText)
                        {
                            passageNumber =
                                (passageNumber + 1) % passageManager.getCount();

                            targetText =
                                passageManager.getPassage(passageNumber);

                            timed =
                                TimedChallenge(targetText, 60);

                            session = &timed;

                            userInput = "";

                            // Do NOT restart timer
                            // because Timed Mode is one continuous 60-sec test
                        }
                    }
                }

                // ENTER TO START
                if(const auto* key =
                    event->getIf<sf::Event::KeyPressed>())
                {
                    if(key->code == sf::Keyboard::Key::Enter &&
                       !started &&
                       !finished)
                    {
                        started = true;
                        finished = false;
                        userInput = "";

                        session->startSession();

                        timer.restart();
                    }
                }
            }

            float elapsedSeconds = 0;

            if(started && !finished)
                elapsedSeconds =
                    timer.getElapsedTime().asSeconds();
            else if(finished)
                elapsedSeconds = finalTime;

            // TIMED MODE - 60 SECOND LIMIT
            if(timedMode &&
               started &&
               !finished &&
               elapsedSeconds >= 60)
            {
                finalTime = 60;

                statistics.calculate(
                    targetText,
                    userInput,
                    60
                );

                fileManager.saveResult(
                    "Timed",
                    statistics.getWPM(),
                    statistics.getAccuracy(),
                    statistics.getCorrect(),
                    statistics.getWrong()
                );

                finished = true;
                started = false;
            }

            // LIVE STATISTICS
            if(started && !finished)
            {
                float timeForStats = elapsedSeconds;

                if(timeForStats <= 0)
                    timeForStats = 0.01f;

                statistics.calculate(
                    targetText,
                    userInput,
                    timeForStats
                );
            }

            window.clear(sf::Color(30, 30, 30));

            // TITLE
            sf::Text title(font, "TYPEMASTER", 40);
            title.setFillColor(sf::Color::White);
            title.setPosition({350, 25});
            window.draw(title);

            // PASSAGE NUMBER
            sf::Text passageNumberText(
                font,
                "Passage " + toString(passageNumber + 1),
                22
            );

            passageNumberText.setFillColor(sf::Color::Yellow);
            passageNumberText.setPosition({80, 90});
            window.draw(passageNumberText);

            // MODE
            sf::Text modeText(
                font,
                timedMode ? "Mode: Timed - 60 Seconds"
                          : "Mode: Practice",
                20
            );

            modeText.setFillColor(sf::Color::Cyan);
            modeText.setPosition({700, 90});
            window.draw(modeText);

            // INSTRUCTION
            sf::Text instruction(
                font,
                started ? "Type the passage..."
                        : "Press ENTER to start typing",
                20
            );

            instruction.setFillColor(sf::Color::Yellow);
            instruction.setPosition({350, 125});
            window.draw(instruction);

            // PASSAGE AREA
            float x = 80;
            float y = 170;

            for(size_t i = 0; i < targetText.length(); i++)
            {
                sf::Text character(
                    font,
                    string(1, targetText[i]),
                    24
                );

                if(i < userInput.length())
                {
                    if(userInput[i] == targetText[i])
                        character.setFillColor(sf::Color::Green);
                    else
                        character.setFillColor(sf::Color::Red);
                }
                else
                {
                    character.setFillColor(sf::Color::White);
                }

                character.setPosition({x, y});
                window.draw(character);

                x += character.getLocalBounds().size.x + 2;

                if(x > 900)
                {
                    x = 80;
                    y += 35;
                }

                // Keep passage inside screen
                if(y > 330)
                    break;
            }

            // INPUT BOX
            sf::RectangleShape inputBox(
                sf::Vector2f(840, 80)
            );

            inputBox.setPosition({80, 350});
            inputBox.setFillColor(sf::Color(50, 50, 50));
            inputBox.setOutlineThickness(2);
            inputBox.setOutlineColor(sf::Color::White);

            window.draw(inputBox);

            // USER INPUT (auto wraps to next line, shows last 2 lines)
            {
                const unsigned int inputFontSize = 22;
                const float maxLineWidth = 810;   // inside the 840px box
                const float lineHeight = 30;
                const size_t visibleLines = 2;

                vector<string> lines;
                string currentLine = "";
                float lineWidth = 0;

                for(size_t i = 0; i < userInput.length(); i++)
                {
                    char c = userInput[i];

                    float w = font.getGlyph(
                        static_cast<unsigned char>(c),
                        inputFontSize,
                        false
                    ).advance;

                    if(lineWidth + w > maxLineWidth)
                    {
                        lines.push_back(currentLine);
                        currentLine = "";
                        lineWidth = 0;
                    }

                    currentLine += c;
                    lineWidth += w;
                }

                lines.push_back(currentLine);

                size_t firstLine = 0;
                if(lines.size() > visibleLines)
                    firstLine = lines.size() - visibleLines;

                for(size_t i = firstLine; i < lines.size(); i++)
                {
                    sf::Text typedText(font, lines[i], inputFontSize);
                    typedText.setFillColor(sf::Color::Cyan);
                    typedText.setPosition({
                        95,
                        358 + (i - firstLine) * lineHeight
                    });

                    window.draw(typedText);
                }
            }

            // TIME
            string timeDisplay;

            if(timedMode)
            {
                int remaining =
                    60 - static_cast<int>(elapsedSeconds);

                if(remaining < 0)
                    remaining = 0;

                timeDisplay =
                    "TIME LEFT: " +
                    toString(remaining) +
                    " sec";
            }
            else
            {
                timeDisplay =
                    "TIME: " +
                    toString(elapsedSeconds) +
                    " sec";
            }

            sf::Text timeText(font, timeDisplay, 22);
            timeText.setFillColor(sf::Color::White);
            timeText.setPosition({80, 460});

            window.draw(timeText);

            // WPM
            sf::Text wpmText(
                font,
                "WPM: " + toString(statistics.getWPM()),
                22
            );

            wpmText.setFillColor(sf::Color::White);
            wpmText.setPosition({300, 460});
            window.draw(wpmText);

            // ACCURACY
            sf::Text accuracyText(
                font,
                "Accuracy: " +
                toString(statistics.getAccuracy()) +
                "%",
                22
            );

            accuracyText.setFillColor(sf::Color::White);
            accuracyText.setPosition({520, 460});
            window.draw(accuracyText);

            // CORRECT
            sf::Text correctText(
                font,
                "Correct: " +
                toString(statistics.getCorrect()),
                20
            );

            correctText.setFillColor(sf::Color::Green);
            correctText.setPosition({80, 510});
            window.draw(correctText);

            // WRONG
            sf::Text wrongText(
                font,
                "Wrong: " +
                toString(statistics.getWrong()),
                20
            );

            wrongText.setFillColor(sf::Color::Red);
            wrongText.setPosition({250, 510});
            window.draw(wrongText);

            // PRACTICE BUTTON
            sf::RectangleShape practiceButton(
                sf::Vector2f(150, 60)
            );

            practiceButton.setPosition({100, 570});
            practiceButton.setFillColor(
                sf::Color(70, 130, 180)
            );

            window.draw(practiceButton);

            sf::Text practiceText(
                font,
                "Practice",
                18
            );

            practiceText.setFillColor(sf::Color::White);
            practiceText.setPosition({125, 588});
            window.draw(practiceText);

            // TIMED BUTTON
            sf::RectangleShape timedButton(
                sf::Vector2f(150, 60)
            );

            timedButton.setPosition({280, 570});
            timedButton.setFillColor(
                sf::Color(70, 130, 180)
            );

            window.draw(timedButton);

            sf::Text timedText(
                font,
                "Timed",
                18
            );

            timedText.setFillColor(sf::Color::White);
            timedText.setPosition({325, 588});
            window.draw(timedText);

            // NEXT BUTTON
            sf::RectangleShape nextButton(
                sf::Vector2f(150, 60)
            );

            nextButton.setPosition({460, 570});
            nextButton.setFillColor(
                sf::Color(70, 130, 180)
            );

            window.draw(nextButton);

            sf::Text nextText(
                font,
                "Next",
                18
            );

            nextText.setFillColor(sf::Color::White);
            nextText.setPosition({510, 588});
            window.draw(nextText);

            // SAVE BUTTON
            sf::RectangleShape saveButton(
                sf::Vector2f(160, 60)
            );

            saveButton.setPosition({640, 570});
            saveButton.setFillColor(
                sf::Color(70, 130, 180)
            );

            window.draw(saveButton);

            sf::Text saveText(
                font,
                "Save Result",
                18
            );

            saveText.setFillColor(sf::Color::White);
            saveText.setPosition({665, 588});
            window.draw(saveText);

            // COMPLETION MESSAGE
            if(finished)
            {
                sf::Text resultText(
                    font,
                    "TEST COMPLETED! Result saved.",
                    25
                );

                resultText.setFillColor(sf::Color::Yellow);
                resultText.setPosition({300, 650});

                window.draw(resultText);
            }

            window.display();
        }
    }
    catch(exception &e)
    {
        cout << e.what() << endl;
    }
    return 0;
}