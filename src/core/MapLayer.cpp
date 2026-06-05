
/* Adaptation of
   https://github.com/fallahn/tmxlite/blob/master/SFML3Example/SFMLExample/src/SFMLOrthogonalLayer.hpp
    - "SFMLOrthogonalLayer.hpp" is renamed into "SFMLOrthogonalLayer.h"
*/
/*********************************************************************
(c) Matt Marchant & contributors 2016 - 2024
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

/*
Creates an SFML drawable from an Orthogonal tmx map layer.
This is an example of drawing with SFML - not all features
are implemented.
*/

#include "MapLayer.h"


void MapLayer::update(sf::Time elapsed) 
{
  for (auto& c : m_visibleChunks) 
  {
    for (AnimationState& as : c->getActiveAnimations()) 
    {
      as.currentTime += elapsed;

      tmx::TileLayer::Tile tile;
      std::int32_t animTime = 0;
      auto x = as.animTile.animation.frames.begin();
      while (animTime < as.currentTime.asMilliseconds()) 
      {
        if (x == as.animTile.animation.frames.end()) 
        {
          x = as.animTile.animation.frames.begin();
          as.currentTime -= sf::milliseconds(animTime);
          animTime = 0;
        }

        tile.ID = x->tileID;
        animTime += x->duration;
        x++;
      }

      setTile(as.tileCords.x, as.tileCords.y, tile);
    }
  }
}

private:
  //increasing m_chunkSize by 4; fixes render problems when mapsize != chunksize
  //sf::Vector2f m_chunkSize = sf::Vector2f(1024.f, 1024.f);
  sf::Vector2f m_chunkSize = sf::Vector2f(512.f, 512.f);
  sf::Vector2u m_chunkCount;
  sf::Vector2u m_mapTileSize;   // general Tilesize of Map
  sf::FloatRect m_globalBounds;
  sf::Vector2f m_offset;

  using TextureResource = std::map<std::string, std::unique_ptr<sf::Texture>>;
  TextureResource m_textureResource;

  struct AnimationState
  {
    sf::Vector2u tileCords;
    sf::Time startTime;
    sf::Time currentTime;
    tmx::Tileset::Tile animTile;
    std::uint8_t flipFlags;
  };


  Chunk::Ptr& MapLayer::getChunkAndTransform(std::int32_t x, std::int32_t y, sf::Vector2u& chunkRelative)
  {
    std::uint32_t chunkX = (x * m_mapTileSize.x) / static_cast<std::uint32_t>(m_chunkSize.x);
    std::uint32_t chunkY = (y * m_mapTileSize.y) / static_cast<std::uint32_t>(m_chunkSize.y);
    chunkRelative.x = ((x * m_mapTileSize.x) - chunkX * static_cast<std::uint32_t>(m_chunkSize.x)) / m_mapTileSize.x ;
    chunkRelative.y = ((y * m_mapTileSize.y) - chunkY * static_cast<std::uint32_t>(m_chunkSize.y)) / m_mapTileSize.y ;
    return  m_chunks[chunkX + chunkY * m_chunkCount.x];
  }

  void MapLayer::createChunks(const tmx::Map& map, const tmx::TileLayer& layer)
  {
    //look up all the tile sets and load the textures
    const auto& tileSets = map.getTilesets();
    const auto& layerIDs = layer.getTiles();
    std::uint32_t maxID = std::numeric_limits<std::uint32_t>::max();
    std::vector<const tmx::Tileset*> usedTileSets;

    for (auto i = tileSets.rbegin(); i != tileSets.rend(); ++i)
    {
      for (const auto& tile : layerIDs)
      {
        if (tile.ID >= i->getFirstGID() && tile.ID < maxID)
        {
          usedTileSets.push_back(&(*i));
          break;
        }
      }
      maxID = i->getFirstGID();
    }

    sf::Image fallback({2, 2}, sf::Color::Magenta);
    for (const auto& ts : usedTileSets)
    {
      const auto& path = ts->getImagePath();

      std::unique_ptr<sf::Texture> newTexture = std::make_unique<sf::Texture>();
      sf::Image img;
      if (!img.loadFromFile(path))
      {
        if (!newTexture->loadFromImage(fallback)) {
            throw std::runtime_error("Unable to load fallback image");
        }
      } else {
        if (ts->hasTransparency())
        {
          auto transparency = ts->getTransparencyColour();
          img.createMaskFromColor({ transparency.r, transparency.g, transparency.b, transparency.a });
        }
        if (!newTexture->loadFromImage(img)) {
          throw std::runtime_error("Unable to load image: " + path);
        }
      }
      m_textureResource.insert(std::make_pair(path, std::move(newTexture)));
    }

    //calculate the number of chunks in the layer
    //and create each one
    const auto bounds = map.getBounds();
    m_chunkCount.x = static_cast<std::uint32_t>(std::ceil(bounds.width / m_chunkSize.x));
    m_chunkCount.y = static_cast<std::uint32_t>(std::ceil(bounds.height / m_chunkSize.y));

    sf::Vector2u tileSize(map.getTileSize().x, map.getTileSize().y);

    for (auto y = 0u; y < m_chunkCount.y; ++y)
    {
      sf::Vector2f tileCount(m_chunkSize.x / tileSize.x, m_chunkSize.y / tileSize.y);
      for (auto x = 0u; x < m_chunkCount.x; ++x)
      {
        // calculate size of each Chunk (clip against map)
        if ((x + 1) * m_chunkSize.x > bounds.width)
        {
          tileCount.x = (bounds.width - x * m_chunkSize.x) /  map.getTileSize().x;
        }
        if ((y + 1) * m_chunkSize.y > bounds.height)
        {
          tileCount.y = (bounds.height - y * m_chunkSize.y) /  map.getTileSize().y;
        }
        //m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets,
        //    sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, map.getTileCount().x, m_textureResource));
        m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets,
            sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, tileSize, map.getTileCount().x, m_textureResource, map.getAnimatedTiles()));
      }
    }
  }

  void MapLayer::updateVisibility(const sf::View& view) const
  {
      sf::Vector2f viewCorner = view.getCenter();
      viewCorner -= view.getSize() / 2.f;

      std::int32_t posX = static_cast<std::int32_t>(std::floor(viewCorner.x / m_chunkSize.x));
      std::int32_t posY = static_cast<std::int32_t>(std::floor(viewCorner.y / m_chunkSize.y));
      std::int32_t posX2 = static_cast<std::int32_t>(std::ceil((viewCorner.x + view.getSize().x) / m_chunkSize.x));
      std::int32_t posY2 = static_cast<std::int32_t>(std::ceil((viewCorner.y + view.getSize().x)/ m_chunkSize.y));

      std::vector<Chunk*> visible;
      for (auto y = posY; y < posY2; ++y)
      {
          for (auto x = posX; x < posX2; ++x)
          {
              std::size_t idx = y * std::int32_t(m_chunkCount.x) + x;
              if (idx >= 0u && idx < m_chunks.size() && !m_chunks[idx]->empty())
              {
                  visible.push_back(m_chunks[idx].get());
              }
          }
      }

      std::swap(m_visibleChunks, visible);
  }


  void MapLayer::draw(sf::RenderTarget& rt, sf::RenderStates states) const override
  {
      states.transform.translate(m_offset);

      //calc view coverage and draw nearest chunks
      updateVisibility(rt.getView());
      for (const auto& c : m_visibleChunks)
      {
          rt.draw(*c, states);
      }
  }
};

void Chunk::maybeRegenerate(bool refresh) {
  if (refresh)
  {
    for (const auto& ca : m_chunkArrays)
    {
        ca->reset();
    }
    generateTiles();
  }
}

std::int32_t Chunk::calcIndexFrom(std::int32_t x, std::int32_t y) const {
  return x + y * static_cast<std::int32_t>(chunkTileCount.x);
}

bool Chunk::empty() const {
  return m_chunkArrays.empty();
}

void Chunk::flipY(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5) {
  //Flip Y
  sf::Vector2f tmp0 = *v0;
  v0->y = v5->y;
  v3->y = v5->y;
  v5->y = tmp0.y;
  sf::Vector2f tmp2 = *v2;
  v2->y = v1->y;
  v4->y = v1->y;
  v1->y = tmp2.y;
}

void Chunk::flipX(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5) {
  //Flip X
  sf::Vector2f tmp0 = *v0;
  v0->x = v1->x;
  v3->x = v1->x;
  v1->x = tmp0.x;
  sf::Vector2f tmp2 = *v2;
  v2->x = v5->x;
  v4->x = v5->x;
  v5->x = tmp2.x;
}

void Chunk::flipD(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5) {
  //Diagonal flip
  sf::Vector2f tmp2 = *v2;
  *v2 = *v4;
  *v4 = tmp2;
  sf::Vector2f tmp0 = *v0;
  *v0 = *v3;
  *v3 = tmp0;
  sf::Vector2f tmp1 = *v1;
  *v1 = *v5;
  *v5 = tmp1;
}

void Chunk::doFlips(std::uint8_t bits, sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5) {
  //0000 = no change
  //0100 = vertical = swap y axis
  //1000 = horizontal = swap x axis
  //1100 = horiz + vert = swap both axes = horiz+vert = rotate 180 degrees
  //0010 = diag = rotate 90 degrees right and swap x axis
  //0110 = diag+vert = rotate 270 degrees right
  //1010 = horiz+diag = rotate 90 degrees right
  //1110 = horiz+vert+diag = rotate 90 degrees right and swap y axis
  if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
    !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //Shortcircuit tests for nothing to do
    return;
  }
  else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    (bits & tmx::TileLayer::FlipFlag::Vertical) &&
    !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //0100
    flipY(v0,v1,v2,v3,v4,v5);
  }
  else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
    !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //1000
    flipX(v0,v1,v2,v3,v4,v5);
  }
  else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    (bits & tmx::TileLayer::FlipFlag::Vertical) &&
    !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //1100
    flipY(v0,v1,v2,v3,v4,v5);
    flipX(v0,v1,v2,v3,v4,v5);
  }
  else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
    (bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //0010
    flipD(v0,v1,v2,v3,v4,v5);
  }
  else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    (bits & tmx::TileLayer::FlipFlag::Vertical) &&
    (bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //0110
    flipX(v0,v1,v2,v3,v4,v5);
    flipD(v0,v1,v2,v3,v4,v5);
  }
  else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
    (bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //1010
    flipY(v0,v1,v2,v3,v4,v5);
    flipD(v0,v1,v2,v3,v4,v5);
  }
  else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
    (bits & tmx::TileLayer::FlipFlag::Vertical) &&
    (bits & tmx::TileLayer::FlipFlag::Diagonal) )
  {
    //1110
    flipY(v0,v1,v2,v3,v4,v5);
    flipX(v0,v1,v2,v3,v4,v5);
    flipD(v0,v1,v2,v3,v4,v5);
  }
}

void Chunk::draw(sf::RenderTarget& rt, sf::RenderStates states) const override {
  states.transform *= getTransform();
  for (const auto& a : m_chunkArrays)
  {
    rt.draw(*a, states);
  }
}

void ChunkArray::reset() {
  m_vertices.clear();
}

void ChunkArray::addTile(const Chunk::Tile& tile) {
  for (const auto& v : tile)
  {
    m_vertices.push_back(v);
  }
}

void ChunkArray::draw(sf::RenderTarget& rt, sf::RenderStates states) const override {
  states.texture = &m_texture;
  rt.draw(m_vertices.data(), m_vertices.size(), sf::PrimitiveType::Triangles, states);
}
