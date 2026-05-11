---
feature_slug: campaign-file-structure
spec_path: DMHelper/src/dev/specs/campaign-file-structure.md
created: 2026-05-10
designer_model: opus
arch_review_required: true
arch_review_model: opus
arch_review_reason: Introduces a new top-level subsystem (CampaignFilesManager owning a recursive QFileSystemWatcher and the on-disk mirror), changes the serialization shape of `Campaign` (new `filesDirectory` attribute) and `EncounterTextLinked` (inputXML now reads through to disk; setText now writes through), and touches both the campaign and UI-shell subsystems.
pre_impl_arch_review_requested: true
supersedes: DMHelper/src/dev/plans/campaign-file-structure.md
status: draft
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

# Replanning Rationale

This revision addresses the 10 `required_plan_changes` from the
pre-implementation Architecture Review (verdict: `Revise`). The chunk
structure and dependency graph are unchanged. Concrete edits:

- **Chunk 1**: Pin the storage convention for `filesDirectory` to
  *relative to the campaign XML's parent directory*, and require
  callers (chunks 6, 7, 11) to resolve to absolute paths via
  `QFileInfo(campaignXmlPath).absoluteDir().absoluteFilePath(...)`
  before passing into `CampaignFilesManager`.
- **Chunk 2**: Expand the `CampaignFilesManager` API surface with
  three additions consumed by downstream chunks: write-suppression
  (`suspendWatch` / `resumeWatch`), expected-paths registration
  (`registerExpectedPath` / `clearExpectedPaths`), and a static
  `findOwningCampaign(const CampaignObjectBase*)` helper. Add a
  no-I/O-in-constructor constraint.
- **Chunk 3**: Bracket the `EncounterTextLinked::setText` write with
  `suspendWatch` / `resumeWatch` so the central watcher does not
  re-read the file we just wrote.
- **Chunk 4**: Add a `qWarning` when `QFileSystemWatcher::addPath`
  returns false; reference the new `findOwningCampaign` helper.
- **Chunk 5**: Pin the class mapping for fabricated entries
  (unknown subdirectory → `EncounterText` container; `_contents.md`
  → child `EncounterTextLinked` body entry) and align Chunk 10 to
  hide the `_contents.md` body-entry type from the UI.
- **Chunk 9**: Add explicit early-return guards (`oldName.isEmpty()`,
  no Campaign ancestor) covering both `inputXML` and `copyValues`;
  use `findOwningCampaign` instead of an ad-hoc parent walk.
- **Chunk 11**: Replace the prose QUuid-preserving step with a
  numbered six-step recipe; require `migrateToFilesDirectory` to be
  a method on `Campaign` (so it has friend access to the protected
  `setID`); bracket the migration with `suspendWatch` / `resumeWatch`.
- **Chunk 12**: Use `registerExpectedPath` for each file placed by
  the importer so `scanForNewEntries` does not double-insert them.

No prior cycle-log entries exist (this is the first revision after
the pre-impl review; no chunk has been dispatched).

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
  - **Storage convention (pinned)**: `_filesDirectory` is stored as a path *relative to the campaign XML file's parent directory*. `inputXML` and `internalOutputXML` read/write the raw relative string. Resolution to an absolute path is the caller's responsibility (chunks 6, 7, 11) via `QFileInfo(campaignXmlPath).absoluteDir().absoluteFilePath(_filesDirectory)`. `Campaign::setFilesDirectory` therefore accepts and stores the relative form only.
- **out_of_scope**:
  - Creating the directory on disk.
  - Migration of older campaigns.
  - Any UI changes.
  - Any path resolution — callers resolve.

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
  - DMHelper/src/campaignfilesmanager.h — public API: `setRootDirectory(const QString& absolutePath)`, `rootDirectory()`, `pathForEntry(CampaignObjectBase*) const`, `relativePathForEntry(CampaignObjectBase*) const`, `allocateUniqueMarkdownPath(const QDir&, const QString& baseName)`, `allocateUniqueSubdirPath(const QDir&, const QString& baseName)`, `verifyMirror(Campaign*, QStringList& missingDirs)`, `renameEntryFile(CampaignObjectBase*, const QString& oldName, const QString& newName)`, `copyMediaInto(const QString& sourcePath, CampaignObjectBase* owner, bool isVideo, QString& outRelativePath)` (the video path simply takes a pre-resolved user choice; the prompt itself lives in `media-copy`). Additionally:
    - `void suspendWatch(const QString& absolutePath)` and `void resumeWatch(const QString& absolutePath)` — reference-counted per-path suppression of the next `linkedFileChanged` emission for that path. While suspended, file-modified events for that path are swallowed; resume decrements the count and re-arms emission when it reaches zero. A parameterless overload (`suspendWatch()` / `resumeWatch()`) suspends/resumes the entire watcher (used by chunk 11 during migration).
    - `void registerExpectedPath(const QString& absolutePath)` and `void clearExpectedPaths()` — maintain a set of paths that `scanForNewEntries` (chunk 5) must skip. Used by chunk 11 (migration writes) and chunk 12 (importer-placed files) to prevent double-insertion when those files appear via watcher events.
    - `static Campaign* findOwningCampaign(const CampaignObjectBase* entry)` — walks `parent()` until a `Campaign*` is found; returns `nullptr` if the entry is not yet attached to a tree. Consumed by chunks 4 and 9.
  - DMHelper/src/campaignfilesmanager.cpp — implement the above. Naming policy: kebab-case from entry name; collision strategy appends `-2`, `-3`, … until free. `_contents.md` is reserved and never produced as a normal entry filename.
- **integration_tasks**:
  - Header must `Q_OBJECT` and inherit `QObject` so future signal additions in `filesdir-watcher` and `filesdir-autodiscovery` do not require a class shape change.
  - `Campaign` constructs the manager once; `setFilesDirectory` calls `manager->setRootDirectory(_filesDirectory)`.
- **acceptance_criteria**:
  - Class declared with `Q_OBJECT`, registered in `CMakeLists.txt` source list and headers list.
  - `pathForEntry` walks parent chain to the campaign root and returns a path under the manager's root directory.
  - `allocateUniqueMarkdownPath` and `allocateUniqueSubdirPath` return paths that do not collide with any existing filesystem entry (verified by checking `QFileInfo::exists` on the returned path).
  - `verifyMirror` populates `missingDirs` with relative paths of entries that have children but no on-disk subdirectory.
  - `suspendWatch` / `resumeWatch` (both overloads), `registerExpectedPath` / `clearExpectedPaths`, and the static `findOwningCampaign` are all declared in the header.
  - `setRootDirectory` accepts an *absolute* path (callers resolve before invoking).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Sources are listed explicitly in CMakeLists.txt — both `.cpp` and `.h` must be added in the same change.
  - No GL calls (none expected here; called out so Execution does not introduce any).
  - **No I/O in constructor**: `CampaignFilesManager`'s constructor performs no I/O and emits no signals; all I/O and signal connections begin at `setRootDirectory`.
  - `findOwningCampaign` must tolerate `nullptr` input and any non-attached entry (return `nullptr`).
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
  - **Bracket the file write with watcher suppression**: before calling `createTextNode`, call `CampaignFilesManager::suspendWatch(_linkedFile)` on the owning campaign's manager (looked up via `CampaignFilesManager::findOwningCampaign(this)->filesManager()`); after the write, call `resumeWatch(_linkedFile)`. Use a small RAII guard or paired calls in a single function. If the manager is `nullptr` (entry not yet attached, or legacy-mode campaign), skip the suspend/resume but still perform the write — the watcher is not active in that case.
  - Persistence to disk is *not* additionally debounced by `MainWindow::_autoSaveTimer`; `EncounterTextLinked::setText` writes immediately and additionally emits `dirty()` so the campaign XML save also gets queued. No new timer is introduced.
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
  - On every `QFileSystemWatcher::addPath(path)` call (initial enumeration and incremental adds for new subdirectories), check the return value; if it returns `false`, emit `qWarning() << "[CampaignFilesManager] addPath failed for:" << path` so platform path-budget exhaustion is observable rather than silent.
  - On `directoryChanged`, the manager must diff the directory contents against a cached snapshot to detect adds vs deletes (`QFileSystemWatcher` does not give per-event detail).
  - On `fileChanged`, consult the suspend-count for that path (chunk 2's `suspendWatch`) before emitting `linkedFileChanged`; swallow the event if suspended.
  - `EncounterTextLinked` connects via `Qt::QueuedConnection` is **not** required here (the watcher delivers on the GUI thread); use the default `Qt::AutoConnection`.
  - `EncounterTextLinked::setLinkedFile` looks up the owning manager via `CampaignFilesManager::findOwningCampaign(this)`; do not duplicate the parent-walk inline.
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
  - DMHelper/src/campaignfilesmanager.cpp — implement the scan: walk the files-dir tree via `QDirIterator`, build the set of expected paths from `verifyMirror`'s reverse direction *plus* the registered-expected-paths set (chunk 2's `registerExpectedPath`), and:
    - For each unknown subdirectory, fabricate an `EncounterText` *container* entry (object type `CampaignType_Text`, no `linkedFile`, empty body) named after the subdirectory — this is the project's existing folder/text dual-purpose type. If the subdirectory contains a `_contents.md`, additionally fabricate an `EncounterTextLinked` *child* of that container with `setLinkedFile` pointing at `_contents.md`; this child is the directory's "body entry". The body-entry's name is the reserved literal `_contents` and its presence is what promotes the parent container to a text-bearing entry.
    - For each unknown `.md` file at the root or under a known subdirectory (but **not** named `_contents.md`), fabricate a stand-alone `EncounterTextLinked`.
    - Skip every path present in the registered-expected-paths set.
  - DMHelper/src/mainwindow.cpp — call `scanForNewEntries` at the end of `openCampaign` (after `postProcessXML`) and again in response to the watcher signals; show a non-blocking `QMessageBox::information` (or a status-bar message) listing the new entries.
- **files_to_create**: []
- **integration_tasks**:
  - The campaign passed to `scanForNewEntries` must already have its files manager root set; chunk `newcampaign-dialog` and chunk `migration-dialog` are responsible for ensuring this happens before this scan runs.
  - When a new directory entry is fabricated, attach it to the parent via `Campaign`'s existing tree-mutation methods (`addObject` / `insertObject` — use whichever the surrounding code uses) so the `dirty()` plumbing fires correctly.
  - Newly discovered linked entries call `setLinkedFile()` to populate `_text` from disk; do not call `setText` (that would write back).
  - Class mapping (pinned): unknown subdirectory → `EncounterText` container; `_contents.md` inside that subdirectory → child `EncounterTextLinked` with reserved name `_contents`. Stand-alone `.md` (anywhere except `_contents.md`) → `EncounterTextLinked`.
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
  - The dialog stores and returns the value as a *relative* directory name. Resolution to an absolute path happens in `MainWindow` after the user picks the campaign save location: `QString absFilesDir = QFileInfo(_campaignFileName).absoluteDir().absoluteFilePath(dialog.getFilesDirectory());` — then `QDir::mkpath(absFilesDir)` and `_campaign->setFilesDirectory(dialog.getFilesDirectory())` (relative form). The manager is then bound via `_campaign->filesManager()->setRootDirectory(absFilesDir)`.
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
  - Resolve the relative `getFilesDirectory()` to an absolute path before invoking the manager: `QString absFilesDir = QFileInfo(_campaignFileName).absoluteDir().absoluteFilePath(_campaign->getFilesDirectory());`. Pass `absFilesDir` (or paths derived from it via `pathForEntry`) to `QDir::mkpath`.
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
  - Use `CampaignFilesManager::findOwningCampaign(this)` (chunk 2's static helper) to look up the owning campaign and its manager. Do not duplicate the parent-walk inline.
  - **Early-return guards** (in this exact order, all in `renameEntryFile`):
    1. If `oldName.isEmpty()`, return immediately. Newly-created entries that have not yet been named on disk have no file to rename.
    2. If `oldName == newName`, return immediately.
    3. If `findOwningCampaign(entry) == nullptr`, return immediately. This case naturally covers both `inputXML` and `copyValues`: in both code paths the entry is constructed but not yet attached to the campaign tree, so the parent-walk does not reach a `Campaign*`. This single guard is the chosen mechanism for suppressing rename I/O during XML load and value-copy.
    4. If the manager's root directory is empty (legacy-mode campaign), return immediately.
  - Collision: if `QFile::rename` returns false because the target exists, allocate a fresh unique path and retry once. Beyond one retry, log a warning and abandon — the next save will surface it via `verifyMirror`.
- **acceptance_criteria**:
  - `CampaignObjectBase::setName` (or whichever existing setter triggers the rename) calls into `CampaignFilesManager::renameEntryFile`.
  - `renameEntryFile` calls either `QFile::rename` or `QDir::rename`.
  - `EncounterTextLinked::_linkedFile` is updated when its file is renamed (via `setLinkedFile`).
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - `setName` may be called during `inputXML` (legacy code path) and during `copyValues`; the rename helper guards against both cases by checking `findOwningCampaign(this) == nullptr` (entry not yet attached to a tree). Never trigger filesystem I/O during XML load or value-copy.
  - All references to legacy `MonsterClass` / `Character` would be planning errors; this chunk only touches text/encounter classes so the v2 rule is informational.
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
  - The body-entry `EncounterTextLinked` named `_contents` (fabricated by chunk 5's auto-discovery for directories that have a `_contents.md`) is **never** offered as an addable entry type in this dialog. The reserved name `_contents` is also rejected by the entry-name validation (`validateNewEntry`) to prevent the user from manually creating a name that collides with the body-entry convention.
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
  - DMHelper/src/campaignmigrationdialog.cpp — minimal logic; the dialog itself just collects the answer. The migration *work* lives on `Campaign` itself as `Campaign::migrateToFilesDirectory(const QString& dirAbsolutePath)` (it **must** be a method on `Campaign`, not a free function, because it calls the protected `CampaignObjectBase::setID` and only `Campaign` is friend).
- **integration_tasks**:
  - **[QT DESIGNER, HUMAN]** Create `DMHelper/src/campaignmigrationdialog.ui` in Qt Designer with: a label explaining the migration, a `QLineEdit` named `edtFilesDirectory` pre-filled by code, two `QPushButton`s named `btnMigrate` and `btnLegacy`, and a `QDialogButtonBox` with Cancel.
  - **[QT DESIGNER, HUMAN]** Add `campaignmigrationdialog.ui` to the Qt resource compilation by listing it in `CMakeLists.txt` alongside the other `.ui` files (the source-list update itself is a Qt-Designer-adjacent step the human performs in the same checkpoint).
  - **Resolution**: `MainWindow::openCampaign` resolves the user's chosen relative directory to an absolute path via `QFileInfo(_campaignFileName).absoluteDir().absoluteFilePath(dialog.getFilesDirectory())` before calling `Campaign::migrateToFilesDirectory(absPath)`. After migration succeeds, call `_campaign->setFilesDirectory(dialog.getFilesDirectory())` (relative form) and `_campaign->filesManager()->setRootDirectory(absPath)`.
  - **Watcher interleave**: bracket the entire migration call with `_campaign->filesManager()->suspendWatch()` (parameterless overload from chunk 2) before `migrateToFilesDirectory` and `resumeWatch()` after. This blanket suspension is correct because the campaign is mid-migration and not yet user-active. As a defence-in-depth measure, the migration code also calls `manager->registerExpectedPath(absMdPath)` for every `.md` file it writes, so any watcher event that *does* sneak through (or fires later when the watcher resumes) is filtered out by `scanForNewEntries`.
  - **Numbered QUuid-preserving swap recipe** — for each inline `EncounterText` whose `getObjectType() == CampaignType_Text` that is being converted to a linked entry, perform these steps in order:
    1. Construct the new `EncounterTextLinked` with the same parent (`auto* newEntry = new EncounterTextLinked(oldEntry->getName(), oldEntry->parent());`).
    2. Call `newEntry->setID(oldEntry->getID())` — `setID` is protected on `CampaignObjectBase`; only `Campaign` is friend, which is why this helper lives on `Campaign`.
    3. Copy any other relevant state from `oldEntry` to `newEntry` via `newEntry->copyValues(oldEntry)` (preserves name, icon, layer scene, text width, etc.). After `copyValues`, write the in-memory `_text` to the freshly allocated `.md` path via `newEntry->setLinkedFile(absMdPath)` followed by direct file write of the old text (do not call `setText` here — that would emit `dirty()` and re-trigger writes during migration; instead, write the file directly through `createTextNode` or equivalent, then assign `_text` so subsequent reads match disk).
    4. Determine the old entry's row in its parent (`int oldRow = parent->getChildRow(oldEntry);` or the equivalent existing API), then call `parent->removeChildObject(oldEntry)` followed by `parent->insertChildObject(newEntry, oldRow)` to preserve tree order.
    5. Call `oldEntry->deleteLater()` — never `delete` directly, because signal connections may still be in flight from the just-completed `inputXML`/`postProcessXML` cycle.
    6. Emit `campaign->changed()` (visual refresh, *not* `dirty()` until after the whole migration completes) at the end of the migration, once, so the tree model refreshes a single time.
  - For each external media path encountered during migration, route through `CampaignFilesManager::copyMediaInto`, prompting for video.
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
  - On import, after files are placed on disk and **before** they appear via the watcher, call `CampaignFilesManager::registerExpectedPath(absPath)` (chunk 2 API) for every file the importer placed. The watcher is *not* suspended during import (the campaign may be open and active and other unrelated paths must continue to be tracked); per-path registration is the correct mechanism here. The complementary blanket-suspension approach used in chunk 11 is not appropriate here.
  - After XML import completes, call `CampaignFilesManager::scanForNewEntries`. Files that were placed by the importer **and** also added via the imported XML will be skipped by the registered-expected-paths set; files placed by the importer that are **not** in the imported XML will be picked up by the scan as auto-discovered entries (the spec's intended behaviour).
  - When the import operation completes (success or failure), call `CampaignFilesManager::clearExpectedPaths()` to reset the set for the next operation.
- **acceptance_criteria**:
  - `CampaignExporter` has a setter for the source files directory and uses it in `populateExport` / `addObjectTree`.
  - Recursive copy uses `QFile::copy` (or a helper that wraps `QDirIterator` + `QFile::copy`) — diff contains literal `QFile::copy`.
  - `objectimportdialog.cpp` handles the directory-collision prompt with a `QMessageBox`.
  - The diff shows calls to `registerExpectedPath` for each placed file and a matching `clearExpectedPaths` at end-of-operation.
  - Build succeeds with no new warnings.
- **constraints_in_scope**:
  - Use `Qt`'s file APIs only — no `std::filesystem` (project consistency).
- **out_of_scope**:
  - "Export as archive (zip)" mentioned in the spec — that is a future enhancement; this chunk delivers the folder-structure form, which is the prerequisite. The acceptance criteria above explicitly do not require zip support.
  - Per-entry export of just a single `.md` (the spec talks about subtree export, which is what `addObjectTree` already does).

# Cycle Log

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

## Pre-Implementation Review — 2026-05-11 (Revision 2)

reviewer_model: opus
verdict: Pass
summary: All 10 `required_plan_changes` from the prior review are addressed substantively. The watcher self-write loop is broken via reference-counted `suspendWatch`/`resumeWatch` declared in Chunk 2 and used in Chunk 3. Chunk 9 has a clean four-step early-return guard ladder that correctly leans on `findOwningCampaign(this) == nullptr` to cover both `inputXML` and `copyValues`. The Chunk 11 QUuid-preserving swap is now a numbered six-step recipe and is explicitly bound to `Campaign` (the only `friend` of `DMHObjectBase` that can call protected `setID`). The migration / autodiscovery race is closed by both the blanket watcher suspension and the defence-in-depth `registerExpectedPath` calls. The relative-vs-absolute convention is pinned in Chunk 1 and consistently resolved in Chunks 6, 7, 11. Chunk 5's class mapping is concrete (unknown subdir → `EncounterText` container, `_contents.md` → child `EncounterTextLinked` named `_contents`) and Chunk 10 enforces the reserved name. `findOwningCampaign` is the shared helper, the constructor-no-I/O constraint is stated, and `addPath` failures will surface via `qWarning`. Architecture is sound; remaining concerns are localized and non-blocking.

triggers_evaluated:
  - threading: addressed
  - layer_interface: not-applicable
  - serialization_shape: addressed
  - subsystem_boundary: addressed
  - new_subsystem_or_flag: addressed

findings:
  - Low: Chunk 11 step 3 vs Chunk 3 — Step 3 instructs the migration to write `.md` files "directly through `createTextNode` or equivalent, then assign `_text` so subsequent reads match disk", explicitly avoiding `newEntry->setText(...)` to "not emit `dirty()` and re-trigger writes". But Chunk 3 has already wrapped `setText` in `suspendWatch`/`resumeWatch` precisely so this kind of write-through is safe and idempotent; using `setText` would also avoid the `_text` protected-member access concern (the migration code lives on `Campaign`, which is `friend` of `DMHObjectBase` but not of `EncounterText`, so it cannot directly assign `_text`). Recommend Design either (a) clarify that step 3 calls `setText` (which is now safe under suspension and which the manager has already suspended for this path via the bracketing `suspendWatch()` call) and emits a single batched `dirty()` at the end of migration, or (b) add an explicit `EncounterText`-level protected-friend or internal helper for the `_text` assignment so the recipe as written is actually compilable. Either is fine architecturally; the current wording is internally inconsistent with Chunk 3's design.
  - Info: Chunk 11 — In the revision the `acceptance_criteria` and `constraints_in_scope` blocks present in the prior version were dropped. This is a schema/process concern (the Review Agent verifies against `acceptance_criteria` per chunk); it is not an architectural defect, but Design should restore the section before Execution dispatches Chunk 11 so the Review Agent has a verification target.
  - Info: Front-matter — `supersedes` is set to the same path as the current plan, which makes the link self-referential. Per `PLAN_SCHEMA.md` this should point to the prior plan revision (typically an archived copy) or be `null`. Non-architectural; flag for Design to correct alongside the Chunk 11 acceptance_criteria restoration.
  - Info: Chunk 11 step 6 instructs the migration to emit `changed()` (visual refresh) but defer `dirty()` until "after the whole migration completes". The plan should briefly state where that single end-of-migration `dirty()` is emitted (in `migrateToFilesDirectory` itself, or by `MainWindow` after the call returns). The `copyValues` call inside step 3 will also reach `setName` → `handleInternalChange` → propagated `dirty()` for each migrated entry; the design intent is that these are tolerated (or suppressed for the duration of migration via a transient flag on `Campaign`). Worth one sentence of clarification, but not blocking — the migration runs after `inputXML` returns, so `dirty()` emissions are technically legal at that point.
  - Info: Chunk 5 — The chosen "fabricated `_contents` child entry" mapping is defensible and aligns with Chunk 10's reserved-name enforcement. One small consequence worth naming explicitly: when a user creates a *new* directory entry in DMHelper (chunk 10) and then adds a text body to it, the body needs to materialize as the child `_contents` entry to keep the on-disk shape symmetric with the autodiscovery shape. Chunk 10 does not currently describe how that happens. This is a design consistency point Design may want to fold into Chunk 10 as a follow-up integration task, not a blocker for the architecture.

required_plan_changes: []

risk_notes:
  - Low: Internal inconsistency between Chunk 3 (setText safe under suspension) and Chunk 11 step 3 (avoid setText, write `_text` directly). Pick one mechanism.
  - Low: Chunk 11 lost its `acceptance_criteria` / `constraints_in_scope` sections in the revision; Review Agent will need them.
  - Low: Migration's batched `dirty()` emission point is not specified.
  - Low: Symmetric on-disk shape when a user adds a body to a directory entry created via the new-entry dialog (chunk 10) is not described.
  - Info: `supersedes` self-reference in front-matter.

# Escalations
