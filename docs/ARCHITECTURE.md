# mod-branded-bots — Architecture

> Status: **DRAFT** — spec for GitHub issue [#89](https://github.com/and-elf/azerothcore-wotlk/issues/89).
> Nothing here is built yet. This document is the authoritative spec; tests and code follow it.

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module: a population of
autonomous **playerbots** that live in the world (roam, quest, queue RDF/BGs) and carry a
**brand** from the companion [`mod-branding`](../../mod-branding) module. The bots are **allies** —
they exist for world consistency and to make a botted raid/party feel like a real, branded group.
They have **no special loot role** and require **no bot-AI changes**.

## 1. Two independent decisions

This feature sits behind one migration and adds one adapter. They are deliberately decoupled so
neither blocks the other.

1. **Playerbots-as-root** *(migration, out of scope for this module's code)* — `mod-playerbots`
   (liyunfan1223) does **not** compile against mainline AzerothCore; it needs the **Playerbot branch
   of the core fork**. Adopting it means re-basing this fork's root onto that core. The 100-bot
   world/RDF population itself is the module's built-in `RandomBotMgr` (`playerbots.conf`:
   `AiPlayerbot.RandomBotAutologin`, `RandomBotAccountCount`, `Min/MaxRandomBots`, bracket
   distribution). No separate "bots per bracket" module is needed.
2. **Branded allies** *(this module)* — a thin, pure module-side adapter that gives each random bot
   a brand. No core patch, no playerbots patch.

`mod-branded-bots` is kept as its **own module for visual separation** (not because the logic
demands it) — the branding adapter for bots is small, but a dedicated module keeps the
`mod-playerbots` dependency out of `mod-branding` and makes the whole feature toggleable in one
place. It depends on `mod-branding` and `mod-playerbots` both being present and enabled; if either
is disabled the module is inert.

## 2. Why no patching is required

Two facts collapse the design:

- **Playerbots are real `Player*` characters.** They log in, equip gear, and take part in combat
  exactly like humans. `mod-branding`'s existing `PlayerScript` / `UnitScript` hooks already fire
  for them.
- **A brand is proc-based and lives on the equipped weapon, not on the bot's decision-making.**
  Brand effects fire off the bot's *normal* combat (swings, casts, hits taken) with zero awareness.
  So there is nothing to teach the bot — no brand-aware rotation, no target-by-school. That was the
  only thing that would have required touching `mod-playerbots`, and it is out of scope.

Consequence: the module is an **adapter only**. Same discipline as `mod-branded-mercenary`'s
`IMercSpawner` ("plug a richer backend in later without touching the rest") — we do not fork the
framework we depend on.

## 3. The "appropriate weapon" question is already solved

A brand is **not a distinct item template**. Per `mod-branding`'s `ItemBrandingMgr`, a brand is
**state etched onto whatever weapon is already equipped**:

```
bool BrandEquipped(Player* player, BrandId brand);   // brands the equipped main-hand, step 0, persists
static bool EtchEligibleSlot(uint8_t equipSlot);      // main-hand / off-hand / ranged / trinkets
enum class EtchResult { ... NoWeapon ... };           // no equipped main-hand to etch
```

Playerbots' `RandomBotMgr` already equips every bot with **class/spec/level-appropriate gear**,
including a weapon. So we do **not** generate or select weapons. We:

1. Roll a brand (§4).
2. Call `sItemBrandingMgr->BrandEquipped(bot, brand)` on the bot's already-equipped main-hand.

"Appropriate weapon" = whatever playerbots gave the bot. Zero weapon-selection logic.

### 3.1 NoWeapon / re-gear handling

If a bot has no eligible main-hand at brand time (`EtchResult::NoWeapon`, or `BrandEquipped`
returns false), defer: hook the equip-change path (`PlayerScript::OnEquip` / mirror
`ItemBrandingMgr::CacheItem`) and (re)apply the brand when an eligible weapon lands. Random bots
re-gear as they level, so the brand must **follow the bot**, not a specific item instance:

- Persist the bot's assigned `BrandId` keyed by the bot character GUID (module table, e.g.
  `branded_bots`).
- On login (`LoadEquipped`) and on equip-change, ensure the current main-hand carries the assigned
  brand; brand it if not. This keeps the brand alive across playerbot gear swaps.

## 4. Brand randomization (pure core + DI)

Follows the pure-core / adapter / policy-injection pattern of the sibling modules.

- `src/core/` — dependency-free C++20, unit-tested with GoogleTest in sub-second runs.
  - `IBotBrandPolicy` — injected strategy: given a bot's class/level/(spec) → `BrandId` (or "none").
    Default policy: uniform random over the enabled `BrandId` set (`urand`-backed adapter; the core
    takes an injected RNG so tests are deterministic).
  - Optional weighting hooks (school rarity, "no brand" chance) live behind the same interface so the
    algorithm is swappable without touching the orchestrator.
- `src/` (adapter, the only code touching the live server):
  - Detect bot-ness via the **public** playerbots API (`sRandomPlayerbotMgr->IsRandomBot(player)` /
    `GET_PLAYERBOT_AI(player)`) — read-only.
  - `BrandingBotsPlayerScript : PlayerScript` — `OnPlayerLogin`: if bot & unbranded, roll via the
    policy, persist, and `BrandEquipped`.

## 5. Interaction with #83 (raid-wide passive buff)

Issue [#83](https://github.com/and-elf/azerothcore-wotlk/issues/83) grants a raid-wide passive buff
(drop-rate / xp-rate) derived from the **highest branding proficiency** in the raid, expensive to
re-select. Requirement for this module:

- **Bots receive the raid buff** as ordinary raid members — nothing special needed; #83 projects to
  all members.
- **Bots do not drive it.** A random bot has **no earned proficiency**, so its (zero) proficiency
  must not enter the "highest in raid" calculation as a real contributor, and it must never *lower*
  the buff. Whatever branded weapon the bot carries here is cosmetic/flavor — it does not confer
  proficiency. #83's "highest proficiency" scan must treat `IsRandomBot` members as non-contributing
  recipients.

**Sequencing:** #83 is nearly complete and merges first. Because bots don't exist in the tree until
the playerbots migration, #83 as merged won't yet special-case `IsRandomBot`. This is therefore a
small **follow-up** landing with this module — a one-line guard in #83's proficiency scan — not a
change #83 must carry pre-merge. Recorded here as a compatibility constraint on #83; no code in this
module implements #83 itself.

## 6. Out of scope / parked

- **Branded raid bosses dropping branded items** — a future idea (a `CreatureScript` + loot-table
  concern, not a bot concern). Explicitly **not planned**; noted so the door stays open.
- **Brand-aware bot combat** (rotation/targeting by school) — unnecessary because brands are
  proc-based (§2). Not planned.
- **Loot from branded bots** — allies only (§ issue #89 decision).

## 7. Dependencies & build

- AzerothCore **Playerbot core fork** (WotLK 3.3.5a), C++20 (see §1.1 migration).
- `mod-playerbots` — present & enabled (population + `IsRandomBot` API).
- `mod-branding` — present & enabled (`ItemBrandingMgr::BrandEquipped`, `BrandId`).
- No core or `mod-playerbots` source modifications.

## 8. Open questions

1. **Brand distribution** — uniform over enabled schools, or weighted (rarity, "no brand" chance)?
   Behind `IBotBrandPolicy` either way; only the default changes.
2. **Off-hand / ranged / trinket etching for bots** — MVP brands main-hand only; multi-slot aggregate
   (`AggregateEtchedIntensity`) is a later toggle.
