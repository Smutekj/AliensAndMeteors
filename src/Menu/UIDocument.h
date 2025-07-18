#pragma once 

#include <string>
#include <vector>
#include <memory>

#include <Rect.h>
#include <Text.h>
#include <Sprite.h>

enum class UIEvent
{
    MOUSE_ENETERED,
    MOUSE_LEFT,
    CLICK
};

enum class Layout
{
    X,
    Y,
    Grid,
    Hidden,
};

enum class Alignement
{
    Left,
    Center,
    Right,
};

struct Style
{
    Alignement align;
    Layout display;
};

enum class Sizing
{
    FIXED,
    SCALE_TO_FIT,
    RESCALE_CHILDREN,
};

enum class Position
{
    ABSOLUTE,
    RELATIVE,
};

class Renderer;
class Font;

struct UIElement
{
    using UIElementP = std::shared_ptr<UIElement>;

    Rect<int> bounding_box;
    UIElement *parent = nullptr;
    std::string id;

    int max_width;

    enum class DimensionType
    {
        Relative,
        Absolute,
    };

    DimensionType width_style = DimensionType::Absolute;
    DimensionType height_style = DimensionType::Absolute;

    bool mouse_hovering = false;

    Sizing sizing = Sizing::FIXED;

    utils::Vector2i margin = {0, 0};
    utils::Vector2i padding = {0, 0};
    int column_count = 3;

    Layout layout = Layout::X;
    Alignement align = Alignement::Center;

    std::vector<UIElementP> children;

    std::unordered_map<UIEvent, std::function<void(UIElementP)>> event_callbacks;

public:
    virtual void update() ;
    virtual void draw(Renderer &canvas);

    template <class... Args>
    void addChildren(Args... child_el);

    UIElement* getElementById(const std::string &id) ;
    
    void drawX(Renderer &canvas);
    
    int width() const;
    int height() const;
    int x() const;
    int y() const;
    
    private:
    utils::Vector2i totalChildrenMargin() const;

    double maxChildrenHeight() const;
    double maxChildrenWidth() const;
    double totalChildrenWidth() const;
    double totalChildrenHeight() const;
};

class UIDocument
{
public:
    UIDocument(Renderer &window_canvas);

    void onEvent(UIEvent event, UIElement::UIElementP node_p);

    void handleEvent(UIEvent event);

    UIElement *getElementById(const std::string &id) const;

    void forEach(std::function<void(UIElement::UIElementP)> callback);

    void drawUI();

    UIElement::UIElementP root = nullptr;
    Renderer &document;
};

struct TextUIELement : UIElement
{
    TextUIELement(Font &font, std::string text);

    virtual void draw(Renderer &canvas) override;

    void setFont(Font &font);

    Text m_text;
};

struct SpriteUIELement : UIElement
{
    virtual void draw(Renderer &canvas) override;
    void setTexture(Texture &tex);

    Sprite image;
};

// struct SliderUIELement : UIElement
// {

//     SliderUIELement()
//     {

//     }
//     // virtual void draw(Renderer &canvas) override;
//     void setTexture(Texture &tex);
    
//     double value = 0.;
//     double min_value = 0.;
//     double max_value = 1.;
    
//     Sprite image;
// };

// struct TextInputElement : UIElement
// {
    
//     // virtual void draw(Renderer &canvas) override;
// };

template <class... Args>
void UIElement::addChildren(Args... child_el)
{
    ((child_el->parent = this), ...);
    ((children.push_back(child_el)), ...);
}

