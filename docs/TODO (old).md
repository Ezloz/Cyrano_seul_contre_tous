- Map
  -> Array of Tiles ?
  -> height
  -> width
- Enum type tile
  -> (Grass | etc...)
  -> Walkability
- Tiles
  -> Type tile
  -> Vector of Entity (Characters | Objects)
  -> Position
- Characters (Player | Ennemies)
  -> Statistiques :
  -> Compétences :
  -> Action value
  -> Inventaires
  ~ Attack
  ~ Get rekt
- Objets :
  -> Types
- Turn :
  - Structure ordonnée des characters ?
    -> Vector Characters
- Game instance :
  -> UI
  -> Round
  -> Camera
  -> Music

main:
  - Load assets
  - Game loop :
    -> Player input
    -> check UI
    -> update
    -> destroy
    -> render
