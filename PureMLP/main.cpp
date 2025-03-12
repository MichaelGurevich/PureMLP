
#include "utils.h"
#include "DataLoader.h"
#include "MLP.h"



#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath> // for sqrt

int main()
{
    // Canvas and window dimensions
    const int canvasSize = 600;           // Drawing area size (600x600)
    const int buttonAreaHeight = 50;      // Extra area for buttons
    const int windowWidth = canvasSize;
    const int windowHeight = canvasSize + buttonAreaHeight;

    // Button dimensions and gap
    const int buttonWidth = 100;
    const int buttonHeight = 30;
    const int buttonGap = 20;
    // Calculate starting X so that the two buttons (total width = 2*buttonWidth + buttonGap) are centered.
    const int totalButtonsWidth = 2 * buttonWidth + buttonGap;
    const int startX = (windowWidth - totalButtonsWidth) / 2;
    const int buttonY = canvasSize + (buttonAreaHeight - buttonHeight) / 2;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Digit Recognizer Canvas");
    window.setFramerateLimit(60);

    // Create a render texture for the drawing canvas
    sf::RenderTexture canvasTexture;
    if (!canvasTexture.create(canvasSize, canvasSize)) {
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

    // Load a font (ensure "arial.ttf" is in your working directory or use another font)
    sf::Font font;
    bool fontLoaded = font.loadFromFile("arial.ttf");
    if (!fontLoaded) {
        std::cerr << "Warning: Could not load 'arial.ttf'. Button texts will not be displayed." << std::endl;
    }

    // Setup button texts if font loaded
    sf::Text clearText, doneText;
    if (fontLoaded) {
        // Clear button text
        clearText.setFont(font);
        clearText.setString("Clear");
        clearText.setCharacterSize(20);
        clearText.setFillColor(sf::Color::Black);
        sf::FloatRect clearRect = clearText.getLocalBounds();
        clearText.setOrigin(clearRect.left + clearRect.width / 2, clearRect.top + clearRect.height / 2);
        clearText.setPosition(clearButton.getPosition().x + clearButton.getSize().x / 2,
            clearButton.getPosition().y + clearButton.getSize().y / 2);

        // Done button text
        doneText.setFont(font);
        doneText.setString("Done");
        doneText.setCharacterSize(20);
        doneText.setFillColor(sf::Color::Black);
        sf::FloatRect doneRect = doneText.getLocalBounds();
        doneText.setOrigin(doneRect.left + doneRect.width / 2, doneRect.top + doneRect.height / 2);
        doneText.setPosition(doneButton.getPosition().x + doneButton.getSize().x / 2,
            doneButton.getPosition().y + doneButton.getSize().y / 2);
    }

    bool isDrawing = false;
    // Variables for continuous line drawing
    sf::Vector2i lastPos;
    bool firstPoint = true;

    // Sprite to display the canvas texture
    sf::Sprite canvasSprite(canvasTexture.getTexture());

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle mouse button pressed
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                // If click is inside the canvas area, start drawing
                if (mouseY < canvasSize) {
                    isDrawing = true;
                    firstPoint = true; // start a new stroke
                }
                // If click is in the button area, check which button is pressed
                else {
                    // Check if Clear button is clicked
                    if (clearButton.getGlobalBounds().contains(mouseX, mouseY)) {
                        // Clear the canvas: fill with black and update texture
                        canvasTexture.clear(sf::Color::Black);
                        canvasTexture.display();
                    }
                    // Check if Done button is clicked
                    else if (doneButton.getGlobalBounds().contains(mouseX, mouseY)) {
                        // Scale down the canvas to 28x28 pixels using a RenderTexture
                        sf::RenderTexture scaledTexture;
                        if (!scaledTexture.create(28, 28)) {
                            std::cerr << "Error creating scaled render texture!" << std::endl;
                            return -1;
                        }
                        sf::Sprite canvasForScaling(canvasTexture.getTexture());
                        // Calculate scale factor from canvas (600) to 28
                        float scaleFactor = 28.f / canvasSize;
                        canvasForScaling.setScale(scaleFactor, scaleFactor);
                        scaledTexture.clear(sf::Color::Black);
                        scaledTexture.draw(canvasForScaling);
                        scaledTexture.display();

                        // Retrieve the scaled image and convert it to a 784-element vector
                        sf::Image scaledImage = scaledTexture.getTexture().copyToImage();
                        std::vector<int> digitVector;
                        digitVector.reserve(28 * 28);
                        for (unsigned int y = 0; y < 28; y++) {
                            for (unsigned int x = 0; x < 28; x++) {
                                sf::Color color = scaledImage.getPixel(x, y);
                                // Use the red channel (grayscale) for intensity
                                int intensity = color.r;
                                digitVector.push_back(intensity);
                            }
                        }

                        // Print the 784-element vector (formatted as 28 rows)
                        std::cout << "Digit vector (784 values):" << std::endl;
                        for (size_t i = 0; i < digitVector.size(); i++) {
                            std::cout << digitVector[i] << " ";
                            if ((i + 1) % 28 == 0)
                                std::cout << std::endl;
                        }
                    }
                }
            }

            // Stop drawing on mouse button release
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            {
                isDrawing = false;
                firstPoint = true; // reset for next stroke
            }
        }

        // Draw continuously while the left mouse button is held down on the canvas
        if (isDrawing)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (mousePos.y < canvasSize) {
                if (firstPoint) {
                    lastPos = mousePos;
                    firstPoint = false;
                }
                else {
                    // Compute differences between current and previous positions
                    float dx = mousePos.x - lastPos.x;
                    float dy = mousePos.y - lastPos.y;
                    float distance = std::sqrt(dx * dx + dy * dy);

                    // Determine the number of steps to interpolate along the line
                    int steps = static_cast<int>(distance);
                    if (steps == 0)
                        steps = 1; // avoid division by zero

                    // Draw intermediate circles along the line to create a continuous stroke
                    for (int i = 0; i <= steps; i++) {
                        float t = i / static_cast<float>(steps);
                        int x = static_cast<int>(lastPos.x + t * dx);
                        int y = static_cast<int>(lastPos.y + t * dy);

                        sf::CircleShape brush(10); // Brush radius
                        // Semi-transparent white brush to gradually build intensity
                        brush.setFillColor(sf::Color(255, 255, 255, 50));
                        brush.setPosition(x - 10, y - 10);
                        canvasTexture.draw(brush, sf::BlendAlpha);
                    }
                    lastPos = mousePos;
                }
                canvasTexture.display();
            }
        }

        // Update the canvas sprite and render everything
        canvasSprite.setTexture(canvasTexture.getTexture());
        window.clear(sf::Color::Black);
        window.draw(canvasSprite);
        window.draw(clearButton);
        window.draw(doneButton);
        if (fontLoaded) {
            window.draw(clearText);
            window.draw(doneText);
        }
        window.display();
    }

    return 0;
}



/*int main() {
   initSeed();

  
    MLP model (784, 40, 10);

    DataLoader dl("mnist_data.txt", "mnist_labels.txt");
   

    std::vector<pair<Matrix, Matrix>> splits = dl.trainValidTestSplit(55000,  5000, 100);



    model.fit(splits[0].first, splits[0].second, splits[1].first, splits[1].second, splits[2].first, splits[2].second);
    
    std::cout << "\n\n";
    
    
   
    return 0;
}*/
