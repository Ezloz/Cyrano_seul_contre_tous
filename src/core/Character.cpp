#include "Character.h"

void Character::update(sf::Time dt) {
  const Animation &anim = template->find(state);
  elapsed += dt;
  while (elapsed >= anim.frames[frameIndex].duration) {
    elapsed -= anim.frames[frameIndex].duration;
    ++frameIndex;
    if (frameIndex >= anim.frames.size()) {
      frameIndex = 0;
    }
  }
}

void Character::draw(sf::RenderTarget &target,
                     const tmx::Vector2u &tileSize) const {
  const Animation &anim = template->find(state);
  const Frame &f = anim.frames[frameIndex];

  sf::Sprite sprite = sprite;
  sprite.setTextureRect({f.x, f.y, f.w, f.h});
  sprite.setPosition(
      static_cast<float>(coord.x * static_cast<std::int32_t>(tileSize.x)),
      static_cast<float>(coord.y * static_cast<std::int32_t>(tileSize.y)));
  target.draw(sprite);
}
