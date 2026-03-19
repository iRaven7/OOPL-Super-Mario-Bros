#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <string>

class Block : public Util::GameObject {
public:
    explicit Block(const std::string& imagePath) {
        SetImage(imagePath);
    }

    void SetImage(const std::string& imagePath) {
        m_ImagePath = imagePath;
        m_Drawable = std::make_shared<Util::Image>(m_ImagePath);
    }

    void SetPosition(const glm::vec2& Position) {
        m_Transform.translation = Position;
    }

private:
    std::string m_ImagePath;
};

#endif