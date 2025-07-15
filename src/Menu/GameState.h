#pragma once

#include "State.h"
#include <memory>
#include <unordered_map>

#include "SettingsState.h"

class StateStack;
class Game;

class GameState : public State
{

public:
    GameState(StateStack &stack, State::Context context);

    virtual ~GameState() override;
    virtual void update(float dt) override;
    virtual void draw() override;
    virtual void handleEvent(const SDL_Event &event) override;

private:
    std::shared_ptr<Game> mp_game;
};

struct ShopItems
{

    int max_fuel = 100;
    int max_speed = 100;
    int max_accel = 100;
    int max_booster = 100;
};

struct ShopItem
{

    int value = 0;
    int max_value = 100;
    int price = 1;
};

struct ItemUIElement
{
    Rect<int> bounding_box = {};
    std::string name = "Placeholder";
    std::string sprite_name = "Fuel";
};

enum class UIEvent
{
    MOUSE_ENETERED,
    CLICK
};

enum class Layout
{
    X,
    Y,
    Grid,
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

enum class SizeStyle
{
    FIXED,
    AUTO,
    FIT_CHIDLREN,
};

struct UIElement
{
    using UIElementP = std::shared_ptr<UIElement>;

    Rect<int> bounding_box;
    UIElement *parent = nullptr;
    std::string id;

    int max_width;
    float m_scale = 1.;

    enum class DimensionType
    {
        Relative,
        Absolute,
    };

    DimensionType width_style = DimensionType::Absolute;
    DimensionType height_style = DimensionType::Absolute;

    SizeStyle size_style = SizeStyle::FIXED;

    utils::Vector2i margin;
    utils::Vector2i padding;
    int column_count = 3;

    Layout layout = Layout::X;
    Alignement align = Alignement::Center;

    std::vector<UIElementP> children;

    std::unordered_map<UIEvent, std::function<void(UIElementP)>> event_callbacks;

public:
    virtual void update() {};
    virtual void draw(Renderer &canvas)
    {
        utils::Vector2f ul = {bounding_box.pos_x, bounding_box.pos_y};
        utils::Vector2f ur = {bounding_box.pos_x + bounding_box.width, bounding_box.pos_y};
        utils::Vector2f ll = {bounding_box.pos_x, bounding_box.pos_y + bounding_box.height};
        utils::Vector2f lr = {bounding_box.pos_x + bounding_box.width, bounding_box.pos_y + bounding_box.height};

        canvas.drawLineBatched(ul, ur, 1, {0, 1, 0, 1});
        canvas.drawLineBatched(ul, ll, 1, {0, 1, 0, 1});
        canvas.drawLineBatched(ur, lr, 1, {0, 1, 0, 1});
        canvas.drawLineBatched(ll, lr, 1, {0, 1, 0, 1});
    };

    template <class... Args>
    void addChildren(Args... child_el)
    {
        ((child_el->parent = this), ...);
        ((children.push_back(child_el)), ...);
    }

    // std::pair<int, int> childrenInLine(int first_child_id)const
    // {
    //     // if(bounding_box.width == 0)//! we calculate the width and so we can fit all children on one line
    //     // {

    //     //     return {children.size()-1, ;
    //     // }
    // }

    double totalChildrenWidth()
    {
        return std::accumulate(children.begin(), children.end(), 0, [](double val, auto &c_p)
                               { return val + c_p->bounding_box.width; });
    }
    double totalChildrenMargin()
    {
        return std::accumulate(children.begin(), children.end(), 0, [](double val, auto &c_p)
                               { return val + c_p->margin.x * 2.; });
    }

    void drawX(Renderer &canvas)
    {
        int max_x = 0;

        if (parent && bounding_box.width == 0)
        {
            max_width = parent->width();
        }
        else if (!parent && max_width == 0)
        {
            max_width = canvas.getTargetSize().x;
        }
        else if (!parent)
        {
            max_width = bounding_box.width;
        }
        max_x = max_width + bounding_box.pos_x + padding.x;

        //! determine size of children then scale them so that they fit
        if (layout == Layout::X && size_style == SizeStyle::FIT_CHIDLREN)
        {
            double total_width = totalChildrenWidth();
            double total_margin = totalChildrenMargin();

            for(auto& child : children)
            {
                double scale = (bounding_box.width - total_margin) / total_width ;
                child->bounding_box.width *= scale;
                child->bounding_box.height *= scale;
            }
        }

        int x = bounding_box.pos_x + padding.x;
        int y = bounding_box.pos_y + padding.y;

        int prev_height = 0;
        //! set children box positions
        int children_width = 0;
        int children_height = 0;
        for (int i = 0; i < children.size(); ++i)
        {
            auto &child_box = children.at(i)->bounding_box;

            if (layout == Layout::Grid && x + child_box.width > max_x) //! x-overflow
            {
                y += prev_height;
                x = bounding_box.pos_x + padding.x;
                children_height += prev_height;
            }

            if (layout == Layout::X && align == Alignement::Center)
            {
                y = bounding_box.pos_y + bounding_box.height / 2 - child_box.height / 2;
            }
            if (layout == Layout::Y && align == Alignement::Center)
            {
                x = bounding_box.pos_x + bounding_box.width / 2 - child_box.width / 2;
            }
            child_box.pos_x = x + children.at(i)->margin.x;
            child_box.pos_y = y + children.at(i)->margin.y;

            prev_height = std::max(prev_height, children.at(i)->margin.y + child_box.height);

            if (layout == Layout::X || layout == Layout::Grid)
            {
                x += (2.*children.at(i)->margin.x + child_box.width);
            }
            else if (layout == Layout::Y)
            {
                y += (2.*children.at(i)->margin.y + child_box.height);
            }

            children_width += (2 * children.at(i)->margin.x + child_box.width);
        }
        if (bounding_box.width == 0)
        {
            bounding_box.width = std::min(max_width, children_width);
        }
        if (bounding_box.height == 0)
        {
            children_height += prev_height;
            bounding_box.height = children_height + padding.y * 2;
        }

        draw(canvas);
        canvas.drawAll();

        for (auto &child : children)
        {
            child->drawX(canvas);
        }
    }

    int width() const
    {
        return bounding_box.width;
    }
    int height() const
    {
        return bounding_box.height;
    }
    int x() const
    {
        return bounding_box.pos_x;
    }
    int y() const
    {
        return bounding_box.pos_y;
    }
};

class UIDocument
{
public:
    UIDocument(Renderer &window_canvas)
        : document(window_canvas)
    {
        root = std::make_shared<UIElement>();
        root->parent = nullptr;
        root->bounding_box = {0, 0,
                              window_canvas.getTargetSize().x,
                              window_canvas.getTargetSize().y};
    }

    void drawUI()
    {
        root->drawX(document);
    }

    UIElement::UIElementP root = nullptr;
    Renderer &document;
};

struct TextUIELement : UIElement
{
    TextUIELement(Font &font, std::string text)
        : m_text(text)
    {
        setFont(font);
    }

    virtual void draw(Renderer &canvas) override
    {
        UIElement::draw(canvas);

        utils::Vector2f center_pos = {bounding_box.pos_x + width() / 2.,
                                      bounding_box.pos_y + height() / 2.};

        utils::Vector2f size = {width(), height()};
        auto text_bounder = m_text.getBoundingBox();
        utils::Vector2f scale = {width(), height()};

        if (std::abs(text_bounder.width - size.x) > 0.01)
        {
            scale.x = size.x / text_bounder.width;
            scale.y = -size.y / text_bounder.height;
            m_text.setScale(m_scale*scale);
        }

        m_text.setPosition(center_pos.x, center_pos.y - text_bounder.height / 2);
        m_text.centerAround({center_pos.x, center_pos.y});
        canvas.drawText(m_text);
    }

    void setFont(Font &font)
    {
        m_text.setFont(&font);
    }

    Text m_text;
};

struct SpriteUIELement : UIElement
{
    virtual void draw(Renderer &canvas) override
    {
        UIElement::draw(canvas);

        utils::Vector2f center_pos = {bounding_box.pos_x + width() / 2.,
                                      bounding_box.pos_y + height() / 2.};

        utils::Vector2f size = {width(), -height()};
        image.setScale(m_scale * size / 2.f);
        image.setPosition(center_pos);
        canvas.drawSprite(image);
    }
    void setTexture(Texture &tex)
    {
        image.setTexture(tex);
    }

    Sprite image;
};

class ShopState : public State
{

public:
    ShopState(StateStack &stack, State::Context context);

    virtual ~ShopState() override;
    virtual void update(float dt) override;
    virtual void draw() override;
    virtual void handleEvent(const SDL_Event &event) override;

private:
    std::unordered_map<std::string, ShopItem> m_items;

    std::vector<ItemUIElement> m_ui_elements;
    int n_elements_per_row = 3;

    std::shared_ptr<Game> mp_game;

    UIDocument document;
};