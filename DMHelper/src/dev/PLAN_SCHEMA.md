# Plan Document Schema — DMHelper Multi-Agent Pipeline

This file is the **canonical specification** for the Plan Document. Every Plan
Document under `DMHelper/dev/plans/<feature-slug>.md` must conform to this
schema.

The Plan Document is the only shared persistence layer in the pipeline. It is
written by the Design agent, consumed by the Coordinator, Execution, and
Review agents, and appended to by the Coordinator, Review agent, and (when
triggered) the Architecture Reviewer.

Format: Markdown with a YAML front-matter header and named sections in a
fixed order. All field names are case-sensitive.

## File Locations

- Plan path: `DMHelper/dev/plans/<feature-slug>.md` (committed, public).
- Spec path: `DMHelper/dev/specs/<feature-slug>.md` (committed, public).
- The slug must match between spec and plan filenames.
- If a plan with the same slug already exists, the new plan **supersedes**
  it. The old plan is not deleted; the new plan sets the `supersedes` field
  to the old plan's path. See the Design agent for replanning rules.

## Front-Matter (YAML)

The plan opens with a YAML front-matter block delimited by `---` lines.

```yaml
---
feature_slug: <kebab-case-identifier>          # required, matches filename
spec_path: DMHelper/dev/specs/<feature-slug>.md # required
created: <ISO-8601 date>                        # required
designer_model: opus                            # required, always "opus"
arch_review_required: <true|false>              # required
arch_review_model: <opus|sonnet>                # required if arch_review_required is true; "sonnet" otherwise
arch_review_reason: <string>                    # required if arch_review_required is true; "n/a" otherwise
pre_impl_arch_review_requested: <true|false>    # required
supersedes: <path-or-null>                      # required; null if first plan for this slug
status: <draft|approved|in-progress|complete|escalated>  # required
---
```

### `arch_review_required` — Trigger Rules

Set to `true` if **any** of the following is true for the feature as a
whole:

1. Touches threading boundaries (VLC callbacks, worker threads) or OpenGL
   context boundaries.
2. Adds or modifies a `Layer` subclass (anything implementing
   `dm*` / `playerGL*` interfaces).
3. Changes the shape of serialization (`createOutputXML` /
   `internalOutputXML` / `inputXML` / `postProcessXML`) for an existing
   class, or introduces a new serialized class.
4. Touches more than one of: battle, audio, campaign, UI shell.
5. Introduces a new top-level subsystem or a new feature flag in
   `dmconstants.h`.

If `arch_review_required` is `true` and any of triggers **1, 2, or 5**
fired, `arch_review_model` **must** be `opus`. Otherwise `sonnet` is
sufficient.

### `pre_impl_arch_review_requested`

Set to `true` only when Design's self-assessment hits triggers 1, 2, or 5
**and** Design judges the chosen approach is non-obvious enough to warrant
sanity-checking the plan itself before any worktree is created. The same
`arch_review_model` value is reused for both pre- and post-impl reviews.

## Section Order

After the front-matter, sections appear in this exact order. Sections
marked **append-only** are written by agents other than Design and must
not be edited in place.

1. `# Summary`
2. `# Replanning Rationale` — present only when `supersedes` is non-null.
3. `# Architectural Risk Assessment`
4. `# Chunks`
5. `# Cycle Log` *(append-only; written by Coordinator and Review)*
6. `# Architecture Review` *(append-only; written by Architecture Reviewer)*
7. `# Escalations` *(append-only; written by Coordinator)*

## Section: Summary

One to three paragraphs describing what the feature is and the design
intent. Must explicitly name the subsystems touched.

## Section: Replanning Rationale

Required only when superseding an earlier plan. Explains:

- What the previous plan got wrong or what changed in requirements.
- Which chunks from the old plan are reused, modified, or dropped.
- Which `cycle_log` entries from the old plan should be treated as known
  failure modes to avoid repeating.

## Section: Architectural Risk Assessment

Bulleted list with one entry per arch trigger (1–5) evaluated. Each entry:

- `Trigger <n>: <Hit | Not hit>` — one-sentence reason.

Closes with a single line stating the values copied to front-matter:
`arch_review_required = ...`, `arch_review_model = ...`,
`pre_impl_arch_review_requested = ...`.

## Section: Chunks

A numbered list of chunks. Each chunk is a level-2 heading with the
following fields, in order. Field names are bold; values follow on the
same or next line. Lists use `-` bullets.

```
## Chunk <id>: <short title>

- **id**: <stable-kebab-case-identifier>          # used for worktree, branch, log keys
- **summary**: <one-sentence purpose>
- **dependencies**: [<chunk-id>, ...]             # other chunks that must pass before this starts; [] if none
- **parallelizable**: <true|false>                # true only if `dependencies` is [] or all listed deps are already complete
- **worktree_path**: ../DMHelper-wt-<id>/         # set by Design; Coordinator creates the directory
- **branch**: agent/work/<id>                     # set by Design
- **files_to_modify**:
  - <repo-relative path> — <reason>
  - ...
- **files_to_create**:
  - <repo-relative path> — <purpose>
  - ...                                            # [] if none
- **integration_tasks**:
  - <imperative sentence describing one wiring step>
  - ...                                            # [] if none
- **acceptance_criteria**:
  - <mechanically verifiable statement>
  - ...
- **constraints_in_scope**:                        # constraints from .agent/execution/CLAUDE.md that this chunk specifically must respect
  - <e.g. "GL context rule applies to new shader init">
  - ...                                            # [] if none beyond the global rule set
- **out_of_scope**:
  - <explicit non-goal>
  - ...                                            # [] if none
```

### Rules for Chunk Authoring

- Every file the Execution agent will touch must be listed in
  `files_to_modify` or `files_to_create`. Anything not listed is
  out-of-scope for that chunk.
- New `.cpp`/`.h` pairs must include the corresponding `CMakeLists.txt`
  in `files_to_modify` for source registration.
- Cross-chunk wiring is **always** an `integration_tasks` entry on the
  later chunk, never an implicit assumption.
- `acceptance_criteria` must be testable by reading diffs and the build
  log — no "looks reasonable" criteria.
- A chunk that requires a `.ui` change must call out the change as a
  human-mediated Qt Designer step in `integration_tasks` and **not** in
  `files_to_modify`. The Execution agent will not edit `.ui` XML.
- A chunk size target: 1–6 source files, ≤ ~400 lines of expected diff.
  Larger work must be split.

## Section: Cycle Log (append-only)

Per-chunk subsections, added as cycles run. Written only by Coordinator
(creating entries, recording dispatch) and Review (recording verdicts).
Execution never edits this section.

```
## <chunk-id>

### Cycle 1 — <ISO-8601 timestamp>
- **dispatched_by**: coordinator
- **executor_commit_range**: <sha-from>..<sha-to>
- **executor_build_status**: <success|failure|skipped>
- **executor_handoff_summary**: <one paragraph from the Execution agent>
- **review_verdict**: <Pass|Gap|DesignProblem>
- **review_findings**:
  - <severity>: <finding>
  - ...
- **next_action**: <merge|re-execute|escalate-to-human>
```

The Coordinator increments the cycle counter for a chunk after each
Review verdict. **Hard cap: 3 cycles per chunk.** Reaching cycle 4
without a `Pass` triggers an `Escalations` entry and halts that chunk.

A `DesignProblem` verdict at any cycle immediately routes to
`Escalations` regardless of cycle count.

## Section: Architecture Review (append-only)

Written only by the Architecture Reviewer. There may be up to two
entries: one pre-impl (if requested) and one post-impl.

```
## Pre-Implementation Review — <ISO-8601 timestamp>
- **reviewer_model**: <opus|sonnet>
- **verdict**: <Pass|Revise|Block>
- **findings**:
  - <severity>: <finding>
- **required_plan_changes**:
  - <chunk-id or "global">: <what must change>

## Post-Implementation Review — <ISO-8601 timestamp>
- **reviewer_model**: <opus|sonnet>
- **verdict**: <Pass|Revise|Block>
- **findings**: ...
- **required_followups**:
  - <chunk-id or "new chunk">: <what must change>
```

`Pass` clears the gate. `Revise` routes back to Design (pre-impl) or to a
new chunk (post-impl). `Block` is escalated to the human.

## Section: Escalations (append-only)

Written only by the Coordinator.

```
## <ISO-8601 timestamp> — <chunk-id or "pipeline">
- **reason**: <cycle-cap-reached|design-problem|arch-block|tool-failure|ambiguity>
- **detail**: <one paragraph>
- **state_at_escalation**:
  - worktrees_left_in_place: [<path>, ...]
  - branches_left_in_place: [<branch>, ...]
  - last_cycle: <chunk-id>:<n>
- **handoff_to**: human
```

Escalations halt the pipeline for that chunk. The Coordinator does not
clean up worktrees or branches on escalation — they are left for human
inspection.

## Write-Scope Discipline

| Section                  | Writers                                  |
| ------------------------ | ---------------------------------------- |
| Front-matter             | Design (create), Coordinator (`status` only) |
| Summary                  | Design                                   |
| Replanning Rationale     | Design                                   |
| Architectural Risk       | Design                                   |
| Chunks                   | Design                                   |
| Cycle Log                | Coordinator + Review (append only)       |
| Architecture Review      | Architecture Reviewer (append only)      |
| Escalations              | Coordinator (append only)                |

The Execution agent **never** writes to the Plan Document. It returns a
handoff note in its final message; the Coordinator transcribes that into
the `Cycle Log`.

## Status Values

- `draft` — written by Design, not yet approved at human checkpoint 1.
- `approved` — human checkpoint 1 passed; Coordinator may dispatch.
- `in-progress` — at least one chunk is in flight.
- `complete` — all chunks merged, all required arch reviews passed,
  human checkpoint 2 passed.
- `escalated` — pipeline halted; awaiting human action.

Only the Coordinator transitions status, except `draft → approved` which
is set by the Coordinator on receipt of human checkpoint 1 approval.

## Persistence Rule

The Plan Document is the only persistence layer for pipeline state. The
Coordinator must flush every `Cycle Log`, `Architecture Review`, or
`Escalations` write to disk **immediately** after a sub-agent returns,
before dispatching the next sub-agent. Restart of the Coordinator session
must be safe: re-reading the plan must reconstitute full pipeline state,
including per-chunk cycle counts (computed by counting `### Cycle N`
headings under each chunk in `Cycle Log`).
