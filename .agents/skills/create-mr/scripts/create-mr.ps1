<#
.SYNOPSIS
    Creates a Pull Request (Merge Request) on GitHub for the current or specified branch.

.PARAMETER Title
    The PR title (should follow Conventional Commits).

.PARAMETER Base
    The target branch to merge into (default: 'develop').

.PARAMETER Head
    The source branch (default: current git branch).

.PARAMETER Body
    Markdown body describing the PR changes.

.PARAMETER Draft
    Optional switch to create as draft.
#>
param(
    [Parameter(Mandatory = $true)]
    [string]$Title,

    [string]$Base = "develop",

    [string]$Head = "",

    [string]$Body = "",

    [switch]$Draft = $false
)

$ErrorActionPreference = "Stop"

# Detect repository root
$repoRoot = (git rev-parse --show-toplevel 2>$null)
if (-not $repoRoot) {
    Write-Error "Not inside a git repository."
    exit 1
}

# Determine origin remote URL
$remoteUrl = (git remote get-url origin 2>$null)
if (-not $remoteUrl -or $remoteUrl -notmatch "github\.com[/:](?<owner>[^/]+)/(?<repo>[^/\.]+)") {
    Write-Error "Could not detect GitHub owner/repo from remote origin '$remoteUrl'."
    exit 1
}
$owner = $Matches["owner"]
$repo = $Matches["repo"]

# Determine head branch if not specified
if (-not $Head) {
    $Head = (git rev-parse --abbrev-ref HEAD 2>$null)
}
if (-not $Head -or $Head -eq "HEAD") {
    Write-Error "Could not determine current branch name."
    exit 1
}

# Ensure branch is pushed
Write-Host "Ensuring branch '$Head' is pushed to origin..."
git push -u origin $Head

# Retrieve token from Git Credential Manager
Write-Host "Retrieving GitHub credentials from Git Credential Manager..."
$credInput = "protocol=https`nhost=github.com`n`n"
$credOutput = $credInput | git credential fill 2>$null
$tokenMatch = ($credOutput | Select-String "password=(.+)").Matches
if (-not $tokenMatch -or -not $tokenMatch.Groups[1].Value) {
    Write-Error "Failed to retrieve GitHub credentials from Git Credential Manager."
    exit 1
}
$token = $tokenMatch.Groups[1].Value.Trim()

# Construct PR payload
$payload = @{
    title = $Title
    head  = $Head
    base  = $Base
    body  = $Body
    draft = $Draft.IsPresent
} | ConvertTo-Json

$headers = @{
    "Authorization" = "Bearer $token"
    "Accept"        = "application/vnd.github+json"
    "User-Agent"    = "Buraq-PR-Agent"
}

$uri = "https://api.github.com/repos/$owner/$repo/pulls"

Write-Host "Opening PR ($Head -> $Base) on $owner/$repo..."
try {
    $response = Invoke-RestMethod -Uri $uri -Method Post -Headers $headers -Body $payload -ContentType "application/json"
    Write-Host "SUCCESS: Pull Request created!" -ForegroundColor Green
    Write-Host "PR URL: $($response.html_url)" -ForegroundColor Cyan
    Write-Output $response.html_url
}
catch {
    Write-Error "GitHub API Error: $_"
    exit 1
}
