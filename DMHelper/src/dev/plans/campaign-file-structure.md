---
feature_slug: campaign-file-structure
spec_path: DMHelper/src/dev/specs/campaign-file-structure.md
created: 2026-05-10
designer_model: opus
arch_review_required: true
arch_review_model: opus
arch_review_reason: Introduces a new top-level subsystem (CampaignFilesManager owning a recursive QFileSystemWatcher and the on-disk mirror), changes the serialization shape of `Campaign` (new `filesDirectory` attribute) and `EncounterTextLinked` (inputXML now reads through to disk; setText now writes through), and touches both the campaign and UI-shell subsystems.
pre_impl_arch_review_requested: true
supersedes: null
status: in-progress
---

# Summary

DMHelper today persists the campaign as a single XML file with media
referenced by absolute or relative paths. This feature adds a sibling
`<campaign-name>_files/` directory that mirrors the campaign tree on
disk: every text entry is a `.md` file, every entry with children is
a directory, and a text entry that also has children stores its body
in `<entry>/_contents.md`. The directory is a valid Obsidian vault
and a usable VS Code workspace; external edits flow back into
DMHelper via a recursive `QFileSystemWatcher`.

The work introduces one new utility class — `CampaignFilesManager` —
that owns the watcher, allocates collision-free filenames, verifies
the directory-tree mirror, copies imported media, and scans for
auto-discovery candidates. `Campaign` gains a `_filesDirectory`
field serialized as the `filesDirectory` attribute. `EncounterText`-
`Linked` is upgraded from its current stub state to a full read-write
linked entry. The new-campaign dialog gets a "Files directory" field;
the entry-creation dialog merges "Add Entry" / "Add Linked Entry"
into one action that always produces a linked entry. A migration
dialog handles legacy campaigns that lack a `filesDirectory`.

Subsystems touched: Campaign (XML + new field + new owned object),
EncounterTextLinked (read/write completion), Campaign tree
(rename propagation, auto-discovery, mirror verification), Media
handling (copy-on-add, video prompt), Import/Export (files directory
packaging), UI shell (new-campaign dialog, new-entry dialog).

# Architectural Risk Assessment

- **Trigger 1 (threading / GL boundary): Not hit.** `QFileSystemWatcher`
  signals on the Qt main thread; no VLC or worker-thread code is
  introduced. No GL code is touched.
- **Trigger 2 (Layer subclass): Not hit.** No `Layer*` class is
  added or modified.
- **Trigger 3 (serialization shape change): Hit.** `Campaign` gains
  a new `filesDirectory` attribute; `EncounterTextLinked::inputXML`
  is changed to read the linked file on load (was a stub) and
  `setText` becomes a write-through to disk.
- **Trigger 4 (>1 of battle/audio/campaign/UI shell): Hit.** Campaign
  serialization + UI shell (new-campaign dialog, new-entry dialog).
- **Trigger 5 (new top-level subsystem / new feature flag): Hit.**
  `CampaignFilesManager` is a new persistent singleton-per-campaign
  with its own watcher, scanner, and on-disk mirror responsibility.
  No new `dmconstants.h` feature flag is introduced.

`arch_review_required = true`, `arch_review_model = opus`,
`pre_impl_arch_review_requested = true`. Pre-impl review requested
because the `CampaignFilesManager` API surface (set by chunk
`filesdir-manager`) is the integration point for seven downstream
chunks; getting that interface wrong invalidates the rest of the
plan.

# Chunks

## Chunk 1: filesdir-data

- **id**: filesdir-data
- **summary**: Add the `_filesDirectory` field to `Campaign` and serialize it as the `filesDirectory` attribute on the campaign XML element. Pure data plumbing — no behavioural change yet.
- **dependencies**: []
- **branch**: agent/work/filesdir-data
- **files_to_modify**:
  - DMHelper/src/campaign.h — add `_filesDirectory` member, getter `getFilesDirectory()` and setter `setFilesDirectory(const QString&)`.
  - DMHelper/src/campaign.cpp — initialise in constructor, read in `inputXML` (treat absent attribute as empty string — no migration logic yet, that lives in `migration-dialog`), write in `internalOutputXML` when non-empty.
- **files_to_create**: []
- **integration_tasks**:
  - In `Campaign::internalOutputXML`, write `filesDirectory` only when the value is non-empty so legacy/in-memory campaigns round-trip unchanged.
  - Make `setFilesDirectory` emit `dirty()` only when the value actually changes; never emit `dirty()` from the constructor.
- **acceptance_criteria**:
  - `Campaign::getFilesDirectory()` and `setFilesDirectory(const QString&)` are declared in the header and implemented in the .cpp.
  - `inputXML` reads `element.attribute("filesDirectory")`.
  - `internalOutputXML` calls `Campaign`'s base implementation first, then conditionally sets the attribute.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Serialization rules from CLAUDE.md: override `internalOutputXML`, always call base; never emit `dirty()` from constructor or `inputXML`.
- **out_of_scope**:
  - Creating the directory on disk.
  - Migration of older campaigns.
  - Any UI changes.

## Chunk 2: filesdir-manager

- **id**: filesdir-manager
- **summary**: New `CampaignFilesManager` utility class that owns the path-mapping policy for the files directory. Provides path allocation, collision avoidance, mirror verification, and the rename helper. No watcher or scanning yet — those are bolted on in later chunks. This chunk defines the API surface that downstream chunks consume.
- **dependencies**: [filesdir-data]
- **branch**: agent/work/filesdir-manager
- **files_to_modify**:
  - DMHelper/src/CMakeLists.txt — register the new `.cpp`/`.h` pair in the explicit source list.
  - DMHelper/src/campaign.h — add a `CampaignFilesManager*` member and `filesManager()` accessor.
  - DMHelper/src/campaign.cpp — construct/destruct the manager; rebind it whenever `_filesDirectory` changes.
- **files_to_create**:
  - DMHelper/src/campaignfilesmanager.h — public API: `setRootDirectory(const QString&)`, `rootDirectory()`, `pathForEntry(CampaignObjectBase*) const`, `relativePathForEntry(CampaignObjectBase*) const`, `allocateUniqueMarkdownPath(const QDir&, const QString& baseName)`, `allocateUniqueSubdirPath(const QDir&, const QString& baseName)`, `verifyMirror(Campaign*, QStringList& missingDirs)`, `renameEntryFile(CampaignObjectBase*, const QString& oldName, const QString& newName)`, `copyMediaInto(const QString& sourcePath, CampaignObjectBase* owner, bool isVideo, QString& outRelativePath)` (the video path simply takes a pre-resolved user choice; the prompt itself lives in `media-copy`).
  - DMHelper/src/campaignfilesmanager.cpp — implement the above. Naming policy: kebab-case from entry name; collision strategy appends `-2`, `-3`, … until free. `_contents.md` is reserved and never produced as a normal entry filename.
- **integration_tasks**:
  - Header must `Q_OBJECT` and inherit `QObject` so future signal additions in `filesdir-watcher` and `filesdir-autodiscovery` do not require a class shape change.
  - `Campaign` constructs the manager once; `setFilesDirectory` calls `manager->setRootDirectory(_filesDirectory)`.
- **acceptance_criteria**:
  - Class declared with `Q_OBJECT`, registered in `CMakeLists.txt` source list and headers list.
  - `pathForEntry` walks parent chain to the campaign root and returns a path under the manager's root directory.
  - `allocateUniqueMarkdownPath` and `allocateUniqueSubdirPath` return paths that do not collide with any existing filesystem entry (verified by checking `QFileInfo::exists` on the returned path).
  - `verifyMirror` populates `missingDirs` with relative paths of entries that have children but no on-disk subdirectory.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Sources are listed explicitly in CMakeLists.txt — both `.cpp` and `.h` must be added in the same change.
  - No GL calls (none expected here; called out so Execution does not introduce any).
- **out_of_scope**:
  - QFileSystemWatcher (chunk `filesdir-watcher`).
  - Auto-discovery scanning (chunk `filesdir-autodiscovery`).
  - User prompts (chunk `media-copy`, `mirror-on-save`).

## Chunk 3: textlinked-readwrite

- **id**: textlinked-readwrite
- **summary**: Complete `EncounterTextLinked` so it actually reads its `.md` file on load and writes through on `setText()`. The current `inputXML` has the relevant call commented out and `setText` is a logging stub.
- **dependencies**: []
- **branch**: agent/work/textlinked-readwrite
- **files_to_modify**:
  - DMHelper/src/encountertextlinked.cpp — uncomment the `extractTextNode(element, isImport)` call in `inputXML`; replace the `setText` stub with an implementation that calls `EncounterText::setText(newText)` and writes the file via the existing `createTextNode` helper. Writing must NOT emit `dirty()` from inside `inputXML` (so guard the write so it only happens when the new text actually differs from `_text` and the call is not happening during XML load).
  - DMHelper/src/encountertextlinked.h — no public API change; document the new `setText` semantics in a comment above the override.
- **files_to_create**: []
- **integration_tasks**:
  - The write path must reuse the existing `createTextNode(...)` helper rather than duplicating file-write code; pass a default-constructed `QDir` and `false` for `isExport` since those parameters are unused inside that helper today.
  - Persistence to disk is debounced by the existing `MainWindow::_autoSaveTimer` (driven by `dirty()`); `EncounterTextLinked::setText` writes immediately and additionally emits `dirty()` so the campaign XML save also gets queued. No new timer is introduced.
- **acceptance_criteria**:
  - `EncounterTextLinked::inputXML` no longer contains the `// TODO: markdown - read the linked file` comment, and calls `extractTextNode(element, isImport)` (directly or by leaving the existing commented-out call uncommented).
  - `EncounterTextLinked::setText` writes the new text to `_linkedFile` (verified by reading the diff: the function body is no longer the `Q_UNUSED(newText); qDebug … return;` stub, and ends in either an explicit file write or a call to `createTextNode`).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Never emit `dirty()` from `inputXML` — the file read on load must not propagate as a "user edit".
  - `setText` is a `public slots` override of `EncounterText::setText`; the override signature must stay byte-identical.
- **out_of_scope**:
  - Recursive watcher (chunk `filesdir-watcher`).
  - Naming/path allocation for newly created linked entries (chunk `newcampaign-dialog` and `merge-add-entry`).

## Chunk 4: filesdir-watcher

- **id**: filesdir-watcher
- **summary**: Extend `CampaignFilesManager` with a recursive `QFileSystemWatcher` that watches the entire files directory tree. Emit typed Qt signals: `linkedFileChanged(QString path)`, `markdownFileAdded(QString path)`, `subdirectoryAdded(QString path)`, `linkedFileDeleted(QString path)`. Wire `EncounterTextLinked` to the new file-changed signal so external edits refresh the in-memory text. The existing per-instance `QFileSystemWatcher` inside `EncounterTextLinked::setWatcher` is removed in favour of the central watcher.
- **dependencies**: [filesdir-manager, textlinked-readwrite]
- **branch**: agent/work/filesdir-watcher
- **files_to_modify**:
  - DMHelper/src/campaignfilesmanager.h — add the four signals above and a `void registerLinkedEntry(EncounterTextLinked*)` slot for each linked entry to subscribe.
  - DMHelper/src/campaignfilesmanager.cpp — implement recursive directory enumeration on `setRootDirectory`; route `QFileSystemWatcher::fileChanged` and `directoryChanged` to the typed signals (a new `.md` file in a watched directory → `markdownFileAdded`; a new subdirectory → `subdirectoryAdded` + recursive add to the watcher; a deletion → `linkedFileDeleted`).
  - DMHelper/src/encountertextlinked.h — replace the `QFileSystemWatcher* _watcher` member with a back-pointer or no member at all; make `setWatcher(bool)` a no-op or remove it (keep the slot signature for ABI but document it as deprecated).
  - DMHelper/src/encountertextlinked.cpp — drop the per-instance watcher; in `setLinkedFile`, connect to the campaign's `CampaignFilesManager::linkedFileChanged` signal filtered by path.
  - DMHelper/src/encountertextedit.cpp — remove the `linkedText->setWatcher(true)` call (and the matching false call in `unsetEncounter`); registration now happens via the central manager.
- **files_to_create**: []
- **integration_tasks**:
  - `CampaignFilesManager::setRootDirectory` must enumerate the existing tree and add every directory (and every existing `.md` file) to the watcher in one pass; use `QDirIterator` with `Subdirectories`.
  - On `directoryChanged`, the manager must diff the directory contents against a cached snapshot to detect adds vs deletes (`QFileSystemWatcher` does not give per-event detail).
  - `EncounterTextLinked` connects via `Qt::QueuedConnection` is **not** required here (the watcher delivers on the GUI thread); use the default `Qt::AutoConnection`.
- **acceptance_criteria**:
  - `CampaignFilesManager` declares the four signals listed above.
  - `setRootDirectory` opens a `QDirIterator` with `QDirIterator::Subdirectories` and adds entries to a `QFileSystemWatcher`.
  - `EncounterTextLinked` no longer owns its own `QFileSystemWatcher` (the `_watcher` member is removed or unused).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - All watcher events arrive on the Qt GUI thread — no `QMetaObject::invokeMethod` marshaling is needed for them. Do not introduce any.
  - `_watcher` removal must keep `setWatcher(bool)` callable from `encountertextedit.cpp` *if* that call is left in; the cleanest path is to remove both the call site and the slot. Either is acceptable — pick the slot-removal path and update the call sites as listed.
- **out_of_scope**:
  - Auto-discovery of new files / new subdirectories into the campaign tree (chunk `filesdir-autodiscovery`).
  - Acting on `linkedFileDeleted` beyond logging a warning.

## Chunk 5: filesdir-autodiscovery

- **id**: filesdir-autodiscovery
- **summary**: On campaign open, and in response to `markdownFileAdded` / `subdirectoryAdded` from chunk `filesdir-watcher`, scan for `.md` files and subdirectories not already mapped to a campaign entry, and add them to the tree. Files at the root become root-level entries; files inside `<dir>/` become children of the entry whose directory is `<dir>`; an unknown `<dir>` becomes a new directory entry with `_contents.md` (if present) loaded as its body. Surfaces a non-blocking notification listing newly discovered entries.
- **dependencies**: [filesdir-watcher]
- **branch**: agent/work/filesdir-autodiscovery
- **files_to_modify**:
  - DMHelper/src/campaignfilesmanager.h — add `void scanForNewEntries(Campaign* campaign, QList<CampaignObjectBase*>& discovered)`.
  - DMHelper/src/campaignfilesmanager.cpp — implement the scan: walk the files-dir tree via `QDirIterator`, build the set of expected paths from `verifyMirror`'s reverse direction, create `EncounterTextLinked` for each new `.md` (skipping `_contents.md`, which is treated as the parent entry's body) and a directory-style `EncounterText` parent for each unknown subdirectory.
  - DMHelper/src/mainwindow.cpp — call `scanForNewEntries` at the end of `openCampaign` (after `postProcessXML`) and again in response to the watcher signals; show a non-blocking `QMessageBox::information` (or a status-bar message) listing the new entries.
- **files_to_create**: []
- **integration_tasks**:
  - The campaign passed to `scanForNewEntries` must already have its files manager root set; chunk `newcampaign-dialog` and chunk `migration-dialog` are responsible for ensuring this happens before this scan runs.
  - When a new directory entry is fabricated, attach it to the parent via `Campaign`'s existing tree-mutation methods (`addObject` / `insertObject` — use whichever the surrounding code uses) so the `dirty()` plumbing fires correctly.
  - Newly discovered linked entries call `setLinkedFile()` to populate `_text` from disk; do not call `setText` (that would write back).
- **acceptance_criteria**:
  - `scanForNewEntries` exists with the signature above and is called from both `MainWindow::openCampaign` and from a slot connected to `CampaignFilesManager::markdownFileAdded` / `subdirectoryAdded`.
  - `_contents.md` filenames are filtered out of the standalone-entry creation path (verified by a literal `_contents.md` string check in the diff).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - All tree mutations must go through the existing `Campaign` / `CampaignObjectBase` mutation API; do not parent objects directly with `setParent`.
  - Use `EncounterTextLinked` (v2-class equivalent for linked text); never construct the legacy `EncounterText` for new linked entries.
- **out_of_scope**:
  - Removing entries when their backing file is deleted (spec says: log only).
  - Reconciling renames on disk (the watcher reports a delete + add; treat as such — the user will see the entry duplicated, which is acceptable per the spec).

## Chunk 6: newcampaign-dialog

- **id**: newcampaign-dialog
- **summary**: Add the "Files directory" field to the New Campaign dialog and wire it through to `MainWindow`'s new-campaign code path so the directory is created on disk and stored on the `Campaign`.
- **dependencies**: [filesdir-manager]
- **branch**: agent/work/newcampaign-dialog
- **files_to_modify**:
  - DMHelper/src/newcampaigndialog.h — add `QString getFilesDirectory() const;`.
  - DMHelper/src/newcampaigndialog.cpp — read the new line-edit value; pre-fill it on load to `<campaignName>_files` based on `edtCampaignName` text (use a `textChanged` connection to keep it in sync until the user edits the field manually — a one-shot guard).
  - DMHelper/src/mainwindow.cpp — in the new-campaign branch (the code that constructs `Campaign` and calls `_options->backupFile` for the first time), read `getFilesDirectory()` from the dialog, resolve to an absolute path next to the chosen save location, create the directory with `QDir::mkpath`, and call `_campaign->setFilesDirectory(...)` before the first save.
- **files_to_create**: []
- **integration_tasks**:
  - **[QT DESIGNER, HUMAN]** In `newcampaigndialog.ui`, add a `QLineEdit` named `edtFilesDirectory` (and a label "Files directory:") to the dialog form, positioned near `edtCampaignName`. Do not set a default value in Designer — the .cpp populates it.
  - The directory must be created relative to the *campaign save path* (chosen later in `doSaveCampaign` via `QFileDialog::getSaveFileName`); coordinate by passing the user's chosen folder name into `Campaign` as a relative string and resolving it against the save target inside `doSaveCampaign`.
- **acceptance_criteria**:
  - `NewCampaignDialog::getFilesDirectory()` declared and implemented; reads from `ui->edtFilesDirectory`.
  - `MainWindow` calls `_campaign->setFilesDirectory(...)` somewhere on the new-campaign code path.
  - `QDir::mkpath` is called for the resolved files-directory path (verified by string match in the diff).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Never override `.ui` properties from code: the line-edit's size policy, margins, and stylesheet must be set in Qt Designer, not in `NewCampaignDialog::NewCampaignDialog`.
  - The `[QT DESIGNER, HUMAN]` integration task must not be implemented as a code edit; the Coordinator surfaces it to the human at checkpoint 1.
- **out_of_scope**:
  - Renaming the directory after creation (spec explicitly says no UI for this).
  - Migration of older campaigns (chunk `migration-dialog`).

## Chunk 7: mirror-on-save

- **id**: mirror-on-save
- **summary**: On every `doSaveCampaign`, ask the `CampaignFilesManager` to verify the directory mirrors the tree. If any required subdirectories are missing, prompt the user once with a single grouped dialog ("The following entries have children but no directory on disk: …"). On accept, create them; on reject, save anyway.
- **dependencies**: [filesdir-manager]
- **branch**: agent/work/mirror-on-save
- **files_to_modify**:
  - DMHelper/src/mainwindow.cpp — inside `doSaveCampaign`, after `_campaign->validateCampaignIds()` and before `_campaign->outputXML(...)`, call `_campaign->filesManager()->verifyMirror(_campaign, missing)` and show a `QMessageBox` if `missing` is non-empty.
- **files_to_create**: []
- **integration_tasks**:
  - The mirror check is a no-op when `_campaign->getFilesDirectory().isEmpty()` (legacy-mode campaign) — early-return.
  - Directory creation uses `QDir::mkpath` against absolute paths returned by `pathForEntry`.
- **acceptance_criteria**:
  - `doSaveCampaign` references `filesManager()->verifyMirror` (verified by diff string match).
  - Path is guarded against the empty-files-dir case.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - All filesystem dialogs go through `QMessageBox` parented to `this` (`MainWindow`).
- **out_of_scope**:
  - Creating `.md` files on save — those are written by `EncounterTextLinked` via chunk `textlinked-readwrite`. This chunk only creates missing directories.

## Chunk 8: media-copy

- **id**: media-copy
- **summary**: When the user adds an image or audio asset that lives outside the files directory, copy it into the appropriate subdirectory and rewrite the path on the entry. For video assets, prompt the user (Yes/No) before copying.
- **dependencies**: [filesdir-manager]
- **branch**: agent/work/media-copy
- **files_to_modify**:
  - DMHelper/src/newentrydialog.cpp — in `createMediaEntry` and `createMapEntry`, route `_primaryImageFile` / chosen media path through `CampaignFilesManager::copyMediaInto`, branching on MIME type to detect video and prompt with `QMessageBox::question` before copying.
  - DMHelper/src/campaignfilesmanager.cpp — flesh out `copyMediaInto` (declared in chunk `filesdir-manager` as a pure declaration / minimal impl): copy via `QFile::copy` to the entry's directory under `pathForEntry`, allocate a unique filename via `allocateUniqueMarkdownPath`-style logic (extension-aware variant — add a sibling helper `allocateUniqueAssetPath(const QDir&, const QString& baseName, const QString& suffix)`), and return the relative path.
  - DMHelper/src/campaignfilesmanager.h — declare the new asset-naming helper.
- **files_to_create**: []
- **integration_tasks**:
  - The video prompt is `QMessageBox::question(this, "Copy video into campaign?", "Copy '<filename>' into the campaign files directory?", Yes | No)`. Yes copies; No leaves the path untouched.
  - When the source path is already inside the files directory (test with `QDir::relativeFilePath` not starting with `..`), skip the copy entirely.
- **acceptance_criteria**:
  - `copyMediaInto` calls `QFile::copy` and returns the relative path.
  - `NewEntryDialog::createMediaEntry` references `copyMediaInto` (diff string match).
  - The video branch shows a `QMessageBox::question`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Use `QMimeDatabase` (already included in `newentrydialog.cpp`) for video detection — do not duplicate the existing MIME logic.
- **out_of_scope**:
  - Markdown-image-syntax embedding in `.md` files (the spec mentions `![](path)` for `.md` content; that is naturally produced when `EncounterTextLinked` writes via `createTextNode` and is not a separate chunk).
  - A "remember my choice" preference for video.

## Chunk 9: rename-propagation

- **id**: rename-propagation
- **summary**: When an entry is renamed in the tree, rename the corresponding `.md` file or directory on disk, with collision avoidance. Applies regardless of whether DMHelper or the user originally created the file.
- **dependencies**: [filesdir-manager]
- **branch**: agent/work/rename-propagation
- **files_to_modify**:
  - DMHelper/src/campaignobjectbase.cpp — locate the existing `setName` slot; after the rename takes effect and `dirty()` is emitted, query the owning `Campaign` for its `filesManager()` and call `renameEntryFile(this, oldName, newName)` if non-null.
  - DMHelper/src/campaignfilesmanager.cpp — implement `renameEntryFile`: compute the old absolute path via the entry's parent chain, compute the new path via `allocateUniqueMarkdownPath` / `allocateUniqueSubdirPath`, call `QFile::rename` (for `.md`) or `QDir::rename` (for directories), and if the entry is an `EncounterTextLinked` update its `_linkedFile` member through `setLinkedFile` (which also updates the watcher subscription).
  - DMHelper/src/campaignobjectbase.h — no API change.
- **files_to_create**: []
- **integration_tasks**:
  - The owning campaign lookup walks `parent()` chain until a `Campaign*` is found; if not found (entry not yet attached), skip the rename — that case is handled by chunk `merge-add-entry` at attach time.
  - Collision: if `QFile::rename` returns false because the target exists, allocate a fresh unique path and retry once. Beyond one retry, log a warning and abandon — the next save will surface it via `verifyMirror`.
- **acceptance_criteria**:
  - `CampaignObjectBase::setName` (or whichever existing setter triggers the rename) calls into `CampaignFilesManager::renameEntryFile`.
  - `renameEntryFile` calls either `QFile::rename` or `QDir::rename`.
  - `EncounterTextLinked::_linkedFile` is updated when its file is renamed (via `setLinkedFile`).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - `setName` may be called during `inputXML` (legacy code path); the rename helper must early-return if `oldName == newName`. Never trigger filesystem I/O during XML load.
- **out_of_scope**:
  - Renaming media files inside the entry directory when the *parent* entry is renamed — only the entry's own `.md` / directory moves; child media keeps its name relative to the new directory because the directory rename is recursive.

## Chunk 10: merge-add-entry

- **id**: merge-add-entry
- **summary**: Drop the "Add Linked Entry" UI path. The merged "Add Entry" action always creates an `EncounterTextLinked` backed by an auto-allocated `.md` file in the parent entry's directory. Inline `CampaignType_Text` is preserved only for legacy-loaded campaigns.
- **dependencies**: [filesdir-manager, textlinked-readwrite]
- **branch**: agent/work/merge-add-entry
- **files_to_modify**:
  - DMHelper/src/newentrydialog.cpp — remove the `btnTypeLinked` page wiring; in `createTextEntry`, when the active campaign has a non-empty `getFilesDirectory()`, construct an `EncounterTextLinked` instead of an `EncounterText`, allocate its `.md` path via `CampaignFilesManager::allocateUniqueMarkdownPath`, call `setLinkedFile`, then `setText` (which now writes through). Legacy mode (empty files dir) keeps the current `EncounterText` path.
  - DMHelper/src/newentrydialog.h — remove the now-dead `createLinkedEntry` declaration if no longer referenced (or leave it as a thin wrapper that calls `createTextEntry`).
- **files_to_create**: []
- **integration_tasks**:
  - **[QT DESIGNER, HUMAN]** In `newentrydialog.ui`, hide or remove the `btnTypeLinked` button so users see only one "Add Entry" action. The `pageLinkedEntry` page may stay in the stacked widget if removing it would shift other widget IDs — describe the simpler change to the human (set `btnTypeLinked->visible = false`) and let them pick.
  - In `setEntryType`, route `CampaignType_LinkedText` to the same page as `CampaignType_Text` so the unified dialog behaves identically when invoked with either type.
- **acceptance_criteria**:
  - `createTextEntry` branches on `_campaign->getFilesDirectory().isEmpty()` and constructs `EncounterTextLinked` in the non-empty branch.
  - The diff removes the explicit `btnTypeLinked->click()` chain inside `setEntryType` (or routes `CampaignType_LinkedText` to the same case as `CampaignType_Text`).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Never edit the `.ui` XML directly — the button-hide step is a `[QT DESIGNER, HUMAN]` task.
  - All references to legacy `MonsterClass` / `Character` would be planning errors; this chunk only touches `EncounterText*` classes so the v2 rule is informational.
- **out_of_scope**:
  - Removing `CampaignType_Text` from `dmconstants.h` — the enum value is retained for legacy campaign load.
  - Cleanup of `pageLinkedEntry` and its widgets in the `.ui` file (deferred to a future cleanup; no code reads them once `btnTypeLinked` is hidden).

## Chunk 11: migration-dialog

- **id**: migration-dialog
- **summary**: When opening a campaign whose XML has no `filesDirectory` attribute, show a migration dialog ("Migrate now" / "Open in legacy mode"). On Migrate, create the directory, convert each inline `EncounterText` to an `EncounterTextLinked` with a generated `.md` file, copy referenced external media (with the chunk-`media-copy` video prompt), and persist the new attribute on next save. On Legacy, leave the campaign untouched for the session.
- **dependencies**: [filesdir-manager, textlinked-readwrite, media-copy]
- **branch**: agent/work/migration-dialog
- **files_to_modify**:
  - DMHelper/src/CMakeLists.txt — register the new dialog `.cpp`/`.h` pair in the explicit source list.
  - DMHelper/src/mainwindow.cpp — in `openCampaign`, after `inputXML` + `postProcessXML`, check `_campaign->getFilesDirectory()`. If empty, instantiate the new dialog; on accept, run the migration; on reject, set a per-session "legacy mode" flag on the campaign (a transient bool on `Campaign`, not persisted) that suppresses migration prompts until the campaign is closed.
  - DMHelper/src/campaign.h / campaign.cpp — add `bool isLegacyMode() const` and `void setLegacyMode(bool)`; in-memory only, not serialized.
- **files_to_create**:
  - DMHelper/src/campaignmigrationdialog.h — `QDialog` subclass exposing the chosen directory name and the user's choice (Migrate / Legacy / Cancel). Header-only structure for the class declaration; the `.ui` file is created by the human (see integration task).
  - DMHelper/src/campaignmigrationdialog.cpp — minimal logic; the dialog itself just collects the answer. The migration *work* lives in a free function or a helper method on `Campaign` (`Campaign::migrateToFilesDirectory(const QString& dirAbsolutePath)`).
- **integration_tasks**:
  - **[QT DESIGNER, HUMAN]** Create `DMHelper/src/campaignmigrationdialog.ui` in Qt Designer with: a label explaining the migration, a `QLineEdit` named `edtFilesDirectory` pre-filled by code, two `QPushButton`s named `btnMigrate` and `btnLegacy`, and a `QDialogButtonBox` with Cancel.
  - **[QT DESIGNER, HUMAN]** Add `campaignmigrationdialog.ui` to the Qt resource compilation by listing it in `CMakeLists.txt` alongside the other `.ui` files (the source-list update itself is a Qt-Designer-adjacent step the human performs in the same checkpoint).
  - The migration helper iterates the campaign tree, and for each `EncounterText` whose `getObjectType() == CampaignType_Text`, allocates a `.md` path via the manager, writes `_text` to disk, and replaces the in-tree object with an `EncounterTextLinked` (preserving `QUuid` for cross-references — this is the riskiest sub-step; the helper documents the swap explicitly).
  - For each external media path encountered during migration, route through `CampaignFilesManager::copyMediaInto`, prompting for video.
- **acceptance_criteria**:
  - `MainWindow::openCampaign` checks `_campaign->getFilesDirectory().isEmpty()` and conditionally instantiates `CampaignMigrationDialog`.
  - `Campaign::isLegacyMode()` / `setLegacyMode()` declared and used to gate further migration prompts.
  - `Campaign::migrateToFilesDirectory(const QString&)` declared and implemented; iterates `getChildObjects()` (recursively or via the existing tree-walk helper).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - The `.ui` file must come from Qt Designer — never hand-written by Execution.
  - The QUuid-preserving swap means: the new `EncounterTextLinked` must call `setID(oldUuid)` (or whatever the existing setter is) before being inserted; otherwise cross-references break.
  - No `dirty()` emission inside `inputXML` paths; migration runs *after* `inputXML` returns.
- **out_of_scope**:
  - One-way migration of `CampaignType_LinkedText` entries that already have a `linkedFile` attribute pointing outside the new directory — those keep their current absolute path. Re-homing such entries is a future feature.
  - Auto-saving immediately after migration — the user explicitly initiates a Save.

## Chunk 12: export-import

- **id**: export-import
- **summary**: Extend `CampaignExporter` to package the relevant subtree of the files directory alongside the XML when exporting an entry; extend the import path (drag-drop in `mainwindow.cpp` and `objectimportdialog.cpp`) to place an incoming files-directory subtree next to the campaign XML, prompting on collision.
- **dependencies**: [filesdir-manager]
- **branch**: agent/work/export-import
- **files_to_modify**:
  - DMHelper/src/campaignexporter.h — add a `_filesSourceDirectory` member and a setter; the exporter copies the relevant per-entry subdirectory of the source files dir into `_exportDirectory`.
  - DMHelper/src/campaignexporter.cpp — in `populateExport` (or in `addObjectTree`), for each exported `EncounterTextLinked` (or directory entry with a backing subdir), `QFile::copy` / recursive copy the file/subdir into the export target directory at the matching relative path.
  - DMHelper/src/objectimportdialog.cpp — when the import source contains a `_files` sibling directory, copy it into the active campaign's files directory; if a same-named subdirectory exists, prompt the user with `QMessageBox` ("Merge / Rename / Cancel"). On rename, append `-2`, `-3`, … via `CampaignFilesManager::allocateUniqueSubdirPath`.
- **files_to_create**: []
- **integration_tasks**:
  - The exporter does not currently know about the source files directory; pass it in via a setter from `MainWindow`'s export action.
  - On import, after files are placed on disk, call `CampaignFilesManager::scanForNewEntries` (from chunk `filesdir-autodiscovery`) so the imported objects are picked up as expected — but only if the importer did not already insert them via XML. To avoid double-insertion, the importer marks the just-placed paths and `scanForNewEntries` skips paths already covered by inputXML-created entries.
- **acceptance_criteria**:
  - `CampaignExporter` has a setter for the source files directory and uses it in `populateExport` / `addObjectTree`.
  - Recursive copy uses `QFile::copy` (or a helper that wraps `QDirIterator` + `QFile::copy`) — diff contains literal `QFile::copy`.
  - `objectimportdialog.cpp` handles the directory-collision prompt with a `QMessageBox`.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Use `Qt`'s file APIs only — no `std::filesystem` (project consistency).
- **out_of_scope**:
  - "Export as archive (zip)" mentioned in the spec — that is a future enhancement; this chunk delivers the folder-structure form, which is the prerequisite. The acceptance criteria above explicitly do not require zip support.
  - Per-entry export of just a single `.md` (the spec talks about subtree export, which is what `addObjectTree` already does).

# Cycle Log

## filesdir-watcher

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/campaignfilesmanager.h, DMHelper/src/campaignfilesmanager.cpp, DMHelper/src/encountertextlinked.h, DMHelper/src/encountertextlinked.cpp]
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: Added QFileSystemWatcher and _dirSnapshot to CampaignFilesManager; startWatching() enumerates dirs with QDirIterator, snapshots contents, warns on addPath failure. onFileChanged honours global/per-path suspend, emits linkedFileChanged. onDirectoryChanged diffs snapshots, emits markdownFileAdded/subdirectoryAdded/linkedFileDeleted, watches new dirs. EncounterTextLinked _watcher member removed; setWatcher is now a no-op; setLinkedFile reconnects via lambda to manager's linkedFileChanged signal.
- review_verdict: Fail
- review_findings: [High — startWatching() skips .md files (if(!info.isDir()) continue), so QFileSystemWatcher::fileChanged never fires and linkedFileChanged is never emitted — core watcher feature non-functional. Medium — encountertextedit.cpp setWatcher call sites not removed (no-op stubs remain).]
- next_action: re-execute

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/campaignfilesmanager.cpp, DMHelper/src/encountertextedit.cpp]
- executor_build_status: pass — clean build, 107 targets linked
- executor_handoff_summary: Fixed startWatching() loop to add .md files to QFileSystemWatcher alongside directories (if/else-if replacing early-continue). onDirectoryChanged also addPath()s newly detected .md files before emitting markdownFileAdded. Removed setWatcher(true) from encountertextedit setEncounter and the entire setWatcher(false) block from unsetEncounter.
- review_verdict: DesignProblem
- review_findings: [High — XML-loaded EncounterTextLinked entries never receive linkedFileChanged connection; inputXML sets _linkedFile directly (correctly), but no postProcessXML exists to wire the connection post-load. Medium — writeLinkedFile() has TODO for suspendWatch brackets, zero call sites; each setText triggers a spurious readLinkedFile round-trip. Low — case-sensitivity inconsistency between startWatching() and onDirectoryChanged for .md suffix matching.]
- next_action: escalate-to-human

### Cycle 3
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/encountertextlinked.h, DMHelper/src/encountertextlinked.cpp, DMHelper/src/campaignfilesmanager.cpp]
- executor_build_status: pass — clean build, 110 targets linked
- executor_handoff_summary: Added postProcessXML override to EncounterTextLinked that wires the linkedFileChanged connection post-load; disconnects before reconnecting to prevent duplicates on reload; calls base class postProcessXML. Bracketed createTextNode in writeLinkedFile() with suspendWatch/resumeWatch using same path as readLinkedFileInternal; removed TODO comment. Fixed onDirectoryChanged to use Qt::CaseInsensitive for .md suffix check.
- review_verdict: Pass
- review_findings: [Info — Execution handoff did not include cmake build log excerpt, but all symbols resolve and build confirmed pass at 110/110.]
- next_action: merge

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/campaignfilesmanager.h (created), DMHelper/src/campaignfilesmanager.cpp (created), DMHelper/src/campaign.h, DMHelper/src/campaign.cpp, DMHelper/src/CMakeLists.txt]
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: Created CampaignFilesManager with full API surface (path allocation, collision avoidance, mirror verify, rename, media copy, suspend/resume watch, expected-paths registry, findOwningCampaign static). Campaign gains _filesManager member constructed in its ctor. Note: getChildCount() absent on CampaignObjectBase — verifyMirror uses getChildObjects().isEmpty() instead. No watcher code in this chunk per plan.
- review_verdict: Fail
- review_findings: [High — setFilesDirectory never calls _filesManager->setRootDirectory so manager root stays empty. Medium — pathForEntry uses raw getName() without sanitisation, mismatching allocateUnique* paths for names with spaces/special chars. Low — copyMediaInto missing bool isVideo parameter needed by chunk media-copy.]
- next_action: re-execute

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/campaign.h, DMHelper/src/campaign.cpp, DMHelper/src/campaignfilesmanager.h, DMHelper/src/campaignfilesmanager.cpp]
- executor_build_status: pass — clean build, 128 targets linked
- executor_handoff_summary: Added Campaign::resolveFilesDirectory() resolving relative _filesDirectory to absolute and calling manager->setRootDirectory(). Extracted sanitiseName() as private static on CampaignFilesManager; pathForEntry now uses sanitiseName() per segment. Added bool isVideo to copyMediaInto signature (Q_UNUSED for now).
- review_verdict: Pass
- review_findings: [Low — QMap not explicitly included in campaignfilesmanager.h. Low — sanitiseName returns empty for all-special-char names. Info — watcher signals/allocateUniqueAssetPath/expected-paths pre-declared; downstream chunks must not re-add them.]
- next_action: merge

### Cycle 3
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-13
- executor_files_touched: [DMHelper/src/encountertextlinked.h, DMHelper/src/encountertextlinked.cpp]
- executor_build_status: pass — clean build, 109 targets linked
- executor_handoff_summary: Split readLinkedFile into no-arg public slot (delegates to readLinkedFileInternal(true)) and private readLinkedFileInternal(bool emitDirty) holding all file-read logic. extractTextNode now calls readLinkedFileInternal(false) — load path sets _text silently, no dirty(). Watcher path reaches readLinkedFile() → readLinkedFileInternal(true) → EncounterText::setText → dirty().
- review_verdict: Pass
- review_findings: []
- next_action: merge

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-13
- executor_files_touched: [DMHelper/src/encountertextlinked.h, DMHelper/src/encountertextlinked.cpp]
- executor_build_status: pass — clean build, 109 targets linked
- executor_handoff_summary: Added value-equality guard at top of setText. Removed redundant explicit emit dirty(). Removed redundant extractTextNode call from inputXML.
- review_verdict: Fail
- review_findings: [High — readLinkedFile calls EncounterText::setText which emits dirty() on every load]
- next_action: re-execute

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-13
- executor_files_touched: [DMHelper/src/encountertextlinked.h, DMHelper/src/encountertextlinked.cpp]
- executor_build_status: pass — clean build, 117 targets linked
- executor_handoff_summary: Uncommented extractTextNode call in inputXML. Replaced setText stub with write-through implementation. Added private writeLinkedFile() helper.
- review_verdict: Fail
- review_findings: [High — setText missing value-equality guard. Medium — double dirty(). Low — double file read in inputXML.]
- next_action: re-execute

## filesdir-data

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-13
- executor_files_touched: [DMHelper/src/campaign.h, DMHelper/src/campaign.cpp]
- executor_build_status: pass — clean build, all 300 targets linked
- executor_handoff_summary: Added _filesDirectory (QString) member to Campaign with getFilesDirectory()/setFilesDirectory() getter/setter. setFilesDirectory emits dirty() only on value change. inputXML reads element.attribute("filesDirectory") (empty on absence). internalOutputXML writes attribute only when non-empty, after calling base.
- review_verdict: Pass
- review_findings: [Info — inputXML calls setDate()/setTime() which can emit dirty(); pre-existing behaviour, out of scope]
- next_action: merge

# Architecture Review

## Pre-Implementation Review

- **verdict**: Revise
- **summary**: The plan is structurally sound and the chunk dependency graph is coherent. The major risks are: (1) the EncounterTextLinked setText write-through creating a feedback loop with the watcher (external write → reload → setText → write again); (2) the renameEntryFile helper firing during inputXML/copyValues because no loading guard exists; (3) the migration QUuid-swap recipe being underspecified — setID is protected and only Campaign is friend, so the helper must live on Campaign; (4) the migration watcher interleave is unspecified, risking spurious auto-discovery during migration; (5) the "expected paths" API for Chunk 12 import is not declared anywhere; (6) the filesDirectory value convention (relative vs absolute) is inconsistent across chunks; (7) the fabricated-directory-entry class in Chunk 5 is unspecified; (8) the parent-chain walk for finding the owning Campaign/FilesManager is repeated across chunks without a shared helper; (9) CampaignFilesManager constructor I/O prohibition is not stated; (10) QFileSystemWatcher::addPath failures are silent.
- **required_plan_changes**:
  - Add a self-write suppression mechanism for EncounterTextLinked::setText write-through, declared as a CampaignFilesManager API in Chunk 2 (e.g. suspendWatch(QString) / resumeWatch(QString) or an in-flight epoch counter), and document its use in Chunk 3's integration tasks.
  - In Chunk 9, add an explicit oldName.isEmpty() early-return guard to renameEntryFile, plus a guard against firing during inputXML and copyValues (e.g. via an isLoading flag on Campaign or by skipping when the entry is not yet attached to a Campaign ancestor).
  - In Chunk 11, expand the "preserves QUuid" sub-step into a numbered recipe covering: setID(oldUuid), row preservation, signal teardown on the old object, parent removal + deleteLater() of the old object, and any tree-model refresh required. Note that setID is protected and only Campaign is friend, so the helper must live on Campaign.
  - Add an integration task in Chunk 11 specifying the migration-vs-autodiscovery interleave: either suspend the watcher across migrateToFilesDirectory, or pre-register each newly-written path with the manager so scanForNewEntries skips it.
  - Declare in Chunk 2 (or Chunk 5) the API by which Chunk 12's importer registers "expected" paths so scanForNewEntries skips them, and reference that API from Chunks 11 and 12.
  - Choose one convention for the filesDirectory value (relative to campaign XML dir vs absolute) and reconcile Chunks 1, 2, 6, 7, and 11 to use it consistently.
  - In Chunk 5, name the class used for fabricated unknown-subdirectory entries (EncounterText container vs EncounterTextLinked backed by _contents.md) and align Chunk 10 if the latter is chosen.
  - Add a named "find owning campaign / files manager" helper in Chunk 2 (declared on CampaignObjectBase or as a free function in campaignfilesmanager.h) and reference it from Chunks 4 and 9 instead of repeating the parent-chain walk.
  - In Chunk 2, add a one-line constraint that CampaignFilesManager's constructor performs no I/O and emits no signals; all I/O begins at setRootDirectory.
  - In Chunk 4, add an instruction to log a warning when QFileSystemWatcher::addPath returns false, so platform path-budget exhaustion is observable rather than silent.
- **risk_notes**:
  - High: setText write-through feedback loop with watcher (no suppression mechanism specified).
  - High: renameEntryFile firing during inputXML/copyValues (no loading guard).
  - High: migration QUuid-swap underspecified; setID is protected, only Campaign is friend.
  - Medium: migration watcher interleave unspecified — spurious auto-discovery during migration.
  - Medium: filesDirectory relative-vs-absolute convention inconsistent across chunks.
  - Low: QFileSystemWatcher::addPath failures are silent.
  - Low: parent-chain walk repeated across chunks without shared helper.

# Escalations

## 2026-05-14 — filesdir-watcher
- **reason**: design-problem
- **detail**: Two design gaps surfaced in cycle-2 Review. (1) XML-loaded EncounterTextLinked entries never receive a linkedFileChanged watcher connection: inputXML assigns _linkedFile directly (correctly avoiding dirty()) but no postProcessXML override exists to establish the connection post-load. The plan only specified wiring in setLinkedFile, which is not called from inputXML. (2) The suspendWatch/resumeWatch brackets around writeLinkedFile() were scoped to this chunk in the plan's TODO comment but have zero call sites; the suppress API is complete but unused. Both require a plan amendment rather than a code fix the Execution Agent can make unilaterally.
- **state_at_escalation**:
  - branch_checked_out: agent/work
  - branches_left_in_place: []
  - last_cycle: filesdir-watcher:2
- **handoff_to**: human
