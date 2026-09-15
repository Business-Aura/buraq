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
- PR titles must clearly describe the change following Conventional Commits.
- Verify that code compiles locally before opening a PR.
- Never force-push to `main` or `develop`.
- Ensure all CI checks pass in `.github/workflows/test-build.yml` before requesting review.

## 4. Creating Merge Requests / Pull Requests
When asked to create an MR/PR or when completing a branch workflow:
1. **Push Branch**: Ensure the current branch is pushed to origin:
   ```powershell
   git push -u origin <branch-name>
   ```
2. **Determine Target**:
   - Features, fixes, and chores (`feature/*`, `fix/*`, `chore/*`) MUST target `develop`.
   - Production releases (`release/*` or `develop`) target `main`.
3. **Automate PR Creation via API**:
   Do not ask the user to manually create the PR in the browser. Query Git Credential Manager for the GitHub token and call the GitHub REST API:
   ```powershell
   $creds = "protocol=https`nhost=github.com`n`n" | git credential fill
   $token = ($creds | Select-String "password=(.*)").Matches.Groups[1].Value
   $body = @{
       title = "<Conventional Commit Title>"
       head  = "<branch-name>"
       base  = "develop"
       body  = "<Markdown PR Summary>"
   } | ConvertTo-Json
   $headers = @{ "Authorization" = "Bearer $token"; "Accept" = "application/vnd.github+json"; "User-Agent" = "Buraq-PR-Agent" }
   $response = Invoke-RestMethod -Uri "https://api.github.com/repos/Business-Aura/buraq/pulls" -Method Post -Headers $headers -Body $body -ContentType "application/json"
   ```
   Or invoke `.agents/skills/create-mr/scripts/create-mr.ps1`.
4. **Report Link**: Output the resulting PR URL (`$response.html_url`) prominently to the user.

