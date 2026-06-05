
#ifndef SFML_ORTHO_HPP_
#define SFML_ORTHO_HPP_

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <stdexcept>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/detail/Log.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <limits>
#include <iostream>
#include <cmath>


class MapLayer final : public sf::Drawable {
public:
  MapLayer(const tmx::Map& map, std::size_t idx)
  {
    const auto& layers = map.getLayers();
    if (map.getOrientation() != tmx::Orientation::Orthogonal)
    {
      std::cout << "Map is not orthogonal - nothing will be drawn" << std::endl;
    }
    else if (idx >= layers.size())
    {
      std::cout << "Layer index " << idx << " is out of range, layer count is " << layers.size() << std::endl;
    }
    else if (layers[idx]->getType() != tmx::Layer::Type::Tile)
    {
      std::cout << "layer " << idx << " is not a Tile layer..." << std::endl;
    }

    else
    {
      //round the chunk size to the nearest tile
      const auto tileSize = map.getTileSize();
      m_chunkSize.x = std::floor(m_chunkSize.x / tileSize.x) * tileSize.x;
      m_chunkSize.y = std::floor(m_chunkSize.y / tileSize.y) * tileSize.y;
      m_mapTileSize.x = map.getTileSize().x;
      m_mapTileSize.y = map.getTileSize().y;
      const auto& layer = layers[idx]->getLayerAs<tmx::TileLayer>();
      createChunks(map, layer);

      auto mapSize = map.getBounds();
      m_globalBounds.size = {mapSize.width, mapSize.height};
    }
  }

  ~MapLayer() = default;
  MapLayer(const MapLayer&) = delete;
  MapLayer& operator = (const MapLayer&) = delete;

  const sf::FloatRect& MapLayer::getGlobalBounds() const { return m_globalBounds; }

  void setTile(std::int32_t tileX, std::int32_t tileY, tmx::TileLayer::Tile tile, bool refresh = true)
  {
    sf::Vector2u chunkLocale;
    const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
    selectedChunk->setTile(chunkLocale.x, chunkLocale.y, tile, refresh);
  }

  tmx::TileLayer::Tile getTile(std::int32_t tileX, std::int32_t tileY) {
    sf::Vector2u chunkLocale;
    const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
    return selectedChunk->getTile(chunkLocale.x, chunkLocale.y);
  }

  void setColor(std::int32_t tileX, std::int32_t tileY, sf::Color color, bool refresh = true)
  {
    sf::Vector2u chunkLocale;
    const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
    selectedChunk->setColor(chunkLocale.x, chunkLocale.y, color, refresh);
  }

  sf::Color getColor(std::int32_t tileX, std::int32_t tileY)
  {
    sf::Vector2u chunkLocale;
    const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
    return selectedChunk->getColor(chunkLocale.x, chunkLocale.y);
  }

  void setOffset(sf::Vector2f offset) { m_offset = offset; }
  sf::Vector2f getOffset() const { return m_offset; }

  void update(sf::Time elapsed);

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

  std::vector<Chunk::Ptr> m_chunks;
  mutable std::vector<Chunk*> m_visibleChunks;

  Chunk::Ptr& getChunkAndTransform(std::int32_t x, std::int32_t y, sf::Vector2u& chunkRelative);
  void createChunks(const tmx::Map& map, const tmx::TileLayer& layer);
  void updateVisibility(const sf::View& view) const;
  void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;



class Chunk final : public sf::Transformable, public sf::Drawable {
public:
  using Ptr = std::unique_ptr<Chunk>;
  using Tile = std::array<sf::Vertex, 6u>;

  Chunk(const tmx::TileLayer& layer,  std::vector<const tmx::Tileset*> tilesets,
        const sf::Vector2f& position, const sf::Vector2f& tileCount, 
        const sf::Vector2u& tileSize, std::size_t rowSize,  
        TextureResource& tr,          const std::map<std::uint32_t, tmx::Tileset::Tile>& animTiles)
      : m_animTiles(animTiles)
  {
    setPosition(position);
    layerOpacity = static_cast<std::uint8_t>(layer.getOpacity() / 1.f * 255.f);

    sf::Color vertColour = sf::Color(200 ,200, 200, layerOpacity);
    auto offset = layer.getOffset();
    layerOffset = { static_cast<float>(offset.x), static_cast<float>(offset.y) };
    chunkTileCount = { tileCount.x, tileCount.y };
    mapTileSize = tileSize;

    const auto& tileIDs = layer.getTiles();

    //go through the tiles and create all arrays (for latter manipulation)
    for (const auto& ts : tilesets)
    {
      if(ts->getImagePath().empty())
      {
        tmx::Logger::log("This example does not support Collection of Images tilesets", tmx::Logger::Type::Info);
        tmx::Logger::log("Chunks using " + ts->getName() + " will not be created", tmx::Logger::Type::Info);
        continue;
      }
      m_chunkArrays.emplace_back(std::make_unique<ChunkArray>(*tr.find(ts->getImagePath())->second, *ts));
    }
    std::size_t xPos = static_cast<std::size_t>(position.x / tileSize.x);
    std::size_t yPos = static_cast<std::size_t>(position.y / tileSize.y);
    for (auto y = yPos; y < yPos + tileCount.y; ++y)
    {
      for (auto x = xPos; x < xPos + tileCount.x; ++x)
      {
        auto idx = (y * rowSize + x);
        m_chunkTileIDs.emplace_back(tileIDs[idx]);
        m_chunkColors.emplace_back(vertColour);
      }
    }
    generateTiles(true);
  }

  void generateTiles(bool registerAnimation = false)
  {
    if (registerAnimation)
    {
        m_activeAnimations.clear();
    }
    for (const auto& ca : m_chunkArrays)
    {
      std::uint32_t idx = 0;
      std::uint32_t xPos = static_cast<std::uint32_t>(getPosition().x / mapTileSize.x);
      std::uint32_t yPos = static_cast<std::uint32_t>(getPosition().y / mapTileSize.y);
      for (auto y = yPos; y < yPos + chunkTileCount.y; ++y)
      {
        for (auto x = xPos; x < xPos + chunkTileCount.x; ++x)
        {
          if (idx < m_chunkTileIDs.size() && m_chunkTileIDs[idx].ID >= ca->m_firstGID
            && m_chunkTileIDs[idx].ID <= ca->m_lastGID)
          {
            if (registerAnimation && m_animTiles.find(m_chunkTileIDs[idx].ID) != m_animTiles.end())
            {
              AnimationState as;
              as.animTile = m_animTiles[m_chunkTileIDs[idx].ID];
              as.startTime = sf::milliseconds(0);
              as.tileCords = sf::Vector2u(x,y);
              m_activeAnimations.push_back(as);
            }

            sf::Vector2f tileOffset(static_cast<float>(x) * mapTileSize.x, static_cast<float>(y) * mapTileSize.y + mapTileSize.y - ca->tileSetSize.y);

            auto idIndex = m_chunkTileIDs[idx].ID - ca->m_firstGID;
            sf::Vector2f tileIndex(sf::Vector2i(idIndex % ca->tsTileCount.x, idIndex / ca->tsTileCount.x));
            tileIndex.x *= ca->tileSetSize.x;
            tileIndex.y *= ca->tileSetSize.y;
            Tile tile =
            {
              sf::Vertex(tileOffset - getPosition(), m_chunkColors[idx], tileIndex),
              sf::Vertex(tileOffset - getPosition() + sf::Vector2f(static_cast<float>(ca->tileSetSize.x), 0.f), m_chunkColors[idx], tileIndex + sf::Vector2f(static_cast<float>(ca->tileSetSize.x), 0.f)),
              sf::Vertex(tileOffset - getPosition() + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y)), m_chunkColors[idx], tileIndex + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y))),
              sf::Vertex(tileOffset - getPosition(), m_chunkColors[idx], tileIndex),
              sf::Vertex(tileOffset - getPosition() + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y)), m_chunkColors[idx], tileIndex + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y))),
              sf::Vertex(tileOffset - getPosition() + sf::Vector2f(0.f,static_cast<float>(ca->tileSetSize.y)), m_chunkColors[idx], tileIndex + sf::Vector2f(0.f, static_cast<float>(ca->tileSetSize.y)))
            };
            doFlips(m_chunkTileIDs[idx].flipFlags,&tile[0].texCoords,&tile[1].texCoords,&tile[2].texCoords,&tile[3].texCoords,&tile[4].texCoords,&tile[5].texCoords);
            ca->addTile(tile);
          }
          idx++;
        }
      }
    }
  }
  ~Chunk() = default;
  Chunk(const Chunk&) = delete;
  Chunk& operator = (const Chunk&) = delete;

  std::vector<AnimationState>& getActiveAnimations()
  { 
    return m_activeAnimations;
  }

  tmx::TileLayer::Tile getTile(std::int32_t x, std::int32_t y) const
  {
    return m_chunkTileIDs[calcIndexFrom(x,y)];
  }

  void setTile(std::int32_t x, std::int32_t y, tmx::TileLayer::Tile tile, bool refresh)
  {
    m_chunkTileIDs[calcIndexFrom(x,y)] = tile;
    maybeRegenerate(refresh);
  }

  sf::Color getColor(std::int32_t x, std::int32_t y) const
  {
    return m_chunkColors[calcIndexFrom(x,y)];
  }

  void setColor(std::int32_t x, std::int32_t y, sf::Color color, bool refresh)
  {
    m_chunkColors[calcIndexFrom(x,y)] = color;
    maybeRegenerate(refresh);
  }

  void maybeRegenerate(bool refresh);

  std::int32_t calcIndexFrom(std::int32_t x, std::int32_t y) const;

  bool empty() const;
  
  void flipY(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5);
  void flipX(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5);
  void flipD(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5);
  void doFlips(std::uint8_t bits, sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5);

private:

  std::uint8_t layerOpacity;  // opacity of the layer
  sf::Vector2f layerOffset;   // Layer offset
  sf::Vector2u mapTileSize;   // general Tilesize of Map
  sf::Vector2f chunkTileCount;   // chunk tilecount
  std::vector<tmx::TileLayer::Tile> m_chunkTileIDs; // stores all tiles in this chunk for later manipulation
  std::vector<sf::Color> m_chunkColors; // stores colors for extended color effects
  std::map<std::uint32_t, tmx::Tileset::Tile> m_animTiles;    // animation catalogue
  std::vector<AnimationState> m_activeAnimations;     // Animations to be done in this chunk
  std::vector<ChunkArray::Ptr> m_chunkArrays;

  void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
};



class ChunkArray final : public sf::Drawable {
public:
  using Ptr = std::unique_ptr<ChunkArray>;

  tmx::Vector2u tileSetSize;
  sf::Vector2u tsTileCount;
  std::uint32_t m_firstGID, m_lastGID;

  explicit ChunkArray(const sf::Texture& t, const tmx::Tileset& ts)
      : m_texture(t)
  {
    auto texSize = getTextureSize();
    tileSetSize = ts.getTileSize();
    tsTileCount.x = texSize.x / tileSetSize.x;
    tsTileCount.y = texSize.y / tileSetSize.y;
    m_firstGID = ts.getFirstGID();
    m_lastGID = ts.getLastGID();
  }

  ~ChunkArray() = default;
  ChunkArray(const ChunkArray&) = delete;
  ChunkArray& operator = (const ChunkArray&) = delete;

  sf::Vector2u getTextureSize() const
  { 
    return m_texture.getSize();
  }

  void reset();
  void addTile(const Chunk::Tile& tile);


private:
  const sf::Texture& m_texture;
  std::vector<sf::Vertex> m_vertices;
  void draw(sf::RenderTarget& rt, sf::RenderStates states) const override;
};

#endif //SFML_ORTHO_HPP_
