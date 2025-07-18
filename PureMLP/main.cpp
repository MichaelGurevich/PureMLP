#include "utils.h"
#include "DataLoader.h"
#include "MLP.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <vector>
#include <cmath>
#include <algorithm>

struct LogInfo {
    double mse;
    double trainAcc;
    double validAcc;
};

static LogInfo readLog()
{
    LogInfo info{0.0,0.0,0.0};
    std::ifstream file("log.txt");
    if(file.is_open())
    {
        std::string line;
        std::getline(file,line);
        std::sscanf(line.c_str(),
            "Train MSE: %lf | Train accuracy: %lf | Valid accuracy: %lf",
            &info.mse,&info.trainAcc,&info.validAcc);
    }
    return info;
}

static bool confirmTraining(const sf::Font& font)
{
    sf::RenderWindow dialog(sf::VideoMode(300,150), "Confirm Training",
                            sf::Style::Titlebar | sf::Style::Close);
    sf::RectangleShape yes(sf::Vector2f(80,30));
    sf::RectangleShape no(sf::Vector2f(80,30));
    yes.setFillColor(sf::Color(200,200,200));
    no.setFillColor(sf::Color(200,200,200));
    yes.setPosition(40,90);
    no.setPosition(180,90);

    sf::Text yesText, noText, msg;
    if(font.getInfo().family != "")
    {
        yesText.setFont(font);
        yesText.setString("Yes");
        yesText.setCharacterSize(18);
        yesText.setFillColor(sf::Color::Black);
        sf::FloatRect r = yesText.getLocalBounds();
        yesText.setOrigin(r.left + r.width/2, r.top + r.height/2);
        yesText.setPosition(yes.getPosition().x + yes.getSize().x/2,
                            yes.getPosition().y + yes.getSize().y/2);

        noText.setFont(font);
        noText.setString("No");
        noText.setCharacterSize(18);
        noText.setFillColor(sf::Color::Black);
        r = noText.getLocalBounds();
        noText.setOrigin(r.left + r.width/2, r.top + r.height/2);
        noText.setPosition(no.getPosition().x + no.getSize().x/2,
                           no.getPosition().y + no.getSize().y/2);

        msg.setFont(font);
        msg.setString("Start training?");
        msg.setCharacterSize(20);
        msg.setFillColor(sf::Color::White);
        r = msg.getLocalBounds();
        msg.setOrigin(r.left + r.width/2, r.top + r.height/2);
        msg.setPosition(150,40);
    }

    bool result = false;
    while(dialog.isOpen())
    {
        sf::Event e;
        while(dialog.pollEvent(e))
        {
            if(e.type == sf::Event::Closed)
                dialog.close();
            if(e.type == sf::Event::MouseButtonPressed &&
               e.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f pos(static_cast<float>(e.mouseButton.x),
                                 static_cast<float>(e.mouseButton.y));
                if(yes.getGlobalBounds().contains(pos))
                {
                    result = true;
                    dialog.close();
                }
                else if(no.getGlobalBounds().contains(pos))
                {
                    dialog.close();
                }
            }
        }
        dialog.clear();
        dialog.draw(yes);
        dialog.draw(no);
        if(font.getInfo().family != "")
        {
            dialog.draw(yesText);
            dialog.draw(noText);
            dialog.draw(msg);
        }
        dialog.display();
    }
    return result;
}

static void runTrainingMode()
{
    initSeed();
    MLP model(784,128,64,10,false);
    DataLoader dl("mnist_data.txt","mnist_labels.txt");
    std::cout << "Finished reading data!" << std::endl;
    std::vector<std::pair<Matrix,Matrix>> splits =
        dl.trainValidTestSplit(64832,5120,0);
    model.fit(splits[0].first, splits[0].second,
              splits[1].first, splits[1].second,
              splits[2].first, splits[2].second,
              30, 0.01);
}

static sf::Texture createBrushTexture(unsigned int diameter)
{
    sf::Image brushImage;
    brushImage.create(diameter, diameter, sf::Color::Transparent);
    float radius = diameter / 2.0f;
    float innerRadius = radius * 0.7f;
    sf::Vector2f center(radius, radius);
    for(unsigned int x=0; x<diameter; ++x)
        for(unsigned int y=0; y<diameter; ++y)
        {
            float dx = x - center.x;
            float dy = y - center.y;
            float distance = std::sqrt(dx*dx + dy*dy);
            sf::Uint8 alpha = 0;
            if(distance <= innerRadius)
                alpha = 255;
            else if(distance <= radius)
            {
                float factor = 1.f - ((distance - innerRadius)/(radius-innerRadius));
                alpha = static_cast<sf::Uint8>(255 * factor);
            }
            brushImage.setPixel(x,y,sf::Color(255,255,255,alpha));
        }
    sf::Texture tex;
    tex.loadFromImage(brushImage);
    return tex;
}

static void runDrawingMode()
{
    MLP model(784,128,64,10,true);
    model.initFromFile();

    const int canvasSize = 500;
    const int buttonAreaHeight = 50;
    const int windowWidth = canvasSize;
    const int windowHeight = canvasSize + buttonAreaHeight;

    const int buttonWidth = 100;
    const int buttonHeight = 30;
    const int buttonGap = 20;
    const int totalButtonsWidth = 2*buttonWidth + buttonGap;
    const int startX = (windowWidth - totalButtonsWidth)/2;
    const int buttonY = canvasSize + (buttonAreaHeight - buttonHeight)/2;

    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight),
                            "Digit Recognizer Canvas");
    window.setFramerateLimit(60);

    sf::RenderTexture canvasTexture;
    if(!canvasTexture.create(canvasSize, canvasSize))
        return;
    canvasTexture.clear(sf::Color::Black);
    canvasTexture.display();

    sf::RectangleShape clearButton(sf::Vector2f(buttonWidth, buttonHeight));
    clearButton.setFillColor(sf::Color(200,200,200));
    clearButton.setPosition(startX, buttonY);

    sf::RectangleShape doneButton(sf::Vector2f(buttonWidth, buttonHeight));
    doneButton.setFillColor(sf::Color(200,200,200));
    doneButton.setPosition(startX + buttonWidth + buttonGap, buttonY);

    sf::Font font;
    bool fontLoaded = font.loadFromFile("arial.ttf");

    sf::Text clearText, doneText;
    if(fontLoaded)
    {
        clearText.setFont(font);
        clearText.setString("Clear");
        clearText.setCharacterSize(20);
        clearText.setFillColor(sf::Color::Black);
        sf::FloatRect r = clearText.getLocalBounds();
        clearText.setOrigin(r.left + r.width/2, r.top + r.height/2);
        clearText.setPosition(clearButton.getPosition().x + clearButton.getSize().x/2,
                              clearButton.getPosition().y + clearButton.getSize().y/2);

        doneText.setFont(font);
        doneText.setString("Done");
        doneText.setCharacterSize(20);
        doneText.setFillColor(sf::Color::Black);
        r = doneText.getLocalBounds();
        doneText.setOrigin(r.left + r.width/2, r.top + r.height/2);
        doneText.setPosition(doneButton.getPosition().x + doneButton.getSize().x/2,
                             doneButton.getPosition().y + doneButton.getSize().y/2);
    }

    const unsigned int brushDiameter = 56;
    sf::Texture brushTexture = createBrushTexture(brushDiameter);
    sf::Sprite brushSprite(brushTexture);
    brushSprite.setOrigin(brushDiameter/2.f, brushDiameter/2.f);

    bool isDrawing = false;
    sf::Vector2i lastPos;
    bool firstPoint = true;
    sf::Sprite canvasSprite(canvasTexture.getTexture());

    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();

            if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                if(mouseY < canvasSize)
                {
                    isDrawing = true;
                    firstPoint = true;
                }
                else
                {
                    if(clearButton.getGlobalBounds().contains(mouseX, mouseY))
                    {
                        canvasTexture.clear(sf::Color::Black);
                        canvasTexture.display();
                    }
                    else if(doneButton.getGlobalBounds().contains(mouseX, mouseY))
                    {
                        sf::RenderTexture scaledTexture;
                        if(!scaledTexture.create(28,28))
                            return;
                        sf::Sprite canvasForScaling(canvasTexture.getTexture());
                        float scaleFactor = 28.f / canvasSize;
                        canvasForScaling.setScale(scaleFactor, scaleFactor);
                        scaledTexture.clear(sf::Color::Black);
                        scaledTexture.draw(canvasForScaling);
                        scaledTexture.display();

                        sf::Image scaledImage = scaledTexture.getTexture().copyToImage();
                        std::vector<int> digitVector;
                        digitVector.reserve(28*28);
                        for(unsigned int y=0; y<28; ++y)
                            for(unsigned int x=0; x<28; ++x)
                                digitVector.push_back(scaledImage.getPixel(x,y).r);
                        int prediction = model.predict(digitVector);
                        std::cout << prediction << std::endl;
                    }
                }
            }

            if(event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDrawing = false;
                firstPoint = true;
            }
        }

        if(isDrawing)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if(mousePos.y < canvasSize)
            {
                if(firstPoint)
                {
                    lastPos = mousePos;
                    firstPoint = false;
                }
                else
                {
                    float dx = mousePos.x - lastPos.x;
                    float dy = mousePos.y - lastPos.y;
                    float distance = std::sqrt(dx*dx + dy*dy);
                    int steps = std::max(1, static_cast<int>(distance/(brushDiameter/4.f)));
                    for(int i=0;i<=steps;i++)
                    {
                        float t = i/static_cast<float>(steps);
                        int x = static_cast<int>(lastPos.x + t*dx);
                        int y = static_cast<int>(lastPos.y + t*dy);
                        brushSprite.setPosition(static_cast<float>(x), static_cast<float>(y));
                        canvasTexture.draw(brushSprite, sf::BlendAdd);
                    }
                    lastPos = mousePos;
                }
                canvasTexture.display();
            }
        }

        canvasSprite.setTexture(canvasTexture.getTexture());
        window.clear(sf::Color::Black);
        window.draw(canvasSprite);
        window.draw(clearButton);
        window.draw(doneButton);
        if(fontLoaded)
        {
            window.draw(clearText);
            window.draw(doneText);
        }
        window.display();
    }
}

static void runMainMenu()
{
    LogInfo info = readLog();
    sf::Font font;
    font.loadFromFile("arial.ttf");

    while(true)
    {
        sf::RenderWindow window(sf::VideoMode(400,200), "Main Menu");

        sf::RectangleShape trainBtn(sf::Vector2f(120,40));
        sf::RectangleShape drawBtn(sf::Vector2f(120,40));
        trainBtn.setFillColor(sf::Color(200,200,200));
        drawBtn.setFillColor(sf::Color(200,200,200));
        trainBtn.setPosition(50,120);
        drawBtn.setPosition(230,120);

        sf::Text trainText, drawText, infoText;
        if(font.getInfo().family != "")
        {
            trainText.setFont(font);
            trainText.setString("Train");
            trainText.setCharacterSize(20);
            trainText.setFillColor(sf::Color::Black);
            sf::FloatRect r = trainText.getLocalBounds();
            trainText.setOrigin(r.left + r.width/2, r.top + r.height/2);
            trainText.setPosition(trainBtn.getPosition().x + trainBtn.getSize().x/2,
                                 trainBtn.getPosition().y + trainBtn.getSize().y/2);

            drawText.setFont(font);
            drawText.setString("Draw");
            drawText.setCharacterSize(20);
            drawText.setFillColor(sf::Color::Black);
            r = drawText.getLocalBounds();
            drawText.setOrigin(r.left + r.width/2, r.top + r.height/2);
            drawText.setPosition(drawBtn.getPosition().x + drawBtn.getSize().x/2,
                                drawBtn.getPosition().y + drawBtn.getSize().y/2);

            std::ostringstream oss;
            oss << "Train MSE: " << info.mse
                << " | Train Acc: " << info.trainAcc
                << "% | Valid Acc: " << info.validAcc << "%";
            infoText.setFont(font);
            infoText.setString(oss.str());
            infoText.setCharacterSize(18);
            infoText.setFillColor(sf::Color::White);
            r = infoText.getLocalBounds();
            infoText.setOrigin(r.left + r.width/2, r.top + r.height/2);
            infoText.setPosition(200,60);
        }

        bool restartMenu = false;
        while(window.isOpen())
        {
            sf::Event event;
            while(window.pollEvent(event))
            {
                if(event.type == sf::Event::Closed)
                {
                    window.close();
                    return;
                }
                if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
                {
                    int mx = event.mouseButton.x;
                    int my = event.mouseButton.y;
                    if(trainBtn.getGlobalBounds().contains(mx,my))
                    {
                        if(confirmTraining(font))
                        {
                            window.close();
                            runTrainingMode();
                            info = readLog();
                            restartMenu = true;
                        }
                    }
                    else if(drawBtn.getGlobalBounds().contains(mx,my))
                    {
                        window.close();
                        runDrawingMode();
                        restartMenu = true;
                    }
                }
            }
            window.clear();
            window.draw(trainBtn);
            window.draw(drawBtn);
            if(font.getInfo().family != "")
            {
                window.draw(trainText);
                window.draw(drawText);
                window.draw(infoText);
            }
            window.display();
        }
        if(!restartMenu)
            break;
    }
}

int main()
{
    try
    {
        runMainMenu();
    }
    catch(const MatrixException& ex)
    {
        std::cerr << "Matrix exception: " << ex.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}

