# Domain Docs

How the engineering skills should consume this repo's domain documentation when exploring the codebase.

## Before exploring, read these

- `CONTEXT.md` at the repo root, if it exists.
- `docs/adr/` for architectural decisions that touch the area about to be changed.

If any of these files do not exist, proceed silently. The producer skill `grill-with-docs` creates them lazily when terms or decisions actually get resolved.

## File structure

This is a single-context repo:

```text
/
├─ CONTEXT.md
├─ docs/
│  └─ adr/
└─ src/
```

## Use the glossary vocabulary

When output names a domain concept in an issue title, refactor proposal, hypothesis, or test name, use the term as defined in `CONTEXT.md`.

If the concept is missing from the glossary, either avoid inventing new language or note it for `grill-with-docs`.

## Flag ADR conflicts

If output contradicts an existing ADR, surface it explicitly rather than silently overriding it.
