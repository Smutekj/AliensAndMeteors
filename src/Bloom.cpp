#include "Bloom.h"

    Bloom::Bloom(int width , int height)
        : texture_size(width, height)
    {
        std::string shader_path = "../Resources/Shaders/";
        m_shaders.load(Shaders::ID::DownSamplePass, shader_path + "basic.vert", shader_path + "downsample.frag");
        m_shaders.load(Shaders::ID::AddPass, shader_path + "basic.vert", shader_path + "add.frag");
        m_shaders.load(Shaders::ID::BrightnessPass,shader_path + "basic.vert", shader_path + "brigthness.frag");
        m_shaders.load(Shaders::ID::GaussianVertPass, shader_path + "basic.vert", shader_path + "discreteVert.frag");
        m_shaders.load(Shaders::ID::GaussianHorizPass, shader_path + "basic.vert",  shader_path + "discreteHoriz.frag");
        m_shaders.load(Shaders::ID::FullPass,shader_path + "basic.vert", shader_path + "fullpass.frag");

        m_pass_textures[0].setSmooth(true);
        m_pass_textures[1].setSmooth(true);

        m_pass_textures[0].create(width, height);
        m_pass_textures[1].create(width, height);

        m_downsized_texture.create(width / 2, height / 2);
        m_downsized_texture.setSmooth(true);
    
        m_texture_rect.resize(4);
        m_texture_rect.setPrimitiveType(sf::Quads);
        
    }

    void Bloom::doTheThing(const sf::RenderTexture &input, sf::RenderTarget &output)
    {
        m_pass_textures[0].clear(sf::Color::Transparent);
        m_pass_textures[1].clear(sf::Color::Transparent);

        auto old_view = output.getView();
        output.setView(output.getDefaultView());


        sf::Vector2f rect_size;
        rect_size.x = m_downsized_texture.getSize().x;
        rect_size.y = m_downsized_texture.getSize().y;
        m_texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        m_texture_rect[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        m_texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        m_texture_rect[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};

        sf::RenderStates states;

        auto& downsampler = m_shaders.get(Shaders::ID::DownSamplePass);
        states.shader = &downsampler;
        states.blendMode = sf::BlendNone;
        downsampler.setUniform("image", input.getTexture());
        m_downsized_texture.draw(m_texture_rect, states);
        m_downsized_texture.display();
        
        rect_size.x = m_pass_textures[0].getSize().x;
        rect_size.y = m_pass_textures[0].getSize().y;
        m_texture_rect[0] = sf::Vertex{{0, 0}, sf::Color::Transparent, {0, 1}};
        m_texture_rect[1] = sf::Vertex{{rect_size.x, 0}, sf::Color::Transparent, {1, 1}};
        m_texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, sf::Color::Transparent, {1, 0}};
        m_texture_rect[3] = sf::Vertex{{0, rect_size.y}, sf::Color::Transparent, {0, 0}};


        auto& brigthness = m_shaders.get(Shaders::ID::BrightnessPass);
        brigthness.setUniform("image", m_downsized_texture.getTexture());
        states.blendMode = sf::BlendNone;
        states.shader = &brigthness;

        m_pass_textures[0].draw(m_texture_rect, states);
        m_pass_textures[0].display();

        for (int i = 0; i < 5; ++i)
        {
            auto& vert_pass = m_shaders.get(Shaders::ID::GaussianVertPass);
            vert_pass.setUniform("image", m_pass_textures[0].getTexture());
            states.shader = &vert_pass;
            m_pass_textures[1].draw(m_texture_rect, states);
            m_pass_textures[1].display();

            auto& horiz_pass = m_shaders.get(Shaders::ID::GaussianVertPass);;
            horiz_pass.setUniform("image", m_pass_textures[1].getTexture());
            states.shader = &horiz_pass;
            m_pass_textures[0].draw(m_texture_rect, states);
            m_pass_textures[0].display();
        }

        
        rect_size.x = input.getSize().x;
        rect_size.y = input.getSize().y;
        sf::Color test_color = sf::Color::Transparent;//sf::Color(0,0,0,255);
        m_texture_rect[0] = sf::Vertex{{0, 0}, test_color, {0, 1}};
        m_texture_rect[1] = sf::Vertex{{rect_size.x, 0}, test_color, {1, 1}};
        m_texture_rect[2] = sf::Vertex{{rect_size.x, rect_size.y}, test_color, {1, 0}};
        m_texture_rect[3] = sf::Vertex{{0, rect_size.y}, test_color, {0, 0}};

        auto& full_pass = m_shaders.get(Shaders::ID::FullPass);;
        full_pass.setUniform("image", m_pass_textures[0].getTexture());

        states.shader = &full_pass;
        states.blendMode = sf::BlendAdd;
        states.blendMode.colorDstFactor = sf::BlendMode::Factor::OneMinusSrcAlpha;
        states.blendMode.alphaSrcFactor = sf::BlendMode::Factor::Zero;
        states.blendMode.alphaDstFactor = sf::BlendMode::Factor::Zero;
        output.draw(m_texture_rect, states);
        output.setView(old_view);
    }