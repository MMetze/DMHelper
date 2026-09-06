# DMHelper v3.9 Release Notes

---

## Reddit Post (r/dmhelper, r/VTT, r/DnD etc.)

---

**DMHelper v3.9 is here — and this one goes to eleven. Polygon FoW, Animated Effects, Linux & macOS ARM support, and a FoW performance overhaul that you need to try RIGHT NOW.**

---

Hey everyone! 🎲

We just dropped **DMHelper v3.9** and it's the biggest release we've done. Like, embarrassingly big. We're going to lead with the thing that absolutely needs to be experienced before you read any further:

### 🚀 Fog of War editing is now *buttery smooth*

We rebuilt the DM View rendering pipeline and the performance improvement is night-and-day. Stop reading. Go download it. Come back. We'll wait.

...

OK! Welcome back. Wasn't that nice? If you enjoyed it, consider jumping on [Patreon](https://www.patreon.com/DMHelper) — it took a *lot* of tokens to figure that one out.

Now, onto the full changelog — and there is a *lot* of new stuff:

---

### ✨ New Features

- **Polygon FoW Drawing** — erase or fill fog in any shape you can dream up. Freeform, precise, weird — go wild.
- **Animated Effects** — drop *Fire*, *Smoke*, *Glowing Lights*, and *Sparks* directly onto your maps. Your players will lose their minds.
- **Multiple Tokens per PC** — portraits, wild shape forms, lycanthrope variants, character evolutions. Each PC can have as many tokens as they need.
- **Initiative Monster Grouping** — hide or reveal an entire group of monsters with a single click. No more fumbling during the reveal.
- **Known/Unknown & Visible/Invisible status visualization** — you'll never accidentally forget to show (or hide) a beastie again. Clear DM-side indicators for every combatant.
- **Improved Camera Rectangle** — the grab handles are so much easier to grab and resize now. Small thing, massive quality of life.
- **Health Bars on Tokens** — opt-in HP bars for any token on the map. Finally.
- **Custom Initiative Events** — add lair actions, environmental hazards, countdowns, or anything else that needs a slot in the initiative order.
- **Customizable Conditions** — the default ones aren't sacred. Replace them with whatever your system (or your homebrew) actually uses.
- **Customizable Spellbooks** — works for any system, any spell list, including completely non-D&D ones.
- **Customizable Health & Initiative Rules** — for when your system does things very differently.
- **Per-field Display Formatting & Computed Fields** — arithmetic expressions right in your character sheets. Make those sheets actually *do* things.
- **Delete Multiple Effects at Once** — yes, this should have always been possible. We know. It's here now.
- **Bulk Show/Hide and Known/Unknown for Combatants** — same deal. Mass operations, finally.

---

### 🐧🍎 Platform News

And yes, by popular demand and beloved community support:

- **Linux support is here!**
- **macOS support for Apple Silicon (M-series) *and* Intel chips!**

---

### 🔧 Fixes & Improvements

- Expanded min/max HP limits
- Fixed audio settings bug for repeating videos
- Videos now loop by default; toggleable per layer in layer settings
- Performance improvements on text entry editing
- FoW editing cursor is now the correct size on first load
- Activating the grid sizer temporarily hides all visible grid layers
- Grid sizer is positioned more consistently

---

**Download:** [dm-helper.itch.io](https://dm-helper.itch.io/dm-helper) | [dm-helper.com](https://www.dm-helper.com/)  
**Community:** [Discord](https://discord.gg/Uzum3mCeDG) | [Patreon](https://www.patreon.com/DMHelper)

Happy adventuring! 🧙‍♂️

---
---

## Discord Announcement

---

**@everyone**

# 🎉 DMHelper v3.9 — BIG release. No, really. BIG.

Before you read anything else: **Fog of War editing performance has been massively improved.** Go download and try it. Come back. We'll wait.

...done? Good. Wasn't that smooth? 😄 If you loved it, show some love on [Patreon](https://www.patreon.com/DMHelper) — that one was *hard*.

Now here's everything else that's new:

---

**✨ New Features**

🔷 **Polygon FoW Drawing** — erase/fill fog in any shape imaginable  
🔥 **Animated Effects** — Fire, Smoke, Glowing Lights & Sparks on your maps  
🧙 **Multiple PC Tokens** — portraits, wild shapes, lycanthropes, evolutions  
👾 **Initiative Monster Grouping** — show/hide a whole group in one click  
👁️ **Known/Unknown & Visible/Invisible status display** — no more forgetting your hidden beasties  
📷 **Improved Camera Rectangle** — grab handles that actually grab  
❤️ **Health Bars on Tokens** — opt-in HP bars for any token  
⚔️ **Custom Initiative Events** — lair actions, traps, timers, anything  
🎯 **Customizable Conditions, Spellbooks, Health & Initiative Rules** — make DMHelper fit *your* system  
📊 **Per-field Formatting & Computed Fields** — arithmetic in your character sheets  
🗑️ **Delete multiple effects at once** — finally  
👥 **Bulk combatant show/hide & known/unknown** — also finally  

---

**🐧🍎 NEW PLATFORMS**

> Linux support is **live**!  
> macOS — both **Apple Silicon** and **Intel** — is **live**!

---

**🔧 Fixes**

- Expanded HP min/max limits
- Fixed audio bug on repeating videos
- Videos loop by default (now toggleable)
- Text entry performance improvements
- FoW cursor correct size on first load
- Grid sizer hides grid layers while active; more consistent positioning

---

**📥 Download:** https://dm-helper.itch.io/dm-helper  
**🌐 Website:** https://www.dm-helper.com/

Go break things and tell us what you find! 🎲

---
---

## GitHub Release — v3.9.0

**Tag:** `v3.9.0`  
**Title:** DMHelper v3.9 — Animated Effects, Polygon FoW, Platform Expansion & Major Performance Overhaul

---

### Overview

v3.9 is the largest feature release to date. The headline change is a complete overhaul of the DM View rendering pipeline delivering dramatically improved Fog of War editing performance. Beyond that, this release adds polygon FoW drawing, animated map effects, multi-token PC support, initiative grouping, extensive customization hooks, and — by popular demand — Linux and macOS (Apple Silicon + Intel) platform support.

---

### Performance

- **DM View rendering overhaul** — significant improvements to Fog of War editing responsiveness and general DM View interaction performance across all map sizes.

---

### New Features

| Feature | Description |
|---|---|
| Polygon FoW Drawing | Draw and erase fog in arbitrary polygon shapes |
| Animated Effects | Fire, Smoke, Glowing Lights, and Sparks effects placeable on maps |
| Multiple Tokens per PC | Supports portraits, wild shapes, lycanthropy, and character evolutions |
| Initiative Monster Grouping | Single-click show/hide for grouped initiative entries |
| Combatant Status Visualization | DM-side display of known/unknown and visible/invisible state |
| Improved Camera Rectangle | Easier grab handles for camera box resizing |
| Token Health Bars | Optional HP bar display on map tokens |
| Custom Initiative Events | Arbitrary named events (lair actions, countdowns, etc.) in initiative order |
| Customizable Conditions | Replace default conditions with system-specific or homebrew equivalents |
| Customizable Spellbooks | System-agnostic spell list configuration |
| Customizable Health & Initiative Rules | Support for non-standard health and initiative mechanics |
| Per-field Formatting & Computed Fields | Display formatting and arithmetic expressions in character sheet fields |
| Bulk Effect Deletion | Delete multiple effects in a single operation |
| Bulk Combatant Visibility | Show/hide and known/unknown state changes across multiple combatants |

---

### Platform Support

- **Linux** — initial release support
- **macOS** — Apple Silicon (ARM) and Intel builds

---

### Bug Fixes & Minor Improvements

- Expanded min/max HP value limits
- Fixed audio settings bug causing incorrect behaviour with looping videos
- Videos now default to looping; loop behaviour is now toggleable per layer in layer settings
- Performance improvements to text entry editing
- FoW editing cursor now renders at correct size on initial load
- Activating the grid sizer now temporarily hides all visible grid layers to reduce visual noise
- Grid sizer positioning is now more consistent across map configurations

---

### Downloads

| Platform | Package |
|---|---|
| Windows | `DMHelper-v3.9.0-windows-x64-installer.exe` |
| macOS (Apple Silicon) | `DMHelper-v3.9.0-macos-arm64.dmg` |
| macOS (Intel) | `DMHelper-v3.9.0-macos-x86_64.dmg` |
| Linux | `DMHelper-v3.9.0-linux-x86_64.AppImage` |

---

**Full Changelog:** [v3.8.0...v3.9.0](https://github.com/dmhelper/dmhelper/compare/v3.8.0...v3.9.0)
