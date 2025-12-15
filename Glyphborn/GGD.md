# **GLYPHBORN — GAME DESIGN DOCUMENT (GDD v2.1)**

*A Multiplayer Historical RP Sandbox Set in the Viking World*

---

# **1. HIGH CONCEPT**

**Glyphborn** is a multiplayer, classless, historical Viking-Age sandbox that blends:

- grounded survival
- deep skill-based progression
- hand-crafted exploration
- player-driven politics
- immersive roleplay systems
- historical authenticity
- symbolic Seer visions (optional path)

Players inhabit a shared Viking world where they define their identity—warrior, trader, hunter, craftsman, raider, seer, leader, outlaw, wanderer, or something entirely unique.

There are no classes, no predetermined roles, and no forced character archetypes.

Your story is shaped by skill, reputation, relationships, and choice.

---

# **2. PLATFORM & SCOPE STATEMENT**

Glyphborn is **multiplayer by default**, but **not an MMO**.

### **Default Experience**

- 1–10 player private/co-op worlds
- RP-focused community servers
- Long-term campaign play
- No global backend or persistent MMO shards

### **Long-Term Vision**

- Community can create large dedicated servers
- Worlds can grow, evolve, and be governed by players
- Player factions form organically
- Entire Viking world (Scandinavia → England → Europe) added across years

The game prioritizes **longevity**, **emergent play**, and **replayability** over scripted content.

---

# **3. SETTING & WORLD**

## **3.1 Time Period**

10th–11th century (900–1200 AD), the height of the Viking Age.

## **3.2 Regions (Long-Term)**

The full Viking world, expanded gradually:

- Norway
- Denmark
- Sweden
- England (Northumbria, Wessex, Mercia, Danelaw)
- Scotland & Ireland
- Wales
- Frankish territories (France)
- Germanic states
- Baltic routes & Rus trade paths

## **3.3 Cultural Tone**

- Grounded historical realism
- No magic, no fantasy monsters
- Pagan beliefs treated as cultural worldview
- Glyphs reflect intuition, symbolism, stories, memory, and fate
- Political tension between Norse paganism and rising Christianity
- Distinct regional cultures & aesthetics

---

# **4. PLAYER EXPERIENCE**

## **4.1 Player Identity (Classless System)**

Players choose their identity through:

- skills used
- roles assumed
- clan affiliation
- reputation
- personal deeds

No classes—only **emergent paths**.

Possible identities:

- warrior, hunter, craftsman, raider
- diplomat, trader, hermit, outlaw
- clan leader, skald, or any mix

### **Optional Seer Path**

Unlocked via:

- rituals
- quests
- glyph stones

Seers gain symbolic visions offering insight & warnings, never magic.

---

# **5. CORE PILLARS**

## **5.1 Exploration**

A handcrafted Viking world designed for RP:

- fjords, forests, mountains, rivers
- multi-floor interiors
- seasonal & weather changes
- travel by foot, horse, and longship
- no fast travel
- meaningful, dangerous exploration

---

## **5.2 Survival**

Grounded survival:

- hunger & nutrition
- warmth vs exposure
- weather effects
- injury & infection
- fatigue & rest

Survival enhances immersion, not frustration.

---

## **5.3 Skill Progression (No Classes)**

Skills grow through use, unlocking:

- crafting recipes
- passive bonuses
- advanced techniques

### **Combat Skills**

Sword, Axe, Spear, Bow, Shield, Stealth, Unarmed, Mounted

### **Non-Combat Skills**

Woodworking, Smithing, Leatherwork, Masonry, Fishing, Sailing, Tracking, Herbalism, Cooking, Trading, Diplomacy, Leadership, Settlement Management, Animal Handling

---

## **5.4 Combat (Updated for True 3D Tile System)**

Combat in Glyphborn is **real-time**, **skill-based**, and built on a **true 3D tile grid**.

Movement is grid-based, but the world and combat interactions occur in full 3D space.

### **5.4.1 Core Combat Philosophy**

- Players attack, block, and dodge using **tile-adjacent combat**
- Attacks resolve using **3D tile offsets**, not hitboxes
- Verticality fully matters (attacking up/down slopes, platforms, stairs)
- Projectiles travel through **true 3D space** using voxel-style tracing
- Combat is **deterministic**, **readable**, and **multiplayer-safe**
- Stamina, timing, positioning, and facing define mastery

This system blends classic tile-based tactics with action timing.

---

### **5.4.2 Melee Combat (3D Tile Adjacency)**

Melee attacks hit a predefined pattern of tiles relative to:

- the player's position `(x, y, z)`
- the player’s facing direction
- the weapon’s reach
- vertical adjacency (attacking up/down layers)

#### **Weapon Examples**

**Sword – Forward Slash**

Hits:

- one tile directly ahead
- optional ±1 vertical tiles
- optional slight side arc

**Axe – Cleave**

- wide 3-tile arc
- slight vertical reach
- higher stamina cost

**Spear – Thrust**

- reaches 2–3 tiles
- narrow straight line
- ideal for formation combat

**Daggers**

- small frontal cone
- bonus from behind

**Hammer – Overhead**

- downward vertical strike
- excels from high ground

### **Vertical Melee**

Attacks can hit:

- enemies on stairs
- enemies on balconies
- enemies below cliffs
- enemies above ramps

All using 3D adjacency, not physics.

---

### **5.4.3 Blocking & Shields (3D Shield Cones)**

Shields protect tiles in a directional cone ahead of the player:

- same layer tiles
- tiles one layer up/down (depending on shield size)

Blocking rules:

- front attacks: fully blockable
- side attacks: reduced effectiveness
- rear attacks: unblockable
- vertical attacks: depends on shield type and angle

Shield skill improves:

- block timing
- stamina cost
- coverage area

---

### **5.4.4 Dodging (Tile-Based Invulnerability Movement)**

A dodge:

- moves the player 1 tile (left/right/back)
- grants a brief invulnerability window (0.1–0.2 sec)
- costs stamina
- has a cooldown reduced by agility-related skills

Dodging avoids melee and projectiles when timed well.

---

### **5.4.5 Stamina & Weight**

Stamina affects:

- attack speed
- block duration
- dodge responsiveness
- weapon recovery
- sprinting between tiles

Armor weight affects:

- movement speed
- stamina drain
- dodge speed

---

### **5.4.6 Injuries**

Hits may inflict:

- bleeding
- broken bones
- reduced stamina regen
- slowed tile movement
- impaired ranged accuracy

Healing requires herbalism, bandages, splints, or rest.

---

## **5.4.7 Ranged Combat (True 3D Projectiles)**

Ranged combat uses **3D voxel trajectory tracing**:

Projectiles:

- travel through `(x, y, z)` space
- step tile-by-tile
- stop on collision
- embed in surfaces
- can pass under bridges, over walls, through windows
- follow realistic drop using simplified gravity

This system enables:

- real line-of-sight
- elevation advantage
- tactical placement

### **Projectile Types**

- arrows
- javelins
- throwing axes
- throwing knives
- fire arrows
- sling stones

Each has unique arc, damage, speed, and range.

### **Elevation Effects**

- shooting downhill: easier, straighter
- shooting uphill: requires more arc, more skill
- high ground = significant tactical advantage

---

## **5.4.8 Aiming Indicator**

A subtle predicted impact marker appears for the shooter:

- faint circle, dust puff, or shimmer
- precision depends on skill
- low skill = wide, uncertain indicator
- high skill = tight, accurate marker

Seers may see light rune hints (optional, symbolic).

This keeps ranged combat fair and intuitive.

---

## **5.4.9 NPC Combat AI (3D Tile Awareness)**

NPCs use the same 3D tile logic:

- take high ground
- use ranged ambushes
- flank through alternate layers
- climb stairs/ramp tiles
- retreat based on stamina
- maintain shield walls or spear formations

All AI behavior is deterministic and tile-accurate.

---

## **5.5 RP & Social Systems**

### **Emotes & Gestures**

- sitting, kneeling, resting
- toasting & feasting
- greetings
- prayers
- ritual stances

### **Multi-Character Interactions**

- duels
- restraints/arrests
- dances
- trials & oaths

### **Clans & Factions**

- player-created clans
- banners, names, hierarchy (Jarl → Thrall)
- laws, roles, feasts, taxes
- clan identity & politics

### **Reputation**

- personal reputation
- clan standing
- regional reputation

### **Law & Crime**

- outlaw status
- bounties
- trials & punishments
- exile
- wergild (blood price)

## **5.6 Crafting & Economy**

Authentic Norse crafting:

- forging, woodwork, leatherwork
- masonry & construction
- boatbuilding
- food preservation

Economy is:

- barter-driven
- regional
- social
- trade voyages & markets

---

## **5.7 Story & Narrative**

Emergent narrative with light authored arcs.

Themes:

- fate vs free will
- tradition vs faith
- community vs ambition
- honor vs survival

Structure:

- region arcs
- clan stories
- seasonal events
- Seer visions with symbolic clues

---

# **6. MULTIPLAYER DESIGN**

## **6.1 Philosophy**

Multiplayer enhances RP and exploration.

Small-scale servers—not MMO.

### **Default**

2–10 players

Private or co-op

Shared progression

### **Community**

- RP towns
- creative worlds
- war servers
- modded expansions

---

## **6.2 Player Interaction**

- group hunting, fishing, raiding
- clan settlement building
- player-run markets
- diplomacy & alliances
- feasts & ceremonies
- player-driven warfare

---

# **7. PROGRESSION**

## **7.1 Character Progression**

Based on:

- skills
- reputation
- relationships
- accomplishments
- clan prestige
- homes & possessions

No levels—only growth through action.

---

## **7.2 Equipment Progression**

Grounded, realistic gear:

- leather, mail, gambeson
- wooden round shields
- spears, axes, swords, bows
- durability & weight matter

No magical items.

---

# **8. ACTIVITIES**

Players can:

- raid towns & monasteries
- lead trade voyages
- form clans
- host feasts
- build settlements
- craft ships, armor, tools
- hunt legendary animals
- discover glyph stones
- rule or wander
- become allies or outlaws

---

# **9. ENDGAME / LONG-TERM GOALS**

There is no “ending.” Paths include:

- become a Jarl
- lead a powerful clan
- master all crafts
- become a feared raider
- become a wise Seer
- unify regions diplomatically
- create a trade empire
- explore every land
- build generational legends

Designed for years of emergent storytelling.

---

# **10. ROADMAP (Design-Level)**

## **Phase 1 — Foundational Region**

- survival
- crafting
- exploration & combat
- basic RP systems
- skill progression
- one major region (e.g., Norway)

## **Phase 2 — Multiplayer RP Expansion**

- clans
- diplomacy
- emotes
- reputation
- settlements
- first Seer arc

## **Phase 3 — World Expansion I**

- Denmark & Sweden
- new biomes & materials
- cultural differences
- seafaring improvements

## **Phase 4 — World Expansion II**

- England
- monasteries
- Anglo-Saxon towns
- political arcs

## **Phase 5+ — Europe, Scotland, Ireland, etc.**

A living, expanding Viking world.
