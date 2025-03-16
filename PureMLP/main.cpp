
#include "utils.h"
#include "DataLoader.h"
#include "MLP.h"
#include <iostream>


#include <SFML/Graphics.hpp>

#include <vector>
#include <cmath>
#include <algorithm>

// Create a brush texture with a hard center and linear fall-off.
// The inner 70% of the radius is full white, then linearly fades to transparent.
sf::Texture createBrushTexture(unsigned int diameter)
{
    sf::Image brushImage;
    brushImage.create(diameter, diameter, sf::Color::Transparent);
    float radius = diameter / 2.0f;
    float innerRadius = radius * 0.7f; // full intensity within 70% of radius
    sf::Vector2f center(radius, radius);
    for (unsigned int x = 0; x < diameter; ++x)
    {
        for (unsigned int y = 0; y < diameter; ++y)
        {
            float dx = x - center.x;
            float dy = y - center.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            sf::Uint8 alpha = 0;
            if (distance <= innerRadius)
            {
                alpha = 255;
            }
            else if (distance <= radius)
            {
                float factor = 1.0f - ((distance - innerRadius) / (radius - innerRadius));
                alpha = static_cast<sf::Uint8>(255 * factor);
            }
            else
            {
                alpha = 0;
            }
            brushImage.setPixel(x, y, sf::Color(255, 255, 255, alpha));
        }
    }
    sf::Texture brushTexture;
    if (!brushTexture.loadFromImage(brushImage))
    {
        std::cerr << "Failed to load brush texture from image!" << std::endl;
    }
    return brushTexture;
}

int main()
{
    MLP model(784, 50, 50, 10, true);
    model.initFromFile();

    // Canvas and window dimensions
    const int canvasSize = 400;           // Drawing area (600x600)
    const int buttonAreaHeight = 50;      // Extra area for buttons
    const int windowWidth = canvasSize;
    const int windowHeight = canvasSize + buttonAreaHeight;

    // Button dimensions and layout
    const int buttonWidth = 100;
    const int buttonHeight = 30;
    const int buttonGap = 20;
    const int totalButtonsWidth = 2 * buttonWidth + buttonGap;
    const int startX = (windowWidth - totalButtonsWidth) / 2;
    const int buttonY = canvasSize + (buttonAreaHeight - buttonHeight) / 2;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Digit Recognizer Canvas");
    window.setFramerateLimit(60);

    // Create a render texture for the drawing canvas and clear to black
    sf::RenderTexture canvasTexture;
    if (!canvasTexture.create(canvasSize, canvasSize))
    {
        std::cerr << "Error creating canvas render texture!" << std::endl;
        return -1;
    }
    canvasTexture.clear(sf::Color::Black);
    canvasTexture.display();

    // Setup the "Clear" button
    sf::RectangleShape clearButton(sf::Vector2f(buttonWidth, buttonHeight));
    clearButton.setFillColor(sf::Color(200, 200, 200)); // light gray
    clearButton.setPosition(startX, buttonY);

    // Setup the "Done" button
    sf::RectangleShape doneButton(sf::Vector2f(buttonWidth, buttonHeight));
    doneButton.setFillColor(sf::Color(200, 200, 200)); // light gray
    doneButton.setPosition(startX + buttonWidth + buttonGap, buttonY);

    // Load a font (ensure "arial.ttf" is available or change the filename)
    sf::Font font;
    bool fontLoaded = font.loadFromFile("arial.ttf");
    if (!fontLoaded)
    {
        std::cerr << "Warning: Could not load 'arial.ttf'. Button texts will not be displayed." << std::endl;
    }

    // Setup button texts if font loaded
    sf::Text clearText, doneText;
    if (fontLoaded)
    {
        clearText.setFont(font);
        clearText.setString("Clear");
        clearText.setCharacterSize(20);
        clearText.setFillColor(sf::Color::Black);
        sf::FloatRect clearRect = clearText.getLocalBounds();
        clearText.setOrigin(clearRect.left + clearRect.width / 2, clearRect.top + clearRect.height / 2);
        clearText.setPosition(clearButton.getPosition().x + clearButton.getSize().x / 2,
            clearButton.getPosition().y + clearButton.getSize().y / 2);

        doneText.setFont(font);
        doneText.setString("Done");
        doneText.setCharacterSize(20);
        doneText.setFillColor(sf::Color::Black);
        sf::FloatRect doneRect = doneText.getLocalBounds();
        doneText.setOrigin(doneRect.left + doneRect.width / 2, doneRect.top + doneRect.height / 2);
        doneText.setPosition(doneButton.getPosition().x + doneButton.getSize().x / 2,
            doneButton.getPosition().y + doneButton.getSize().y / 2);
    }

    // Create a brush texture with a hard center.
    const unsigned int brushDiameter = 56; // increased brush size for thicker strokes
    sf::Texture brushTexture = createBrushTexture(brushDiameter);
    sf::Sprite brushSprite(brushTexture);
    // Center the brush sprite on its texture.
    brushSprite.setOrigin(brushDiameter / 2.0f, brushDiameter / 2.0f);

    bool isDrawing = false;
    sf::Vector2i lastPos;
    bool firstPoint = true;

    // Sprite for displaying the canvas texture
    sf::Sprite canvasSprite(canvasTexture.getTexture());

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle mouse button press events
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                if (mouseY < canvasSize) // Inside drawing area: start drawing
                {
                    isDrawing = true;
                    firstPoint = true;
                }
                else // Check if a button is pressed
                {
                    if (clearButton.getGlobalBounds().contains(mouseX, mouseY))
                    {
                        // Clear the canvas to black
                        canvasTexture.clear(sf::Color::Black);
                        canvasTexture.display();
                    }
                    else if (doneButton.getGlobalBounds().contains(mouseX, mouseY))
                    {
                        // Scale the canvas down to 28x28 pixels
                        sf::RenderTexture scaledTexture;
                        if (!scaledTexture.create(28, 28))
                        {
                            std::cerr << "Error creating scaled render texture!" << std::endl;
                            return -1;
                        }
                        sf::Sprite canvasForScaling(canvasTexture.getTexture());
                        float scaleFactor = 28.f / canvasSize;
                        canvasForScaling.setScale(scaleFactor, scaleFactor);
                        scaledTexture.clear(sf::Color::Black);
                        scaledTexture.draw(canvasForScaling);
                        scaledTexture.display();

                        // Convert the scaled image into a 784-element vector (using red channel)
                        sf::Image scaledImage = scaledTexture.getTexture().copyToImage();
                        std::vector<int> digitVector;
                        digitVector.reserve(28 * 28);
                        for (unsigned int y = 0; y < 28; y++)
                        {
                            for (unsigned int x = 0; x < 28; x++)
                            {
                                sf::Color color = scaledImage.getPixel(x, y);
                                int intensity = color.r;
                                digitVector.push_back(intensity);
                            }
                        }

                        int prediction = model.predict(digitVector);
                        std::cout << prediction << std::endl;

                    }
                }
            }

            // Stop drawing when the mouse button is released
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDrawing = false;
                firstPoint = true;
            }
        }

        // If drawing is active, interpolate between positions and draw the brush sprite.
        // We use a fixed step spacing of brushDiameter/4 to ensure heavy overlap.
        if (isDrawing)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (mousePos.y < canvasSize)
            {
                if (firstPoint)
                {
                    lastPos = mousePos;
                    firstPoint = false;
                }
                else
                {
                    float dx = mousePos.x - lastPos.x;
                    float dy = mousePos.y - lastPos.y;
                    float distance = std::sqrt(dx * dx + dy * dy);
                    // Use a fixed spacing (e.g., brushDiameter/4) for interpolation.
                    int steps = std::max(1, static_cast<int>(distance / (brushDiameter / 4.0f)));
                    for (int i = 0; i <= steps; i++)
                    {
                        float t = i / static_cast<float>(steps);
                        int x = static_cast<int>(lastPos.x + t * dx);
                        int y = static_cast<int>(lastPos.y + t * dy);
                        brushSprite.setPosition(static_cast<float>(x), static_cast<float>(y));
                        // Draw with additive blending so overlapping stamps accumulate intensity.
                        canvasTexture.draw(brushSprite, sf::BlendAdd);
                    }
                    lastPos = mousePos;
                }
                canvasTexture.display();
            }
        }

        // Render the updated canvas and UI
        canvasSprite.setTexture(canvasTexture.getTexture());
        window.clear(sf::Color::Black);
        window.draw(canvasSprite);
        window.draw(clearButton);
        window.draw(doneButton);
        if (fontLoaded)
        {
            window.draw(clearText);
            window.draw(doneText);
        }
        window.display();
    }

    return 0;
}


/*void print28x28(const double* arr);

void print28x28(const double* arr) {
    for (int row = 0; row < 28; ++row) {
        for (int col = 0; col < 28; ++col) {
            int index = row * 28 + col;

            std::cout << arr[index] << " ";
        }
        std::cout << std::endl;
    }
}
*/


/*int main() {
   

    initSeed();

    MLP model(784, 50, 50, 10, false);
    //model.initFromFile();

    


    DataLoader dl("mnist_data.txt", "mnist_labels.txt");
    std::cout << "Finished reading data!" << "\n";
    //print28x28(dl.getData()[0]);
    

    std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(64832,  5120, 0);
    //std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(128,  128, 100);

    model.fit(splits[0].first, splits[0].second, splits[1].first, splits[1].second, splits[2].first, splits[2].second, 50);
    
    std::cout << "\n\n";
    
    
   
    return 0;
} */



/*std::cout << "Digit vector (784 values):" << std::endl;
for (size_t i = 0; i < digitVector.size(); i++)
{
    std::cout << digitVector[i] << " ";
    if ((i + 1) % 28 == 0)
        std::cout << std::endl;
}*/