# Campaign File Structure

## Summary
DMHelper currently stores all campaign content in a single XML file,
with external media referenced by path. This feature introduces a
companion `<campaign-name>_files/` directory that mirrors the campaign
tree as a directory hierarchy: every text entry has a corresponding
`.md` file; entries with children have a corresponding subdirectory;
directory-type entries have a `_contents.md`. This makes the campaign
content a valid Obsidian vault and VS Code workspace, editable
externally with changes automatically detected by DMHelper.

## User-Visible Behaviour

### Campaign creation
- The New Campaign dialog gains a "Files directory" field, pre-filled
  to `<campaign-name>_files/` relative to the chosen save location.
  The user may edit this directory name at creation time. There is no
  UI to rename it after creation.

### Campaign tree ↔ files directory mirroring
- The files directory is a structural mirror of the campaign tree:
  - Every text entry → `<entry-name>.md` in the corresponding directory.
  - Every entry that has children → a subdirectory `<entry-name>/` in
    the parent's directory.
  - Every text entry that also has children (i.e. its subdirectory
    exists) → `<entry-name>/_contents.md` as the entry's text body.
  - Non-text entries with children (e.g. a battle that contains
    combatants) → subdirectory exists but no `_contents.md`.
- On first save (or on any structural change), DMHelper verifies the
  files directory mirrors the tree. If a required subdirectory is
  missing (e.g. an entry with children has no directory on disk),
  DMHelper prompts: "Entry '<name>' has children but no directory on
  disk — create it?" before proceeding.
- `_contents.md` is always created when a text entry gains its first
  child. It is not created for non-text entries.

### Unified entry creation (no more "Add Linked Entry")
- "Add Entry" and "Add Linked Entry" are merged into a single "Add
  Entry" action. All new text entries are linked entries backed by a
  `.md` file. The `CampaignType_Text` (inline) variant is preserved
  only for legacy-loaded campaigns in legacy mode.

### EncounterTextLinked — full read/write
- `setText()` writes through to the linked `.md` file, debounced to
  the existing auto-save timer (approximately 5 s of inactivity). The
  current stub that logs an error is replaced.
- `inputXML()` calls `readLinkedFile()` on load (the commented-out
  `extractTextNode` call is completed). Loaded linked entries
  immediately have their text populated.

### Non-video media
- When the user adds an image or audio file not already inside the
  files directory, DMHelper copies it into the appropriate
  subdirectory of the files directory and updates the XML path to
  point to the copy. Standard Markdown image syntax `![](path)` is
  used in `.md` files (CommonMark-compatible, renders in Obsidian).

### Video media
- When the user adds a video file, a dialog asks whether to copy it
  into the files directory. Accepting copies and updates the path;
  declining leaves the path unchanged. No preference to suppress the
  prompt permanently.

### File collision avoidance
- If an entry name would produce a filename or directory name that
  already exists, a numeric suffix is appended (`name-2.md`,
  `name-2/`, …) until a free slot is found. The XML stores the
  actual filename, not just the entry name.
- Renaming a tree entry renames the corresponding `.md` file or
  directory (and `_contents.md` within it if applicable), regardless
  of whether DMHelper or the user originally created that file. If a
  collision occurs, the suffix strategy applies.

### External edit detection (live sync)
- `QFileSystemWatcher` is extended to watch the entire files directory
  tree (not just individually registered files). Events are:
  - **File modified**: if the modified file is a registered linked
    `.md`, reload its text.
  - **New `.md` file in a watched directory**: trigger auto-discovery
    for that location.
  - **New subdirectory**: trigger auto-discovery for the new directory
    and its contents.
  - **Deletion of a `.md` file**: log a warning; do not remove the
    entry from the tree automatically.

### Auto-discovery on open and on external change
- When a campaign is opened (and when the watcher detects additions),
  DMHelper scans the files directory for `.md` files and
  subdirectories not already referenced by any campaign entry:
  - Files directly in `<campaign>_files/` → added at campaign root.
  - Files inside a subdirectory `<campaign>_files/<dir>/` → added as
    children of the entry whose directory is `<dir>`. If `<dir>` is
    new and not in the tree, a new directory entry is created for it,
    with `_contents.md` loaded as its text if present.
  - The rule applies recursively for nested subdirectories.
  - `_contents.md` files are never created as standalone entries; they
    are treated as the text body of their parent directory entry.
  - Non-markdown files placed in the directory by the user are ignored
    by auto-discovery.
- Newly discovered entries are added to the tree. A non-blocking
  notification lists them.

### Backwards compatibility
- A campaign XML lacking a `filesDirectory` attribute opens normally.
  Before rendering the first entry, a migration dialog offers:
  - Suggested directory name (derived from campaign filename).
  - Editable directory path.
  - "Migrate now" or "Open in legacy mode".
- "Migrate now": creates the directory, converts all inline text
  entries to `.md` files, copies referenced external files (with the
  video prompt), saves the updated XML.
- "Open in legacy mode": opens as before. No migration prompts in
  that session. Inline `CampaignType_Text` entries remain inline.

### Import / Export
- Existing `CampaignExporter` is extended to include the files
  directory (or the relevant subdirectory subtree) alongside the XML.
  Export packages them into a folder structure (or zip if the user
  selects "export as archive"), matching the subtree being exported.
- Import unpacks the folder and places the files directory next to the
  campaign XML. If a files directory with the same name already exists,
  the user is prompted to merge or rename.

## Obsidian / VS Code Compatibility
- All `.md` files use UTF-8 with YAML front-matter preserved as-is.
- The files directory is a valid Obsidian vault root. No DMHelper-
  specific files outside `_contents.md` (a documented convention).
- `_contents.md` as the note for a directory maps naturally to
  Obsidian's "folder note" plugin convention, which is widely used.
- Image references use `![alt](relative-path)` (CommonMark); this
  renders in both Obsidian and Qt's Markdown renderer without custom
  parsing.
- VS Code / Copilot users open the files directory as a workspace;
  edits are picked up via `QFileSystemWatcher` when DMHelper is open.

## Subsystems
- Campaign (XML serialization, new-campaign dialog, migration)
- EncounterTextLinked (full read-write, watcher extension)
- Campaign tree (rename propagation, auto-discovery, structure mirror)
- Media handling (copy-on-add, video prompt)
- Import / Export (files directory packaging)
- UI (merge "Add Entry" / "Add Linked Entry")

## Done Conditions
1. New campaign → files directory created on first save; tree and
   directory structure stay in sync on all subsequent saves.
2. Adding a text entry and typing → `.md` file updated within the
   auto-save debounce period.
3. Editing the `.md` file externally → DMHelper text view refreshes.
4. Adding a new `.md` file or subdirectory externally → auto-
   discovery fires and adds entries to the tree.
5. Adding a non-video asset copies it to the files directory; XML
   path updates.
6. Adding a video asset shows the prompt; both choices work.
7. Entry rename → file/directory rename on disk; collision handled.
8. Entry with children but missing subdirectory → user prompted.
9. Old campaign (no `filesDirectory`) → migration dialog works;
   legacy mode works without data loss.
10. Export includes files directory; import reconstructs correctly.
11. "Add Entry" / "Add Linked Entry" are merged; only one action
    visible in UI.

## Open Questions
None — all resolved above.
