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
# NOTE: All 12 original chunks merged. Addendum (chunks 13-15) added 2026-05-16 in response to post-implementation Architecture Review (Revise). Executing follow-up chunks.
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

# Addendum (2026-05-16, post-implementation Architecture Review)

The post-implementation Architecture Review on the merged `agent/work` HEAD
returned `Revise` with three required follow-ups. Original chunks 1–12
are merged and not rewritten. The three new chunks below address the
follow-ups, each depending on the merged work it touches.

Guiding choices made in this addendum:

- **Follow-up 1 (watcher self-write suppression)**: choose **Option (c)
  from the review** — remove the `suspendWatch`/`resumeWatch` API
  entirely. The empirical observation in the review is that
  `EncounterText::setText`'s `if(newText == getText()) return;` equality
  guard already breaks the write/reload cycle in practice, and the
  per-path counter is unfixable in its current shape because the
  `QFileSystemWatcher::fileChanged` event is queued. Option (a) and
  Option (b) both require non-trivial state (a per-path budget that
  outlives the synchronous bracket) and add complexity to a hot path
  for a problem already neutralised by the equality guard. Removing the
  API deletes dead code, simplifies the manager, and documents the real
  loop-breaker.
- **Follow-up 2 (`registerExpectedPath`/`clearExpectedPaths`)**: the
  importer's live-tree `knownPaths` filter in `scanForNewEntries` is
  already the working mechanism. Delete the unused API rather than wire
  it in — same rationale as Follow-up 1: prefer deleting dead code to
  preserving a parallel mechanism with no proven need. Both follow-ups
  together remove one consistent class of dead `CampaignFilesManager`
  API surface.
- **Follow-up 3 (Cancel branch `deleteLater`)**: trivial one-call swap,
  isolated to `MainWindow::openCampaign`.

## Chunk 13: watcher-suspend-removal

- **id**: watcher-suspend-removal
- **summary**: Remove the non-functional `suspendWatch`/`resumeWatch` API from `CampaignFilesManager` and the now-unnecessary bracketing in `EncounterTextLinked::writeLinkedFile`. Document `EncounterText::setText`'s value-equality guard as the loop-breaker for the write → watcher → reload → write cycle.
- **dependencies**: [textlinked-readwrite, filesdir-watcher]
- **branch**: agent/work/watcher-suspend-removal
- **files_to_modify**:
  - DMHelper/src/campaignfilesmanager.h — remove `suspendWatch(const QString&)`, `resumeWatch(const QString&)`, the parameterless `suspendWatch()` / `resumeWatch()` overloads, and the `_suspendedPaths` (and any related `_globallySuspended` / counter) members. Keep all other public API unchanged.
  - DMHelper/src/campaignfilesmanager.cpp — delete the implementations and the early-return guard inside `onFileChanged` that consulted `_suspendedPaths`. Add a short comment in `onFileChanged` referencing the equality guard in `EncounterText::setText` as the loop-breaker.
  - DMHelper/src/encountertextlinked.cpp — in `writeLinkedFile`, remove the `suspendWatch(absPath)` / `resumeWatch(absPath)` calls; the file write itself stays. Leave a one-line comment above the write naming the equality-guard loop-breaker so the next reader does not re-introduce a suppression scheme.
  - DMHelper/src/campaign.cpp — in `Campaign::migrateToFilesDirectory`, remove the paired parameterless `suspendWatch()` / `resumeWatch()` bracketing if present. Migration relies on the same equality-guard mechanism plus `registerExpectedPath` (which is *also* being removed by chunk 14, so this chunk leaves migration with no special suppression; that is correct because migration writes happen before the watcher root is established for the new files directory — see integration tasks).
- **files_to_create**: []
- **integration_tasks**:
  - Verify by inspection that `Campaign::migrateToFilesDirectory` writes the `.md` files **before** calling `filesManager()->setRootDirectory(absPath)` on the freshly migrated directory; if so, no watcher events can fire for those writes (the watcher has no paths yet). If the existing order calls `setRootDirectory` first, swap the order so writes precede watcher attachment. Document the chosen order in a comment.
  - The equality guard `if(newText == getText()) return;` lives in `EncounterText::setText` (existing code, not introduced here). This chunk does not modify it; it only documents reliance on it via comments at the two new comment sites above.
- **acceptance_criteria**:
  - Diff shows the `suspendWatch` and `resumeWatch` member declarations removed from `campaignfilesmanager.h`.
  - Diff shows the implementations removed from `campaignfilesmanager.cpp` and the `_suspendedPaths`-consulting branch removed from `onFileChanged`.
  - `encountertextlinked.cpp::writeLinkedFile` no longer contains the strings `suspendWatch` or `resumeWatch`.
  - `campaign.cpp::migrateToFilesDirectory` no longer contains the strings `suspendWatch` or `resumeWatch`.
  - A grep across the entire `DMHelper/src/` tree for `suspendWatch` and `resumeWatch` returns zero hits.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - This is a pure deletion plus comments — no functional change to the user-visible behaviour. Manual smoke test confirming that external `.md` edits still reload and that internal `setText` still writes through is part of the human checkpoint, not Execution's responsibility.
  - Never emit `dirty()` from `inputXML` (unchanged constraint from chunk 3).
- **out_of_scope**:
  - Alternative suppression schemes (options (a) and (b) from the review): explicitly rejected per the addendum guidance above.
  - Touching `EncounterText::setText` itself — the equality guard is pre-existing and remains the load-bearing mechanism.

## Chunk 14: expected-paths-removal

- **id**: expected-paths-removal
- **summary**: Remove the unused `registerExpectedPath` / `clearExpectedPaths` API from `CampaignFilesManager`. Document that `scanForNewEntries`'s live-tree `knownPaths` filter is the sole double-insertion guard.
- **dependencies**: [filesdir-manager, filesdir-autodiscovery, export-import]
- **branch**: agent/work/expected-paths-removal
- **files_to_modify**:
  - DMHelper/src/campaignfilesmanager.h — remove `void registerExpectedPath(const QString&)`, `void clearExpectedPaths()`, and the backing set (e.g. `_expectedPaths`). Keep all other public API unchanged.
  - DMHelper/src/campaignfilesmanager.cpp — delete the implementations; remove the `_expectedPaths` set membership check from `scanForNewEntries` (the `knownPaths` live-tree filter at lines around 185–199 remains and is now the sole mechanism). Add a one-line comment above `knownPaths` construction naming it as the canonical double-insertion guard.
  - DMHelper/src/campaign.cpp — in `Campaign::migrateToFilesDirectory`, remove any `registerExpectedPath(...)` calls that were added as defence-in-depth. The migrated entries are inserted into the tree synchronously, so by the time any watcher event fires for the new `.md` files (after the watcher root is set on the new directory), `knownPaths` will already contain them.
- **files_to_create**: []
- **integration_tasks**:
  - Confirm by inspection that `objectimportdialog.cpp::importFinished` does **not** call `registerExpectedPath` or `clearExpectedPaths` today (review finding confirms zero call sites). No change to `objectimportdialog.cpp` is required by this chunk.
  - Verify migration order: tree insertion (`addObject` for each new linked entry) must occur **before** `setRootDirectory` is called on the new files directory. If the current implementation already does this (Cycle 2 of `migration-dialog` indicates it does), no behavioural change is required beyond the API removal.
- **acceptance_criteria**:
  - Diff shows `registerExpectedPath` and `clearExpectedPaths` member declarations removed from `campaignfilesmanager.h`.
  - Diff shows the implementations removed from `campaignfilesmanager.cpp` and the `_expectedPaths` membership check removed from `scanForNewEntries`.
  - `campaign.cpp::migrateToFilesDirectory` no longer contains the strings `registerExpectedPath` or `clearExpectedPaths`.
  - A grep across `DMHelper/src/` for `registerExpectedPath` and `clearExpectedPaths` returns zero hits.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - This is the second of two dead-API removals (paired with chunk 13). Combined effect: `CampaignFilesManager`'s public surface shrinks by four methods and two members; no behavioural change.
- **out_of_scope**:
  - Wiring the API into `objectimportdialog.cpp` — the review explicitly offers "or delete the unused API" and the addendum chooses delete (see guidance above).
  - Refactoring `scanForNewEntries`'s `knownPaths` construction; it is correct as-is and only gets a clarifying comment.

## Chunk 15: migration-cancel-deletelater

- **id**: migration-cancel-deletelater
- **summary**: In `MainWindow::openCampaign`, change the migration-dialog Cancel branch from `delete _campaign;` to `_campaign->deleteLater();` to avoid dangling queued signals that may have been emitted during `inputXML`, `postProcessXML`, or `Bestiary::Instance()->finishBatchProcessing()`.
- **dependencies**: [migration-dialog]
- **branch**: agent/work/migration-cancel-deletelater
- **files_to_modify**:
  - DMHelper/src/mainwindow.cpp — locate the migration-Cancel branch in `openCampaign` (the path that runs when the user dismisses `CampaignMigrationDialog` without choosing Migrate or Legacy). Replace `delete _campaign;` with `_campaign->deleteLater();`. The assignment `_campaign = nullptr;` (if present) must immediately follow the `deleteLater` call so the pointer is cleared in the same statement sequence.
- **files_to_create**: []
- **integration_tasks**:
  - There is exactly one Cancel branch to fix; do not change any other `delete _campaign` site in the file (e.g. the close-campaign path, which runs after the GUI has had time to flush its signal queue).
  - If the surrounding code does not already null out `_campaign` after the delete, add `_campaign = nullptr;` immediately after the `deleteLater` call. `deleteLater` does not synchronously destroy the object, so the local pointer must be cleared so subsequent checks (`if(!_campaign) ...`) still behave correctly.
- **acceptance_criteria**:
  - Diff shows exactly one `delete _campaign;` → `_campaign->deleteLater();` replacement inside `MainWindow::openCampaign`, located in the migration-dialog Cancel branch.
  - `_campaign = nullptr;` follows the `deleteLater` call (already present or newly added).
  - No other `delete _campaign` occurrences in the file are modified.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Never use `delete` on a `QObject` that has had signal emissions in flight in the current event-loop turn; `deleteLater` is the canonical Qt pattern.
- **out_of_scope**:
  - Auditing other `delete` sites in `MainWindow` (the review only flagged this one).

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

## mirror-on-save

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/mainwindow.cpp]
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: Inserted mirror-check block in doSaveCampaign between the backup block and outputXML call. Guards on !_campaign->getFilesDirectory().isEmpty() && _campaign->filesManager(). Calls verifyMirror(_campaign, missing). If missing non-empty, shows QMessageBox with Create/Skip buttons. On Create, iterates missing with QDir().mkpath(dir) (relative paths — fixed in cycle 2).
- review_verdict: Fail
- review_findings: [High — QDir().mkpath(dir) resolves relative paths against process CWD rather than manager rootDirectory(); directory creation silently targets wrong location.]
- next_action: re-execute

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/mainwindow.cpp]
- executor_build_status: pass — clean build, 106 targets linked
- executor_handoff_summary: Replaced QDir().mkpath(dir) with QDir(_campaign->filesManager()->rootDirectory()).mkpath(dir) so relative paths from verifyMirror are resolved against the absolute campaign files root. Single call site fixed.
- review_verdict: Pass
- review_findings: [Info — uses relative paths from relativePathForEntry resolved via QDir(rootDirectory()) rather than absolute paths from pathForEntry directly; mathematically equivalent and acceptance criteria met.]
- next_action: merge

## rename-propagation

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/campaignfilesmanager.cpp, DMHelper/src/campaignobjectbase.cpp]
- executor_build_status: pass — clean build, 108 targets linked
- executor_handoff_summary: Implemented renameEntryFile in campaignfilesmanager.cpp (was stub). In campaignobjectbase.cpp setName, captures oldName before update then calls renameEntryFile(this, oldName, newName) if owning campaign has filesManager. Collision avoidance added. EncounterTextLinked: post-rename setLinkedFile call (fixed in cycle 2 to avoid dirty()).
- review_verdict: Fail
- review_findings: [High — renameEntryFile calls setLinkedFile which chains readLinkedFileInternal(true) → EncounterText::setText → emit dirty(). Content unchanged on rename; re-read and dirty emission both wrong.]
- next_action: re-execute

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/encountertextlinked.h, DMHelper/src/encountertextlinked.cpp, DMHelper/src/campaignfilesmanager.cpp]
- executor_build_status: pass — clean build, 110 targets linked
- executor_handoff_summary: Added EncounterTextLinked::updateLinkedFilePath(const QString&) — updates _linkedFile and re-wires watcher connection without calling readLinkedFile(). renameEntryFile now calls updateLinkedFilePath instead of setLinkedFile. Added newName.isEmpty() early-return guard.
- review_verdict: Pass
- review_findings: [Low — build status not in handoff note but confirmed 110/110. Info — plan criterion said setLinkedFile; updateLinkedFilePath used instead per coordinator instruction; architecturally correct.]
- next_action: merge

## media-copy

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/campaignfilesmanager.cpp, DMHelper/src/newentrydialog.cpp]
- executor_build_status: pass — clean build, 111 targets linked
- executor_handoff_summary: Implemented copyMediaInto in campaignfilesmanager.cpp (was stub): early-return if root empty; already-inside check via relativeFilePath; allocateUniqueAssetPath for filename; QDir::mkpath then QFile::copy; outRelativePath relative to root. In newentrydialog.cpp createMediaEntry and createMapEntry route _primaryImageFile through copyMediaInto when filesDirectory non-empty; video detected via QMimeDatabase; QMessageBox::question shown for video. Owner passed is _currentObject.
- review_verdict: Pass
- review_findings: [Low — campaignfilesmanager.h not touched (allocateUniqueAssetPath pre-declared by prior chunk; correct). Low — QMessageBox title "Copy video?" vs plan-spec "Copy video into campaign?"; cosmetic only.]
- next_action: merge

## newcampaign-dialog

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/newcampaigndialog.h, DMHelper/src/newcampaigndialog.cpp, DMHelper/src/mainwindow.cpp]
- executor_build_status: pass — verified 188/188 (shared working tree with merge-add-entry)
- executor_handoff_summary: Code was added manually by the user (not via Execution Agent dispatch). getFilesDirectory() reads edtFilesDirectory; btnBrowseFilesDirectory opens QFileDialog::getExistingDirectory; edtCampaignName textChanged auto-fills edtFilesDirectory with name_files until user manually edits (one-shot guard via textEdited); mainwindow.cpp calls setFilesDirectory on new-campaign path and QDir::mkpath in doSaveCampaign after resolveFilesDirectory.
- review_verdict: Pass
- review_findings: [Info — auto-fill guard correctly uses textEdited for user input detection. Info — directory creation deferred to doSaveCampaign when save path is known.]
- next_action: merge

## merge-add-entry

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-14
- executor_files_touched: [DMHelper/src/newentrydialog.cpp]
- executor_build_status: pass — clean build, 188 targets linked
- executor_handoff_summary: Removed all 8 ui->btnTypeLinked references from newentrydialog.cpp. CampaignType_LinkedText in setEntryType now routes to btnTypeText (unified flow). createTextEntry branches on filesDirectory non-empty: creates EncounterTextLinked with allocateUniqueMarkdownPath + setLinkedFile + setText when in file-structure mode; falls back to EncounterText for legacy mode.
- review_verdict: Pass
- review_findings: [Low — setLinkedFile before parenting means watcher not wired until reload; pre-existing architectural limitation. Low — null/Campaign _currentObject edge case for pathForEntry. Low — writeLinkedFile may fail silently if directory not yet on disk (deferred to mirror-on-save).]
- next_action: merge

## filesdir-autodiscovery

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-15
- executor_files_touched: [] (code already committed in working tree before dispatch)
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: scanForNewEntries already fully implemented. campaignfilesmanager.h declares void scanForNewEntries(Campaign*, QList<CampaignObjectBase*>&). campaignfilesmanager.cpp implements two-pass scan (dirs then .md files) with SCAN_CONTENTS_FILENAME="_contents.md" guard. mainwindow.cpp calls after postProcessXML/resolveFilesDirectory and connects markdownFileAdded/subdirectoryAdded via QueuedConnection lambdas. All tree mutations via addObject. setLinkedFile used (not setText) for linked entries.
- review_verdict: Pass
- review_findings: [Info — sort-by-path-length heuristic for directory depth; correct in common layouts. Info — _contents.md child inside discovered dir shows redundant display name; cosmetic.]
- next_action: merge (already committed)

## migration-dialog

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-15
- executor_files_touched: [DMHelper/src/campaignmigrationdialog.h, DMHelper/src/campaignmigrationdialog.cpp, DMHelper/src/campaign.h, DMHelper/src/campaign.cpp, DMHelper/src/mainwindow.cpp]
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: Implemented CampaignMigrationDialog (browse, legacy done(2), getFilesDirectory). Added Campaign::isLegacyMode/setLegacyMode (transient, no dirty). Added Campaign::migrateToFilesDirectory: creates dir, iterates tree recursively, for each EncounterText writes text to .md, creates EncounterTextLinked with same QUuid (setID), calls setLinkedFile, replaces old entry. mainwindow.cpp openCampaign shows dialog if filesDirectory empty; Accepted=migrate, done(2)=legacy, Rejected=abort.
- review_verdict: Fail
- review_findings: [Medium — isLegacyMode() has zero call sites; suppression not wired. Medium — copyMediaInto not called during migration for media paths. Low — bare magic number 2 used for legacy result.]
- next_action: re-execute

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-15
- executor_files_touched: [DMHelper/src/campaignmigrationdialog.h, DMHelper/src/campaignmigrationdialog.cpp, DMHelper/src/campaign.h, DMHelper/src/campaign.cpp, DMHelper/src/mainwindow.cpp]
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: Added !isLegacyMode() guard to openCampaign migration check. migrateToFilesDirectory now takes QWidget* parent=nullptr; migrateObjectRecursive calls copyMediaInto for non-empty getIconFile() paths outside root with video prompt when parent non-null. CampaignMigrationDialog::LegacyModeResult=2 named constant replaces bare 2 in both sites.
- review_verdict: Pass
- review_findings: [Info — handoff missing explicit build_status field.]
- next_action: merge

## export-import

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-16
- executor_files_touched: [DMHelper/src/campaignexporter.h, DMHelper/src/campaignexporter.cpp, DMHelper/src/objectimportdialog.cpp]
- executor_build_status: pass — clean build, 130 targets linked
- executor_handoff_summary: Added setFilesSourceDirectory setter + _filesSourceDirectory member. Per-entry copy logic for EncounterTextLinked in checkObjectReferences. Recursive QDirIterator+QFile::copy import logic in objectimportdialog.cpp importFinished with QMessageBox collision prompt. Executor flagged: setter approach incompatible with constructor calling populateExport() immediately — _filesSourceDirectory always empty at copy time.
- review_verdict: DesignProblem
- review_findings: [High — CampaignExporter constructor calls populateExport() during construction; setter approach makes _filesSourceDirectory permanently empty at copy time. Medium — integration task 2 references scanForNewEntries but export-import only lists filesdir-manager as dependency (filesdir-autodiscovery already committed, function exists). Low — "/_files" string literal without shared named constant.]
- next_action: escalate-to-human (human chose Option A: constructor parameter)

### Cycle 2
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-16
- executor_files_touched: [DMHelper/src/campaignexporter.h, DMHelper/src/campaignexporter.cpp, DMHelper/src/objectimportdialog.cpp]
- executor_build_status: pass — clean build, 108 targets linked
- executor_handoff_summary: Changed to constructor parameter (Option A); _filesSourceDirectory assigned before populateExport(). Per-entry copy logic in checkObjectReferences. objectimportdialog.cpp recursive copy + QMessageBox collision prompt. No CampaignExporter call site found in codebase — executor flagged CODEBASE_DRIFT.
- review_verdict: DesignProblem
- review_findings: [High — CampaignExporter has zero instantiation call sites anywhere in codebase; export-side copy is dead code. High — Cancel branch in collision handler falls through to copy loop (does not return). High — integration task 2 requires extending scanForNewEntries via campaignfilesmanager.cpp which is not in files_to_modify. Medium — QMessageBox label says "overwrite" but code skips existing files. Low — named constants at wrong scope.]
- next_action: escalate-to-human (human resolved: delete CampaignExporter, watcher handles discovery, fix bugs in cycle 3)

### Cycle 3
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-16
- executor_files_touched: [DMHelper/src/CMakeLists.txt, DMHelper/src/objectimportdialog.cpp] (campaignexporter.h/.cpp claimed deleted but still on disk)
- executor_build_status: pass — clean build, 107 targets linked
- executor_handoff_summary: CMakeLists.txt updated to remove campaignexporter.h/.cpp references. objectimportdialog.cpp: Cancel branch now returns before copy loop (Fix A); QMessageBox label updated to accurate skip description (Fix B); FILES_SUBDIR_NAME/FILES_SUBDIR_BASE moved to file scope (Fix C); subDirName initialised from FILES_SUBDIR_BASE (Fix D). Executor reported file deletion but files remain on disk.
- review_verdict: Gap (not Pass — physical deletion of campaignexporter.h/.cpp not performed by executor)
- review_findings: [High — campaignexporter.h not deleted (still on disk). High — campaignexporter.cpp not deleted (still on disk). Info — DMHelper.pro still references both files (out-of-scope qmake file).]
- next_action: escalate-to-human (cycle cap reached; human must delete two files — RESOLVED: files deleted and committed by human)

## migration-cancel-deletelater

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-16
- executor_files_touched: [DMHelper/src/mainwindow.cpp]
- executor_build_status: pass — clean build, 107 targets linked
- executor_handoff_summary: Replaced `delete _campaign;` with `_campaign->deleteLater(); _campaign = nullptr;` in the migration-Cancel branch of MainWindow::openCampaign (line ~2228). No other delete _campaign sites modified.
- review_verdict: Pass
- review_findings: []
- next_action: merge

## watcher-suspend-removal

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-16
- executor_files_touched: [DMHelper/src/campaignfilesmanager.h, DMHelper/src/campaignfilesmanager.cpp, DMHelper/src/encountertextlinked.cpp, DMHelper/src/campaign.cpp]
- executor_build_status: pass — clean build, 129 targets linked
- executor_handoff_summary: Removed suspendWatch/resumeWatch per-path and global overloads from header and implementation, _suspendedPaths and _globalSuspendCount members, and early-return guards from onFileChanged and onDirectoryChanged. Removed suspend/resume calls and unused helper vars from encountertextlinked.cpp::writeLinkedFile. Loop-breaker comments added in onFileChanged and writeLinkedFile. No suspend calls found in campaign.cpp. Executor flagged SCOPE_AMBIGUOUS: writes-before-setRootDirectory swap impossible because pathForEntry() requires _rootDirectory; watcher safety relies on queued-signal delivery after synchronous migration completes.
- review_verdict: Pass
- review_findings: [Low — integration task comment about setRootDirectory-before-writes safety rationale not added to campaign.cpp. Info — SCOPE_AMBIGUOUS migration order reasoning verified as sound.]
- next_action: merge

## expected-paths-removal

### Cycle 1
- dispatched_by: coordinator
- dispatch_timestamp: 2026-05-16
- executor_files_touched: [DMHelper/src/campaignfilesmanager.h, DMHelper/src/campaignfilesmanager.cpp]
- executor_build_status: pass — clean build, 129 targets linked
- executor_handoff_summary: Removed registerExpectedPath, clearExpectedPaths, isExpectedPath declarations and implementations. Removed _expectedPaths (QSet<QString>) member. Removed _expectedPaths check from scanForNewEntries. Added knownPaths canonical-guard comment. QSet include moved from header to .cpp. Zero calls found in campaign.cpp.
- review_verdict: Pass
- review_findings: [Info — campaign.cpp not touched (correctly zero calls to remove). Info — isExpectedPath removed as necessary collateral. Info — QSet include correctly relocated to .cpp.]
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

## Post-Implementation Review — 2026-05-16

reviewer_model: opus
verdict: Revise
summary: Implementation builds, the feature works end-to-end, and the major architectural rules (GL boundaries, no constructor I/O, CMake registration, base-class serialisation calls) are respected — but the watcher self-write suppression that Cycle 2 Required Change #1 added to the plan is dead code in the merged result, leaving the safety of `EncounterTextLinked::setText` resting on `EncounterText::setText`'s equality guard rather than the documented `suspendWatch`/`resumeWatch` mechanism.
reviewed_range: pre-feature-base..agent/work HEAD (merged at chunk-12 completion)

triggers_evaluated:
  - threading: concern: `CampaignFilesManager::suspendWatch(path)`/`resumeWatch(path)` is called synchronously around a file write, but `QFileSystemWatcher::fileChanged` is delivered asynchronously through the event loop — by the time the handler runs the suspend count has already been zeroed by `resumeWatch`, so the suppression never fires.
  - layer_interface: not-applicable (feature does not touch Layer subclasses)
  - serialization_shape: addressed — `EncounterTextLinked::createOutputXML`/`internalOutputXML` override pair calls the base, `postProcessXML` is used for the watcher reconnection (not `inputXML`), and `Campaign::inputXML` reads `_filesDirectory` via direct field assignment so the `setFilesDirectory` setter's `emit dirty()` is not triggered during load.
  - subsystem_boundary: addressed — new `CampaignFilesManager` is owned by `Campaign`, accessed via the documented `filesManager()` accessor, and the `findOwningCampaign` parent-walk helper is centralised on the manager as the cycle-2 plan required.
  - new_subsystem_or_flag: addressed — no new `dmconstants.h` feature flag added; disabled flags (`INCLUDE_NETWORK_SUPPORT`, `LAYERVIDEO_USE_OPENGL`) untouched; `CampaignFilesManager` and `CampaignMigrationDialog` source files are explicitly listed in `DMHelper/src/CMakeLists.txt` (lines 117–118, 470–471, 759).

findings:
  - High: campaignfilesmanager.cpp:474–509 — the per-path `suspendWatch`/`resumeWatch` mechanism is non-functional. `EncounterTextLinked::writeLinkedFile` (encountertextlinked.cpp:66–75) brackets the write between `suspendWatch(absPath)` and `resumeWatch(absPath)`, but both run synchronously while the `QFileSystemWatcher::fileChanged` signal is queued and delivered later — by then `_suspendedPaths` no longer contains the path, the early-return in `onFileChanged` (campaignfilesmanager.cpp:578–584) is skipped, and `linkedFileChanged` fires. The system only avoids an infinite write/reload cycle because `EncounterText::setText`'s `if(newText == getText()) return;` guard breaks it on the re-entry; the architectural safety mechanism documented in the plan is dead. (Pre-Implementation Review Cycle 2 explicitly anticipated this risk.)
  - Medium: objectimportdialog.cpp:135–192 — the importer never calls `CampaignFilesManager::registerExpectedPath` or `clearExpectedPaths`, despite these APIs being added per Pre-Implementation Review required change #5 and being publicly exposed on `CampaignFilesManager`. The importer's correctness instead relies on `scanForNewEntries`'s live-tree `knownPaths` filter (campaignfilesmanager.cpp:185–199). The planned expected-paths API is dead code.
  - Medium: mainwindow.cpp:2229–2232 — the migration-dialog Cancel branch uses raw `delete _campaign;` after `inputXML`/`postProcessXML` have run and after `Bestiary::Instance()->finishBatchProcessing()`. Any queued signal emissions or pending `deleteLater` callbacks targeting the campaign could dangle. Should be `deleteLater()`.
  - Low: campaign.cpp:629–650 — `migrateObjectRecursive` only copies `getIconFile()` external media into the files directory; other external media references (audio paths, layerscene image layers) remain pointing outside the vault. Chunk 11 documented this as out-of-scope, so this is recorded for visibility rather than as a blocker.
  - Low: campaign.cpp:686–688 — `parentObj->addObject(newLinked)` appends to the end of the parent's child list rather than replacing in-place. The plan's chunk-11 recipe step 4 asked for row-preservation, but no `insertChildObject(row)` API exists on `CampaignObjectBase`; the implementer did the only thing possible with the existing API. The plan was over-specified rather than the implementation defective — but the migrated tree's display order changes from the legacy load.
  - Low: objectimportdialog.cpp:152–162 — the merge-vs-rename collision prompt uses stock Yes/No/Cancel buttons rather than localised "Merge"/"Rename" labels.
  - Info: campaignfilesmanager.cpp:118–134 — `allocateUnique*` exhausts at suffix 99 and silently returns a colliding path. Caller writes/renames would overwrite. Pre-existing trade-off from chunk 2; not addressed here.
  - Info: campaignfilesmanager.cpp:259–273 — when `scanForNewEntries` finds an unknown directory whose parent is not in `pathToEntry`, it attaches to the campaign root instead. Sort-by-length ordering minimises the cases this triggers; recorded for visibility.
  - Info: campaign.cpp:228–238 — `Campaign::inputXML` still indirectly emits `dirty()` via `setDate`/`setTime` calls (pre-existing prior to this feature, unchanged by it).

required_followups:
  - Repair the watcher self-write suppression so it actually consumes the queued `fileChanged` event. Either (a) change `onFileChanged` to consume one suspend count per delivered event without `resumeWatch` zeroing the count first (i.e. `writeLinkedFile` calls only `suspendWatch(absPath)` and lets the next watcher event decrement), or (b) replace the per-path counter with a per-path "ignore-next-N-events" budget that survives until the FS event arrives, or (c) remove the suspend API entirely and document that the equality guard in `EncounterText::setText` is the loop-breaker. Whichever path is chosen, document it in the plan and update `EncounterTextLinked::writeLinkedFile` to match.
  - Wire the `registerExpectedPath`/`clearExpectedPaths` API into `ObjectImportDialog::importFinished` around the `_files` copy block, calling `clearExpectedPaths()` once the post-copy `scanForNewEntries` run completes — or delete the unused API from `CampaignFilesManager` and update the plan to reflect that the live-tree filter is the sole mechanism.
  - In `MainWindow::openCampaign`, change the migration-Cancel branch to `_campaign->deleteLater()` instead of `delete _campaign;` to avoid dangling queued signals fired during `inputXML`/`postProcessXML`.

## Pre-Implementation Review — Addendum 2026-05-16

reviewer_model: opus
verdict: Pass
summary: The addendum's three chunks (13: watcher-suspend-removal, 14: expected-paths-removal, 15: migration-cancel-deletelater) are pure-deletion plus a one-call swap; the guiding-choices preamble explicitly justifies Option (c) and the "delete the unused API" branch of the post-impl review, and each chunk has well-formed acceptance criteria with grep checks that catch any missed call site. Minor documentation gaps and one load-bearing assumption are recorded as Info findings rather than required changes.

triggers_evaluated:
  - threading: addressed — Chunk 13 explicitly names `EncounterText::setText`'s equality guard as the new loop-breaker and instructs comments at both the manager and the linked-text write site so future maintainers do not re-introduce a parallel suppression. Chunk 13's integration task asks the executor to verify that migration writes do not race the watcher (either by writing-before-attaching or by relying on synchronous tree population before any queued event runs).
  - layer_interface: not-applicable (addendum touches no Layer subclass).
  - serialization_shape: not-applicable (addendum touches no XML/serialisation surface; pre-existing `createOutputXML`/`internalOutputXML` overrides in `EncounterTextLinked` are unchanged).
  - subsystem_boundary: addressed — both deletions shrink `CampaignFilesManager`'s public surface; no new cross-subsystem coupling is introduced.
  - new_subsystem_or_flag: addressed — no new feature flags, no new subsystem; `INCLUDE_NETWORK_SUPPORT` and `LAYERVIDEO_USE_OPENGL` untouched.

findings:
  - Info: Chunk 13 — the equality guard `if(newText == getText()) return;` in `EncounterText::setText` is now the sole loop-breaker for the write → watcher → reload → setText cycle. Its correctness depends on `readLinkedFileInternal` producing a string identical to what `createTextNode` writes. The addendum documents the guard as the mechanism but does not add an acceptance criterion verifying round-trip identity (e.g. for representative inputs: empty, single-line, multi-line with trailing LF, with-metadata, without-metadata). Recommend the human smoke test cover this; not blocking because the property is testable post-merge and the architectural direction is sound.
  - Info: Chunks 13 and 14 — both modify `campaign.cpp::migrateToFilesDirectory` but their `dependencies` lists omit `migration-dialog` (the chunk that introduced that function). Coordinator dispatch ordering will work fine in practice because all original chunks 1–12 are already merged, but the dependency graph is incomplete for documentation purposes.
  - Info: Chunk 13 — the parenthetical rationale "no watcher events can fire (the watcher has no paths yet)" is conditional on swapping the current merged ordering (which calls `setRootDirectory` before the writes). In the current ordering, safety actually comes from synchronous tree population in `migrateToFilesDirectory` running to completion before any queued `fileChanged`/`directoryChanged` event reaches `scanForNewEntries` — by which time `knownPaths` already contains the new entries. The integration task correctly leaves the order-swap as conditional; the rationale in `files_to_modify` could be clearer that either ordering is safe under the live-tree filter.

required_plan_changes: []

# Escalations

## 2026-05-16 — export-import (third escalation — cycle cap)
- **reason**: cycle-cap-reached
- **detail**: All code fixes in objectimportdialog.cpp (Fix A–D) were correctly applied and verified by Review. CMakeLists.txt no longer references campaignexporter. However, the Execution Agent's file-deletion commands did not physically remove campaignexporter.h and campaignexporter.cpp from disk (files still exist but are excluded from the build). Cycle cap reached (3 cycles). Human must manually delete the two files to close the chunk.
- **state_at_escalation**:
  - branch_checked_out: agent/work
  - branches_left_in_place: []
  - last_cycle: export-import:3
- **handoff_to**: human

## 2026-05-16 — export-import (second escalation)
- **reason**: design-problem
- **detail**: Review Agent returned DesignProblem on cycle 2 with three blocking findings. (1) CampaignExporter has zero instantiation call sites anywhere in the codebase; no file includes campaignexporter.h except its own .cpp; the export-side file-copy feature is dead code until a call site is created. (2) The collision handler's Cancel branch in objectimportdialog.cpp does not return — control falls through to the copy loop, making Cancel identical to Yes/merge. (3) Integration task 2 requires extending CampaignFilesManager::scanForNewEntries with a path-marking skip mechanism, which would require modifying campaignfilesmanager.cpp — not listed in the chunk's files_to_modify.
- **state_at_escalation**:
  - branch_checked_out: agent/work
  - branches_left_in_place: []
  - last_cycle: export-import:2
- **handoff_to**: human

## 2026-05-16 — export-import (first escalation)
- **reason**: design-problem
- **detail**: Two design gaps surfaced in cycle-2 Review. (1) XML-loaded EncounterTextLinked entries never receive a linkedFileChanged watcher connection: inputXML assigns _linkedFile directly (correctly avoiding dirty()) but no postProcessXML override exists to establish the connection post-load. The plan only specified wiring in setLinkedFile, which is not called from inputXML. (2) The suspendWatch/resumeWatch brackets around writeLinkedFile() were scoped to this chunk in the plan's TODO comment but have zero call sites; the suppress API is complete but unused. Both require a plan amendment rather than a code fix the Execution Agent can make unilaterally.
- **state_at_escalation**:
  - branch_checked_out: agent/work
  - branches_left_in_place: []
  - last_cycle: filesdir-watcher:2
- **handoff_to**: human
