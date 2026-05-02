#include "utils.h"
#include "DataLoader.h"
#include "MLP.h"
#include "MatrixException.h"

#include <SFML/Graphics.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

struct LogInfo
{
    double mse;
    double trainAcc;
    double validAcc;
};

static LogInfo readLog()
{
    LogInfo info{0.0, 0.0, 0.0};
    std::ifstream file("log.txt");
    if(file.is_open())
    {
        std::string line;
        std::getline(file, line);
        std::istringstream iss(line);
        std::string trainLabel;
        std::string mseLabel;
        std::string trainAccuracyLabel;
        std::string validLabel;
        std::string validAccuracyLabel;
        char separator1 = '\0';
        char separator2 = '\0';

        if(!(iss >> trainLabel >> mseLabel >> info.mse
                 >> separator1 >> trainLabel >> trainAccuracyLabel >> info.trainAcc
                 >> separator2 >> validLabel >> validAccuracyLabel >> info.validAcc)
           || separator1 != '|' || separator2 != '|')
        {
            info = LogInfo{0.0, 0.0, 0.0};
        }
    }
    return info;
}

static void setupWorkingDirectory()
{
    namespace fs = std::filesystem;

    const fs::path current = fs::current_path();
    const fs::path requiredAsset = "arial.ttf";

    if(fs::exists(current / requiredAsset))
        return;

    const std::vector<fs::path> candidates = {
        current / "PureMLP",
        current.parent_path(),
        current.parent_path().parent_path(),
        current.parent_path().parent_path() / "PureMLP"
    };

    for(const fs::path& candidate : candidates)
    {
        if(!candidate.empty() && fs::exists(candidate / requiredAsset))
        {
            fs::current_path(candidate);
            return;
        }
    }
}

static std::string formatNumber(double value, int precision)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

static sf::Vector2f toVector2f(const sf::Vector2i& point)
{
    return sf::Vector2f(static_cast<float>(point.x), static_cast<float>(point.y));
}

static sf::Texture createBrushTexture(unsigned int diameter)
{
    sf::Image brushImage;
    brushImage.create(diameter, diameter, sf::Color::Transparent);
    const float radius = diameter / 2.0f;
    const float innerRadius = radius * 0.68f;
    const sf::Vector2f center(radius, radius);

    for(unsigned int x = 0; x < diameter; ++x)
    {
        for(unsigned int y = 0; y < diameter; ++y)
        {
            const float dx = x - center.x;
            const float dy = y - center.y;
            const float distance = std::sqrt(dx * dx + dy * dy);
            sf::Uint8 alpha = 0;

            if(distance <= innerRadius)
                alpha = 255;
            else if(distance <= radius)
            {
                const float factor = 1.f - ((distance - innerRadius) / (radius - innerRadius));
                alpha = static_cast<sf::Uint8>(255 * factor);
            }

            brushImage.setPixel(x, y, sf::Color(255, 255, 255, alpha));
        }
    }

    sf::Texture texture;
    texture.loadFromImage(brushImage);
    return texture;
}

namespace Theme
{
    const sf::Color background(19, 24, 31);
    const sf::Color topBar(24, 31, 40);
    const sf::Color sidebar(21, 27, 36);
    const sf::Color panel(31, 39, 50);
    const sf::Color panelSoft(38, 48, 62);
    const sf::Color border(66, 79, 96);
    const sf::Color text(235, 240, 246);
    const sf::Color mutedText(155, 168, 184);
    const sf::Color accent(74, 196, 161);
    const sf::Color accentSoft(38, 108, 94);
    const sf::Color blue(81, 145, 245);
    const sf::Color danger(230, 91, 102);
    const sf::Color warning(235, 184, 86);
    const sf::Color canvas(4, 6, 8);
}

static sf::Text makeText(const sf::Font& font, const std::string& value,
                         unsigned int size, sf::Color color,
                         sf::Vector2f position)
{
    sf::Text text;
    text.setFont(font);
    text.setString(value);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(position);
    return text;
}

static void drawText(sf::RenderTarget& target, const sf::Font& font,
                     const std::string& value, unsigned int size,
                     sf::Color color, sf::Vector2f position)
{
    target.draw(makeText(font, value, size, color, position));
}

static void drawCenteredText(sf::RenderTarget& target, const sf::Font& font,
                             const std::string& value, unsigned int size,
                             sf::Color color, const sf::FloatRect& bounds)
{
    sf::Text text = makeText(font, value, size, color, sf::Vector2f());
    const sf::FloatRect local = text.getLocalBounds();
    text.setOrigin(local.left + local.width / 2.f, local.top + local.height / 2.f);
    text.setPosition(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    target.draw(text);
}

static void drawPanel(sf::RenderTarget& target, const sf::FloatRect& bounds,
                      sf::Color fill = Theme::panel,
                      sf::Color outline = Theme::border)
{
    sf::RectangleShape panel(sf::Vector2f(bounds.width, bounds.height));
    panel.setPosition(bounds.left, bounds.top);
    panel.setFillColor(fill);
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(outline);
    target.draw(panel);
}

static bool pointInRect(const sf::Event& event, const sf::FloatRect& bounds)
{
    if(event.type != sf::Event::MouseButtonPressed ||
       event.mouseButton.button != sf::Mouse::Left)
    {
        return false;
    }

    const sf::Vector2f point(static_cast<float>(event.mouseButton.x),
                             static_cast<float>(event.mouseButton.y));
    return bounds.contains(point);
}

static void drawButton(sf::RenderTarget& target, const sf::Font& font,
                       const sf::FloatRect& bounds, const std::string& label,
                       const sf::Vector2f& mouse, bool primary = false,
                       bool selected = false, bool enabled = true)
{
    sf::RectangleShape body(sf::Vector2f(bounds.width, bounds.height));
    body.setPosition(bounds.left, bounds.top);

    sf::Color fill = primary ? Theme::accentSoft : Theme::panelSoft;
    sf::Color outline = primary ? Theme::accent : Theme::border;
    sf::Color labelColor = Theme::text;

    if(selected)
    {
        fill = Theme::blue;
        outline = sf::Color(130, 180, 250);
    }
    else if(enabled && bounds.contains(mouse))
    {
        fill = primary ? sf::Color(45, 132, 113) : sf::Color(49, 61, 77);
        outline = primary ? sf::Color(98, 224, 188) : sf::Color(90, 106, 126);
    }

    if(!enabled)
    {
        fill = sf::Color(42, 48, 56);
        outline = sf::Color(55, 61, 70);
        labelColor = sf::Color(102, 112, 125);
    }

    body.setFillColor(fill);
    body.setOutlineColor(outline);
    body.setOutlineThickness(1.f);
    target.draw(body);
    drawCenteredText(target, font, label, 18, labelColor, bounds);
}

static void drawProgressBar(sf::RenderTarget& target, const sf::FloatRect& bounds,
                            float progress, sf::Color fill)
{
    progress = std::max(0.f, std::min(1.f, progress));

    sf::RectangleShape track(sf::Vector2f(bounds.width, bounds.height));
    track.setPosition(bounds.left, bounds.top);
    track.setFillColor(sf::Color(20, 25, 32));
    track.setOutlineThickness(1.f);
    track.setOutlineColor(Theme::border);
    target.draw(track);

    if(progress > 0.f)
    {
        sf::RectangleShape bar(sf::Vector2f(bounds.width * progress, bounds.height));
        bar.setPosition(bounds.left, bounds.top);
        bar.setFillColor(fill);
        target.draw(bar);
    }
}

class Slider
{
public:
    Slider(float minValue, float maxValue, float initialValue)
        : minValue(minValue), maxValue(maxValue), value(initialValue)
    {
    }

    void setBounds(const sf::FloatRect& newBounds)
    {
        bounds = newBounds;
    }

    bool handleEvent(const sf::Event& event)
    {
        if(event.type == sf::Event::MouseButtonPressed &&
           event.mouseButton.button == sf::Mouse::Left)
        {
            const sf::Vector2f point(static_cast<float>(event.mouseButton.x),
                                     static_cast<float>(event.mouseButton.y));
            if(bounds.contains(point))
            {
                dragging = true;
                setValueFromX(point.x);
                return true;
            }
        }

        if(event.type == sf::Event::MouseMoved && dragging)
        {
            setValueFromX(static_cast<float>(event.mouseMove.x));
            return true;
        }

        if(event.type == sf::Event::MouseButtonReleased &&
           event.mouseButton.button == sf::Mouse::Left)
        {
            dragging = false;
        }

        return false;
    }

    void draw(sf::RenderTarget& target, const sf::Font& font,
              const std::string& label) const
    {
        drawText(target, font, label, 16, Theme::mutedText,
                 sf::Vector2f(bounds.left, bounds.top - 26.f));

        const float trackY = bounds.top + bounds.height / 2.f - 3.f;
        sf::RectangleShape track(sf::Vector2f(bounds.width, 6.f));
        track.setPosition(bounds.left, trackY);
        track.setFillColor(sf::Color(21, 27, 34));
        target.draw(track);

        const float t = (value - minValue) / (maxValue - minValue);
        sf::RectangleShape fill(sf::Vector2f(bounds.width * t, 6.f));
        fill.setPosition(bounds.left, trackY);
        fill.setFillColor(Theme::accent);
        target.draw(fill);

        sf::CircleShape knob(9.f);
        knob.setOrigin(9.f, 9.f);
        knob.setPosition(bounds.left + bounds.width * t, bounds.top + bounds.height / 2.f);
        knob.setFillColor(Theme::text);
        knob.setOutlineThickness(2.f);
        knob.setOutlineColor(Theme::accent);
        target.draw(knob);

        drawText(target, font, std::to_string(static_cast<int>(std::round(value))),
                 16, Theme::text,
                 sf::Vector2f(bounds.left + bounds.width + 16.f, bounds.top - 1.f));
    }

    float getValue() const
    {
        return value;
    }

private:
    void setValueFromX(float x)
    {
        const float t = std::max(0.f, std::min(1.f, (x - bounds.left) / bounds.width));
        value = minValue + t * (maxValue - minValue);
    }

    sf::FloatRect bounds;
    float minValue = 0.f;
    float maxValue = 1.f;
    float value = 0.f;
    bool dragging = false;
};

class DigitCanvas
{
public:
    explicit DigitCanvas(unsigned int size)
        : size(size)
    {
        canvasTexture.create(size, size);
        clear();
        setBrushSize(44);
    }

    void setPosition(sf::Vector2f newPosition)
    {
        position = newPosition;
    }

    void setBrushSize(unsigned int diameter)
    {
        diameter = std::max(12u, std::min(76u, diameter));
        if(diameter == brushDiameter)
            return;

        brushDiameter = diameter;
        brushTexture = createBrushTexture(brushDiameter);
        brushSprite.setTexture(brushTexture, true);
        brushSprite.setOrigin(brushDiameter / 2.f, brushDiameter / 2.f);
    }

    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window)
    {
        bool strokeFinished = false;

        if(event.type == sf::Event::MouseButtonPressed &&
           event.mouseButton.button == sf::Mouse::Left)
        {
            const sf::Vector2f point(static_cast<float>(event.mouseButton.x),
                                     static_cast<float>(event.mouseButton.y));
            if(getBounds().contains(point))
            {
                pushUndo();
                isDrawing = true;
                firstPoint = true;
                drawAt(windowToCanvas(point));
                lastPos = sf::Vector2i(static_cast<int>(point.x), static_cast<int>(point.y));
            }
        }

        if(event.type == sf::Event::MouseButtonReleased &&
           event.mouseButton.button == sf::Mouse::Left)
        {
            if(isDrawing)
            {
                strokeFinished = true;
                dirty = true;
            }
            isDrawing = false;
            firstPoint = true;
        }

        return strokeFinished;
    }

    void update(const sf::RenderWindow& window)
    {
        if(!isDrawing)
            return;

        const sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
        const sf::Vector2f mousePoint = toVector2f(mousePosition);

        if(!getBounds().contains(mousePoint))
            return;

        if(firstPoint)
        {
            lastPos = mousePosition;
            firstPoint = false;
            return;
        }

        const float dx = static_cast<float>(mousePosition.x - lastPos.x);
        const float dy = static_cast<float>(mousePosition.y - lastPos.y);
        const float distance = std::sqrt(dx * dx + dy * dy);
        const int steps = std::max(1, static_cast<int>(distance / (brushDiameter / 4.f)));

        for(int i = 0; i <= steps; ++i)
        {
            const float t = i / static_cast<float>(steps);
            const float x = lastPos.x + t * dx;
            const float y = lastPos.y + t * dy;
            drawAt(windowToCanvas(sf::Vector2f(x, y)));
        }

        lastPos = mousePosition;
        canvasTexture.display();
    }

    void draw(sf::RenderTarget& target, const sf::RenderWindow& window) const
    {
        sf::RectangleShape backdrop(sf::Vector2f(static_cast<float>(size + 12),
                                                 static_cast<float>(size + 12)));
        backdrop.setPosition(position.x - 6.f, position.y - 6.f);
        backdrop.setFillColor(sf::Color(12, 16, 22));
        backdrop.setOutlineThickness(1.f);
        backdrop.setOutlineColor(Theme::border);
        target.draw(backdrop);

        sf::Sprite canvasSprite(canvasTexture.getTexture());
        canvasSprite.setPosition(position);
        target.draw(canvasSprite);

        sf::RectangleShape border(sf::Vector2f(static_cast<float>(size),
                                               static_cast<float>(size)));
        border.setPosition(position);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(1.f);
        border.setOutlineColor(sf::Color(82, 95, 112));
        target.draw(border);

        const sf::Vector2f mouse = toVector2f(sf::Mouse::getPosition(window));
        if(getBounds().contains(mouse))
        {
            sf::CircleShape cursor(brushDiameter / 2.f);
            cursor.setOrigin(brushDiameter / 2.f, brushDiameter / 2.f);
            cursor.setPosition(mouse);
            cursor.setFillColor(sf::Color::Transparent);
            cursor.setOutlineThickness(1.f);
            cursor.setOutlineColor(sf::Color(180, 220, 210, 130));
            target.draw(cursor);
        }
    }

    void clear()
    {
        canvasTexture.clear(Theme::canvas);
        canvasTexture.display();
        dirty = true;
    }

    void undo()
    {
        if(undoStack.empty())
            return;

        sf::Texture restoreTexture;
        restoreTexture.loadFromImage(undoStack.back());
        undoStack.pop_back();

        sf::Sprite restoreSprite(restoreTexture);
        canvasTexture.clear(Theme::canvas);
        canvasTexture.draw(restoreSprite);
        canvasTexture.display();
        dirty = true;
    }

    bool canUndo() const
    {
        return !undoStack.empty();
    }

    std::vector<int> toDigitVector() const
    {
        sf::RenderTexture scaledTexture;
        scaledTexture.create(28, 28);

        sf::Sprite canvasForScaling(canvasTexture.getTexture());
        canvasForScaling.setScale(28.f / size, 28.f / size);

        scaledTexture.clear(Theme::canvas);
        scaledTexture.draw(canvasForScaling);
        scaledTexture.display();

        const sf::Image scaledImage = scaledTexture.getTexture().copyToImage();
        std::vector<int> digitVector;
        digitVector.reserve(28 * 28);

        for(unsigned int y = 0; y < 28; ++y)
        {
            for(unsigned int x = 0; x < 28; ++x)
            {
                digitVector.push_back(scaledImage.getPixel(x, y).r);
            }
        }

        return digitVector;
    }

    sf::Texture makePreviewTexture() const
    {
        sf::RenderTexture scaledTexture;
        scaledTexture.create(28, 28);

        sf::Sprite canvasForScaling(canvasTexture.getTexture());
        canvasForScaling.setScale(28.f / size, 28.f / size);

        scaledTexture.clear(Theme::canvas);
        scaledTexture.draw(canvasForScaling);
        scaledTexture.display();

        sf::Texture preview;
        preview.loadFromImage(scaledTexture.getTexture().copyToImage());
        return preview;
    }

    bool consumeDirty()
    {
        const bool wasDirty = dirty;
        dirty = false;
        return wasDirty;
    }

    sf::FloatRect getBounds() const
    {
        return sf::FloatRect(position.x, position.y,
                             static_cast<float>(size), static_cast<float>(size));
    }

private:
    sf::Vector2f windowToCanvas(sf::Vector2f point) const
    {
        return sf::Vector2f(
            std::max(0.f, std::min(static_cast<float>(size - 1), point.x - position.x)),
            std::max(0.f, std::min(static_cast<float>(size - 1), point.y - position.y))
        );
    }

    void drawAt(sf::Vector2f canvasPoint)
    {
        brushSprite.setPosition(canvasPoint);
        canvasTexture.draw(brushSprite, sf::BlendAdd);
        canvasTexture.display();
    }

    void pushUndo()
    {
        undoStack.push_back(canvasTexture.getTexture().copyToImage());
        if(undoStack.size() > maxUndo)
            undoStack.erase(undoStack.begin());
    }

    unsigned int size;
    unsigned int brushDiameter = 44;
    sf::Vector2f position;
    sf::RenderTexture canvasTexture;
    sf::Texture brushTexture;
    sf::Sprite brushSprite;
    bool isDrawing = false;
    bool firstPoint = true;
    bool dirty = true;
    sf::Vector2i lastPos;
    std::vector<sf::Image> undoStack;
    static constexpr std::size_t maxUndo = 20;
};

struct TrainingState
{
    mutable std::mutex mutex;
    bool running = false;
    bool done = false;
    bool failed = false;
    bool consumed = false;
    std::string message = "Ready";
    MLP::TrainingProgress progress{0, 30, 0.0, 0.0, 0.0};
};

class App
{
public:
    App()
        : window(sf::VideoMode(windowWidth, windowHeight), "PureMLP"),
          canvas(420),
          brushSlider(12.f, 76.f, 44.f),
          trainingState(std::make_shared<TrainingState>())
    {
        window.setFramerateLimit(60);
        fontLoaded = font.loadFromFile("arial.ttf");
        logInfo = readLog();
        loadModel();
    }

    ~App()
    {
        if(trainingThread.joinable())
        {
            std::lock_guard<std::mutex> lock(trainingState->mutex);
            if(trainingState->running)
                trainingThread.detach();
            else
                trainingThread.join();
        }
    }

    void run()
    {
        while(window.isOpen())
        {
            processEvents();
            update();
            draw();
        }
    }

private:
    enum class Screen
    {
        Draw,
        Train,
        Model
    };

    void processEvents()
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();

            handleNavigation(event);

            switch(currentScreen)
            {
            case Screen::Draw:
                handleDrawEvents(event);
                break;
            case Screen::Train:
                handleTrainEvents(event);
                break;
            case Screen::Model:
                handleModelEvents(event);
                break;
            }
        }
    }

    void handleNavigation(const sf::Event& event)
    {
        if(pointInRect(event, sf::FloatRect(20.f, 104.f, 140.f, 42.f)))
            currentScreen = Screen::Draw;
        else if(pointInRect(event, sf::FloatRect(20.f, 154.f, 140.f, 42.f)))
            currentScreen = Screen::Train;
        else if(pointInRect(event, sf::FloatRect(20.f, 204.f, 140.f, 42.f)))
            currentScreen = Screen::Model;
    }

    void handleDrawEvents(const sf::Event& event)
    {
        if(pointInRect(event, sf::FloatRect(218.f, 594.f, 112.f, 42.f)))
        {
            canvas.clear();
            hasPrediction = false;
            statusText = "Canvas cleared";
        }
        else if(pointInRect(event, sf::FloatRect(342.f, 594.f, 112.f, 42.f)))
        {
            canvas.undo();
            hasPrediction = false;
            statusText = "Undo applied";
        }
        else if(pointInRect(event, sf::FloatRect(466.f, 594.f, 126.f, 42.f)))
        {
            runPrediction();
        }
        else if(pointInRect(event, sf::FloatRect(604.f, 594.f, 126.f, 42.f)))
        {
            autoPredict = !autoPredict;
            statusText = autoPredict ? "Auto predict enabled" : "Auto predict disabled";
        }

        if(brushSlider.handleEvent(event))
            canvas.setBrushSize(static_cast<unsigned int>(std::round(brushSlider.getValue())));

        const bool strokeFinished = canvas.handleEvent(event, window);
        if(strokeFinished && autoPredict)
            runPrediction();
    }

    void handleTrainEvents(const sf::Event& event)
    {
        const bool dataAvailable = std::filesystem::exists("mnist_data.txt") &&
                                   std::filesystem::exists("mnist_labels.txt");
        const bool running = isTrainingRunning();

        if(pointInRect(event, sf::FloatRect(238.f, 212.f, 42.f, 36.f)))
            trainEpochs = std::max(1, trainEpochs - 1);
        else if(pointInRect(event, sf::FloatRect(360.f, 212.f, 42.f, 36.f)))
            trainEpochs = std::min(100, trainEpochs + 1);
        else if(pointInRect(event, sf::FloatRect(238.f, 296.f, 42.f, 36.f)))
            learningRate = std::max(0.0001, learningRate / 2.0);
        else if(pointInRect(event, sf::FloatRect(360.f, 296.f, 42.f, 36.f)))
            learningRate = std::min(1.0, learningRate * 2.0);
        else if(dataAvailable && !running &&
                pointInRect(event, sf::FloatRect(238.f, 370.f, 184.f, 44.f)))
        {
            startTraining();
        }
    }

    void handleModelEvents(const sf::Event& event)
    {
        if(pointInRect(event, sf::FloatRect(218.f, 594.f, 150.f, 42.f)))
        {
            loadModel();
            logInfo = readLog();
        }
    }

    void update()
    {
        canvas.update(window);
        maybeConsumeTrainingCompletion();
    }

    void draw()
    {
        window.clear(Theme::background);
        drawShell();

        if(!fontLoaded)
        {
            window.display();
            return;
        }

        switch(currentScreen)
        {
        case Screen::Draw:
            drawDrawScreen();
            break;
        case Screen::Train:
            drawTrainScreen();
            break;
        case Screen::Model:
            drawModelScreen();
            break;
        }

        drawStatusBar();
        window.display();
    }

    void drawShell()
    {
        sf::RectangleShape top(sf::Vector2f(static_cast<float>(windowWidth), 64.f));
        top.setFillColor(Theme::topBar);
        window.draw(top);

        sf::RectangleShape side(sf::Vector2f(180.f, static_cast<float>(windowHeight)));
        side.setPosition(0.f, 64.f);
        side.setFillColor(Theme::sidebar);
        window.draw(side);

        if(!fontLoaded)
            return;

        drawText(window, font, "PureMLP", 26, Theme::text, sf::Vector2f(24.f, 17.f));
        drawText(window, font, modelLoaded ? "Model loaded" : "Model unavailable",
                 16, modelLoaded ? Theme::accent : Theme::warning,
                 sf::Vector2f(790.f, 22.f));

        const sf::Vector2f mouse = toVector2f(sf::Mouse::getPosition(window));
        drawButton(window, font, sf::FloatRect(20.f, 104.f, 140.f, 42.f),
                   "Draw", mouse, false, currentScreen == Screen::Draw);
        drawButton(window, font, sf::FloatRect(20.f, 154.f, 140.f, 42.f),
                   "Train", mouse, false, currentScreen == Screen::Train);
        drawButton(window, font, sf::FloatRect(20.f, 204.f, 140.f, 42.f),
                   "Model", mouse, false, currentScreen == Screen::Model);

        drawText(window, font, "MNIST digit recognizer", 15, Theme::mutedText,
                 sf::Vector2f(24.f, 622.f));
    }

    void drawDrawScreen()
    {
        canvas.setPosition(sf::Vector2f(218.f, 132.f));
        brushSlider.setBounds(sf::FloatRect(218.f, 670.f, 250.f, 28.f));

        drawText(window, font, "Draw", 30, Theme::text, sf::Vector2f(218.f, 88.f));
        drawText(window, font, "Recognition canvas", 17, Theme::mutedText,
                 sf::Vector2f(300.f, 100.f));

        canvas.draw(window, window);

        const sf::Vector2f mouse = toVector2f(sf::Mouse::getPosition(window));
        drawButton(window, font, sf::FloatRect(218.f, 594.f, 112.f, 42.f),
                   "Clear", mouse);
        drawButton(window, font, sf::FloatRect(342.f, 594.f, 112.f, 42.f),
                   "Undo", mouse, false, false, canvas.canUndo());
        drawButton(window, font, sf::FloatRect(466.f, 594.f, 126.f, 42.f),
                   "Predict", mouse, true, false, modelLoaded);
        drawButton(window, font, sf::FloatRect(604.f, 594.f, 126.f, 42.f),
                   autoPredict ? "Auto on" : "Auto off", mouse, false, autoPredict, modelLoaded);
        brushSlider.draw(window, font, "Brush");

        drawPredictionPanel();
    }

    void drawPredictionPanel()
    {
        const sf::FloatRect panel(680.f, 132.f, 270.f, 420.f);
        drawPanel(window, panel);
        drawText(window, font, "Prediction", 22, Theme::text,
                 sf::Vector2f(panel.left + 22.f, panel.top + 20.f));

        if(!modelLoaded)
        {
            drawText(window, font, "Weights not loaded", 18, Theme::warning,
                     sf::Vector2f(panel.left + 22.f, panel.top + 70.f));
            drawText(window, font, modelMessage, 14, Theme::mutedText,
                     sf::Vector2f(panel.left + 22.f, panel.top + 104.f));
            return;
        }

        if(hasPrediction)
        {
            drawCenteredText(window, font, std::to_string(prediction), 96,
                             Theme::accent,
                             sf::FloatRect(panel.left + 22.f, panel.top + 62.f, 104.f, 120.f));

            sf::Texture previewTexture = canvas.makePreviewTexture();
            sf::Sprite preview(previewTexture);
            preview.setScale(4.f, 4.f);
            preview.setPosition(panel.left + 150.f, panel.top + 78.f);
            window.draw(preview);

            sf::RectangleShape previewBorder(sf::Vector2f(112.f, 112.f));
            previewBorder.setPosition(panel.left + 150.f, panel.top + 78.f);
            previewBorder.setFillColor(sf::Color::Transparent);
            previewBorder.setOutlineThickness(1.f);
            previewBorder.setOutlineColor(Theme::border);
            window.draw(previewBorder);

            float y = panel.top + 210.f;
            for(int i = 0; i < 10; ++i)
            {
                drawText(window, font, std::to_string(i), 15, Theme::mutedText,
                         sf::Vector2f(panel.left + 24.f, y - 3.f));

                const float probability = static_cast<float>(probabilities[i]);
                drawProgressBar(window,
                                sf::FloatRect(panel.left + 50.f, y, 150.f, 13.f),
                                probability,
                                i == prediction ? Theme::accent : Theme::blue);

                drawText(window, font, formatNumber(probability * 100.0, 1) + "%",
                         14, Theme::text,
                         sf::Vector2f(panel.left + 210.f, y - 4.f));
                y += 20.f;
            }
        }
        else
        {
            drawText(window, font, "No prediction yet", 18, Theme::mutedText,
                     sf::Vector2f(panel.left + 22.f, panel.top + 76.f));
        }
    }

    void drawTrainScreen()
    {
        drawText(window, font, "Train", 30, Theme::text, sf::Vector2f(218.f, 88.f));

        drawPanel(window, sf::FloatRect(218.f, 132.f, 300.f, 300.f));
        drawText(window, font, "Configuration", 22, Theme::text,
                 sf::Vector2f(238.f, 154.f));

        const sf::Vector2f mouse = toVector2f(sf::Mouse::getPosition(window));

        drawText(window, font, "Epochs", 16, Theme::mutedText, sf::Vector2f(238.f, 184.f));
        drawButton(window, font, sf::FloatRect(238.f, 212.f, 42.f, 36.f), "-", mouse);
        drawCenteredText(window, font, std::to_string(trainEpochs), 20, Theme::text,
                         sf::FloatRect(282.f, 212.f, 76.f, 36.f));
        drawButton(window, font, sf::FloatRect(360.f, 212.f, 42.f, 36.f), "+", mouse);

        drawText(window, font, "Learning rate", 16, Theme::mutedText,
                 sf::Vector2f(238.f, 268.f));
        drawButton(window, font, sf::FloatRect(238.f, 296.f, 42.f, 36.f), "-", mouse);
        drawCenteredText(window, font, formatNumber(learningRate, 4), 18, Theme::text,
                         sf::FloatRect(282.f, 296.f, 76.f, 36.f));
        drawButton(window, font, sf::FloatRect(360.f, 296.f, 42.f, 36.f), "+", mouse);

        const bool dataAvailable = std::filesystem::exists("mnist_data.txt") &&
                                   std::filesystem::exists("mnist_labels.txt");
        const bool running = isTrainingRunning();
        drawButton(window, font, sf::FloatRect(238.f, 370.f, 184.f, 44.f),
                   running ? "Training" : "Start training", mouse, true,
                   false, dataAvailable && !running);

        drawDatasetPanel();
        drawTrainingProgressPanel();
    }

    void drawDatasetPanel()
    {
        const sf::FloatRect panel(548.f, 132.f, 402.f, 178.f);
        drawPanel(window, panel);
        drawText(window, font, "Dataset", 22, Theme::text,
                 sf::Vector2f(panel.left + 20.f, panel.top + 20.f));

        drawFileStatus(panel.left + 22.f, panel.top + 70.f, "mnist_data.txt");
        drawFileStatus(panel.left + 22.f, panel.top + 104.f, "mnist_labels.txt");

        if(!std::filesystem::exists("mnist_data.txt") &&
           std::filesystem::exists("mnist_data.7z"))
        {
            drawText(window, font, "Compressed data archive is present",
                     15, Theme::warning,
                     sf::Vector2f(panel.left + 22.f, panel.top + 138.f));
        }
    }

    void drawTrainingProgressPanel()
    {
        TrainingState snapshot;
        {
            std::lock_guard<std::mutex> lock(trainingState->mutex);
            snapshot.running = trainingState->running;
            snapshot.done = trainingState->done;
            snapshot.failed = trainingState->failed;
            snapshot.message = trainingState->message;
            snapshot.progress = trainingState->progress;
        }

        const sf::FloatRect panel(548.f, 340.f, 402.f, 236.f);
        drawPanel(window, panel);
        drawText(window, font, "Progress", 22, Theme::text,
                 sf::Vector2f(panel.left + 20.f, panel.top + 20.f));

        const float progress = snapshot.progress.totalEpochs > 0
            ? snapshot.progress.epoch / static_cast<float>(snapshot.progress.totalEpochs)
            : 0.f;

        drawProgressBar(window, sf::FloatRect(panel.left + 22.f, panel.top + 72.f, 350.f, 18.f),
                        progress, snapshot.failed ? Theme::danger : Theme::accent);

        const sf::Color messageColor = snapshot.failed ? Theme::danger :
                                       snapshot.done ? Theme::accent :
                                       Theme::mutedText;
        drawText(window, font, snapshot.message, 16, messageColor,
                 sf::Vector2f(panel.left + 22.f, panel.top + 108.f));

        drawMetric(panel.left + 22.f, panel.top + 150.f, "Train MSE",
                   snapshot.progress.epoch > 0 ? formatNumber(snapshot.progress.trainMse, 6)
                                               : formatNumber(logInfo.mse, 6));
        drawMetric(panel.left + 150.f, panel.top + 150.f, "Train Acc",
                   snapshot.progress.epoch > 0 ? formatNumber(snapshot.progress.trainAcc, 2) + "%"
                                               : formatNumber(logInfo.trainAcc, 2) + "%");
        drawMetric(panel.left + 278.f, panel.top + 150.f, "Valid Acc",
                   snapshot.progress.epoch > 0 ? formatNumber(snapshot.progress.validAcc, 2) + "%"
                                               : formatNumber(logInfo.validAcc, 2) + "%");
    }

    void drawModelScreen()
    {
        drawText(window, font, "Model", 30, Theme::text, sf::Vector2f(218.f, 88.f));

        drawPanel(window, sf::FloatRect(218.f, 132.f, 732.f, 192.f));
        drawText(window, font, "Architecture", 22, Theme::text, sf::Vector2f(240.f, 154.f));
        drawArchitecture(sf::Vector2f(344.f, 230.f));

        drawPanel(window, sf::FloatRect(218.f, 356.f, 356.f, 196.f));
        drawText(window, font, "Saved Metrics", 22, Theme::text, sf::Vector2f(240.f, 378.f));
        drawMetric(240.f, 432.f, "Train MSE", formatNumber(logInfo.mse, 6));
        drawMetric(360.f, 432.f, "Train Acc", formatNumber(logInfo.trainAcc, 2) + "%");
        drawMetric(480.f, 432.f, "Valid Acc", formatNumber(logInfo.validAcc, 2) + "%");

        drawPanel(window, sf::FloatRect(604.f, 356.f, 346.f, 196.f));
        drawText(window, font, "Weights", 22, Theme::text, sf::Vector2f(626.f, 378.f));
        drawFileStatus(626.f, 426.f, "hidden1_weights.weights");
        drawFileStatus(626.f, 456.f, "hidden2_weights.weights");
        drawFileStatus(626.f, 486.f, "output_weights.weights");

        const sf::Vector2f mouse = toVector2f(sf::Mouse::getPosition(window));
        drawButton(window, font, sf::FloatRect(218.f, 594.f, 150.f, 42.f),
                   "Reload", mouse, true);
        drawText(window, font, modelMessage, 16,
                 modelLoaded ? Theme::accent : Theme::warning,
                 sf::Vector2f(388.f, 604.f));
    }

    void drawArchitecture(sf::Vector2f origin)
    {
        const std::array<std::string, 4> labels = {"784", "128", "64", "10"};
        const std::array<std::string, 4> names = {"Input", "Hidden 1", "Hidden 2", "Output"};
        const float spacing = 160.f;

        for(int i = 0; i < 3; ++i)
        {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(origin.x + i * spacing + 48.f, origin.y), Theme::border),
                sf::Vertex(sf::Vector2f(origin.x + (i + 1) * spacing - 48.f, origin.y), Theme::border)
            };
            window.draw(line, 2, sf::Lines);
        }

        for(int i = 0; i < 4; ++i)
        {
            sf::CircleShape node(42.f);
            node.setOrigin(42.f, 42.f);
            node.setPosition(origin.x + i * spacing, origin.y);
            node.setFillColor(i == 3 ? Theme::accentSoft : Theme::panelSoft);
            node.setOutlineThickness(1.f);
            node.setOutlineColor(i == 3 ? Theme::accent : Theme::border);
            window.draw(node);

            drawCenteredText(window, font, labels[i], 20, Theme::text,
                             sf::FloatRect(origin.x + i * spacing - 42.f,
                                           origin.y - 22.f, 84.f, 44.f));
            drawCenteredText(window, font, names[i], 14, Theme::mutedText,
                             sf::FloatRect(origin.x + i * spacing - 60.f,
                                           origin.y + 52.f, 120.f, 28.f));
        }
    }

    void drawMetric(float x, float y, const std::string& label, const std::string& value)
    {
        drawText(window, font, label, 14, Theme::mutedText, sf::Vector2f(x, y));
        drawText(window, font, value, 22, Theme::text, sf::Vector2f(x, y + 24.f));
    }

    void drawFileStatus(float x, float y, const std::string& fileName)
    {
        const bool exists = std::filesystem::exists(fileName);

        sf::CircleShape dot(5.f);
        dot.setPosition(x, y + 6.f);
        dot.setFillColor(exists ? Theme::accent : Theme::danger);
        window.draw(dot);

        drawText(window, font, fileName, 15, exists ? Theme::text : Theme::mutedText,
                 sf::Vector2f(x + 18.f, y));
        drawText(window, font, exists ? "found" : "missing", 15,
                 exists ? Theme::accent : Theme::danger,
                 sf::Vector2f(x + 230.f, y));
    }

    void drawStatusBar()
    {
        sf::RectangleShape bar(sf::Vector2f(static_cast<float>(windowWidth), 34.f));
        bar.setPosition(0.f, windowHeight - 34.f);
        bar.setFillColor(sf::Color(15, 19, 25));
        window.draw(bar);

        drawText(window, font, statusText, 15, Theme::mutedText,
                 sf::Vector2f(218.f, windowHeight - 25.f));
    }

    void runPrediction()
    {
        if(!modelLoaded || !model)
        {
            statusText = "Model weights are not loaded";
            return;
        }

        try
        {
            probabilities = model->predictProba(canvas.toDigitVector());
            prediction = maxIndex(probabilities.data(), 10);
            hasPrediction = true;
            statusText = "Prediction: " + std::to_string(prediction);
            canvas.consumeDirty();
        }
        catch(const std::exception& ex)
        {
            hasPrediction = false;
            statusText = ex.what();
        }
        catch(...)
        {
            hasPrediction = false;
            statusText = "Prediction failed";
        }
    }

    void loadModel()
    {
        try
        {
            model = std::make_unique<MLP>(784, 128, 64, 10, true);
            modelLoaded = true;
            modelMessage = "Weights loaded";
            statusText = "Model ready";
        }
        catch(const std::exception& ex)
        {
            model.reset();
            modelLoaded = false;
            modelMessage = ex.what();
            statusText = "Model load failed";
        }
        catch(...)
        {
            model.reset();
            modelLoaded = false;
            modelMessage = "Weight files could not be read";
            statusText = "Model load failed";
        }
    }

    void startTraining()
    {
        if(trainingThread.joinable())
            trainingThread.join();

        trainingState = std::make_shared<TrainingState>();
        {
            std::lock_guard<std::mutex> lock(trainingState->mutex);
            trainingState->running = true;
            trainingState->message = "Preparing training data";
            trainingState->progress.totalEpochs = trainEpochs;
        }

        const int epochs = trainEpochs;
        const double rate = learningRate;
        const std::shared_ptr<TrainingState> state = trainingState;

        trainingThread = std::thread([state, epochs, rate]()
        {
            try
            {
                if(!std::filesystem::exists("mnist_data.txt"))
                    throw std::runtime_error("mnist_data.txt is missing");
                if(!std::filesystem::exists("mnist_labels.txt"))
                    throw std::runtime_error("mnist_labels.txt is missing");

                initSeed();

                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->message = "Loading MNIST data";
                }

                MLP trainingModel(784, 128, 64, 10, false);
                DataLoader dataLoader("mnist_data.txt", "mnist_labels.txt");
                std::vector<std::pair<Matrix, Matrix>> splits =
                    dataLoader.trainValidTestSplit(64832, 5120, 0);

                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->message = "Training started";
                }

                trainingModel.fit(splits[0].first, splits[0].second,
                                  splits[1].first, splits[1].second,
                                  splits[2].first, splits[2].second,
                                  epochs, rate,
                                  [state](const MLP::TrainingProgress& progress)
                                  {
                                      std::lock_guard<std::mutex> lock(state->mutex);
                                      state->progress = progress;
                                      state->message = "Epoch " +
                                          std::to_string(progress.epoch) + " of " +
                                          std::to_string(progress.totalEpochs);
                                  });

                {
                    std::lock_guard<std::mutex> lock(state->mutex);
                    state->running = false;
                    state->done = true;
                    state->message = "Training complete";
                }
            }
            catch(const std::exception& ex)
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                state->running = false;
                state->failed = true;
                state->done = true;
                state->message = ex.what();
            }
            catch(...)
            {
                std::lock_guard<std::mutex> lock(state->mutex);
                state->running = false;
                state->failed = true;
                state->done = true;
                state->message = "Training failed";
            }
        });

        statusText = "Training started";
    }

    bool isTrainingRunning() const
    {
        std::lock_guard<std::mutex> lock(trainingState->mutex);
        return trainingState->running;
    }

    void maybeConsumeTrainingCompletion()
    {
        bool shouldReload = false;
        bool failed = false;
        {
            std::lock_guard<std::mutex> lock(trainingState->mutex);
            if(trainingState->done && !trainingState->consumed)
            {
                trainingState->consumed = true;
                shouldReload = true;
                failed = trainingState->failed;
            }
        }

        if(shouldReload)
        {
            if(trainingThread.joinable())
                trainingThread.join();

            logInfo = readLog();
            if(!failed)
                loadModel();
        }
    }

    static constexpr unsigned int windowWidth = 1000;
    static constexpr unsigned int windowHeight = 740;

    sf::RenderWindow window;
    sf::Font font;
    bool fontLoaded = false;
    Screen currentScreen = Screen::Draw;

    DigitCanvas canvas;
    Slider brushSlider;

    std::unique_ptr<MLP> model;
    bool modelLoaded = false;
    std::string modelMessage = "Weights not loaded";
    std::array<double, 10> probabilities{};
    int prediction = -1;
    bool hasPrediction = false;
    bool autoPredict = false;

    LogInfo logInfo{0.0, 0.0, 0.0};
    int trainEpochs = 30;
    double learningRate = 0.01;
    std::shared_ptr<TrainingState> trainingState;
    std::thread trainingThread;

    std::string statusText = "Ready";
};

int main()
{
    try
    {
        setupWorkingDirectory();
        App app;
        app.run();
    }
    catch(const MatrixException& ex)
    {
        std::cerr << "Matrix exception: " << ex.what() << std::endl;
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Unexpected application error." << std::endl;
    }

    return 0;
}
