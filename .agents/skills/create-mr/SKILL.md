---
name: create-mr
description: >-
  Automates the creation of Merge Requests (Pull Requests) on GitHub for the current
  branch using Git Credential Manager and the GitHub REST API. Use whenever asked to create
  an MR, create a PR, submit changes for review, or merge a branch into develop or main.
---

# Create Merge Request (Pull Request)

This skill provides an automated, zero-friction procedure to open Pull Requests on GitHub without requiring the GitHub CLI (`gh`) or manual browser intervention.

## Trigger Scenarios
Activate this skill when:
- The user requests to "create an MR", "open a PR", "submit a pull request", or "merge branch X into develop/main".
- Completing a feature, fix, or refactor workflow that requires opening a PR against `develop` per the repository Gitflow rules.

---

## Branch Routing Rules

| Working Branch Type | Target Base Branch | Notes |
| :--- | :--- | :--- |
| `feature/*` | `develop` | All new capabilities and extensions |
| `fix/*`, `bugfix/*` | `develop` | Bugfixes and patches |
| `chore/*` | `develop` | Build, CI, or dependency updates |
| `develop` / `release/*` | `main` | Production release candidate only |

> [!IMPORTANT]
> Never open a PR directly from a `feature/*` or `fix/*` branch targeting `main`. Branch protection rules will reject it. All features must merge to `develop` first.

---

## Workflow Steps

### Step 1: Ensure Working Branch is Pushed
```powershell
git push -u origin <current-branch>
```

### Step 2: Execute the Automated PR Creator Script
Run the bundled script `scripts/create-mr.ps1`:
```powershell
& .agents/skills/create-mr/scripts/create-mr.ps1 `
    -Title "<Conventional Commit Title>" `
    -Base "develop" `
    -Body @"
## Overview
Brief description of the changes.

## Key Changes
- Itemized change 1
- Itemized change 2

## Verification
- Automated build / test results
"@
```

### Step 3: Report Results
The script prints the created GitHub PR URL (`https://github.com/Business-Aura/buraq/pull/<number>`).
Always format the URL as a clickable markdown link in the response to the user.
