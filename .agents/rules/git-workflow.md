# Git Workflow & CI/CD Guidelines

These rules govern branch management, commit standards, and pull request verification for the Buraq repository.

## 1. Branch Strategy (Gitflow)
- **`main`**:
  - The production release branch.
  - Represents stable, tested releases.
  - **Never commit directly to `main`.**
  - **Only PRs originating from `develop` are allowed into `main`** (strictly validated by `.github/workflows/validate-merge.yml`).
- **`develop`**:
  - The active development branch.
  - All feature branches, bugfixes, and chores merge here.
- **`feature/*`, `fix/*`, `chore/*`**:
  - Branch out from `develop`.
  - Open pull requests targeting `develop`.

## 2. Commit Message Standards
Follow Conventional Commits:
```
<type>(<optional-scope>): <description>

[optional body]

[optional footer(s)]
```
- **Allowed Types**:
  - `feat`: A new user-facing or architectural feature.
  - `fix`: A bug fix.
  - `chore`: Build system, CI workflow, or dependency maintenance.
  - `refactor`: Code restructuring without modifying behavior or fixing bugs.
  - `style`: Formatting, missing semicolons, white-space changes.
  - `docs`: Documentation updates only.
  - `test`: Adding or modifying automated test suites.
  - `ci`: Changes to GitHub Actions workflows or scripts.
- **Example**: `feat(terminal): add ANSI escape code parsing support`

## 3. Pull Request Requirements
- PR titles must clearly describe the change.
- Verify that code compiles locally (`./build.sh` or `cmake --build build`) before opening a PR.
- Never force-push to `main` or `develop`.
- Ensure all CI checks pass in `.github/workflows/test-build.yml` before requesting review.
