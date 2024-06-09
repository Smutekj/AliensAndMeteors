#include "Bloom.h"

Bloom::Bloom(int width, int height)
    : texture_size(width, height)
{
    std::string shader_path = "../Resources/Shaders/";
    m_shaders.load(Shaders::ID::DownSamplePass, shader_path + "basic.vert", shader_path + "downsample.frag");
    m_shaders.load(Shaders::ID::AddPass, shader_path + "basic.vert", shader_path + "add.frag");
    m_shaders.load(Shaders::ID::BrightnessPass, shader_path + "basic.vert", shader_path + "brigthness.frag");
    m_shaders.load(Shaders::ID::GaussianVertPass, shader_path + "basic.vert", shader_path + "discreteVert.frag");
    m_shaders.load(Shaders::ID::GaussianHorizPass, shader_path + "basic.vert", shader_path + "discreteHoriz.frag");
    m_shaders.load(Shaders::ID::FullPass, shader_path + "basic.vert", shader_path + "fullpass.frag");

    m_pass_textures[0].setSmooth(true);
    m_pass_textures[1].setSmooth(true);

    m_pass_textures[0].create(width, height);
    m_pass_textures[1].create(width, height);

    m_downsized_texture.create(width / 2, height / 2);
    m_downsized_texture.setSmooth(true);

    m_texture_rect.resize(4);
    m_texture_rect.setPrimitiveType(sf::Quads);
}

void Bloom::setTextureRectSize(sf::Vector2u new_size)
{
    
    sf::Vector2f rect_size(new_size.x , new_size.y);
    m_texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
    m_texture_rect[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
    m_texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
    m_texture_rect[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};
}

void Bloom::doTheThing(const sf::RenderTexture &input, sf::RenderTarget &output)
{
    m_pass_textures[0].clear(sf::Color::Transparent);
    m_pass_textures[1].clear(sf::Color::Transparent);

    auto old_view = output.getView();
    output.setView(output.getDefaultView());

    setTextureRectSize(m_downsized_texture.getSize());

    sf::RenderStates states;

    auto &downsampler = m_shaders.get(Shaders::ID::DownSamplePass);
    states.shader = &downsampler;
    states.blendMode = sf::BlendNone;
    downsampler.setUniform("image", input.getTexture());
    m_downsized_texture.draw(m_texture_rect, states);
    m_downsized_texture.display();

    setTextureRectSize(m_pass_textures[0].getSize());


    auto &brigthness = m_shaders.get(Shaders::ID::BrightnessPass);
    brigthness.setUniform("image", m_downsized_texture.getTexture());
    states.blendMode = sf::BlendNone;
    states.shader = &brigthness;

    m_pass_textures[0].draw(m_texture_rect, states);
    m_pass_textures[0].display();

    for (int i = 0; i < 5; ++i)
    {
        auto &vert_pass = m_shaders.get(Shaders::ID::GaussianHorizPass);
        vert_pass.setUniform("image", m_pass_textures[0].getTexture());
        states.shader = &vert_pass;
        m_pass_textures[1].draw(m_texture_rect, states);
        m_pass_textures[1].display();

        auto &horiz_pass = m_shaders.get(Shaders::ID::GaussianVertPass);
        ;
        horiz_pass.setUniform("image", m_pass_textures[1].getTexture());
        states.shader = &horiz_pass;
        m_pass_textures[0].draw(m_texture_rect, states);
        m_pass_textures[0].display();
    }

    setTextureRectSize(input.getSize());


    auto &full_pass = m_shaders.get(Shaders::ID::FullPass);
    ;
    full_pass.setUniform("image", m_pass_textures[0].getTexture());

    states.shader = &full_pass;
    states.blendMode = sf::BlendAdd;
    // states.blendMode.colorDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
    // states.blendMode.alphaSrcFactor = sf::BlendMode::Factor::Zero;
    // states.blendMode.alphaDstFactor = sf::BlendMode::Factor::Zero;
    output.draw(m_texture_rect, states);
    output.setView(old_view);
}