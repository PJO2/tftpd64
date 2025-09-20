<#  vt_upload.ps1 — VirusTotal v3 uploader (PS 5.1+ compatible)
    Usage:
      $env:VT_API_KEY = 'YOUR_API_KEY'
      powershell -ExecutionPolicy Bypass -File .\vt_upload.ps1 -Path "..\ARTS\BIN\dist\4.72\signed\" -ApiKey f1..
      # Or pass a directory; the script will expand to *.exe inside it.
#>

param(
  [Parameter(Mandatory=$true)]
  [string[]]$Path,
  [string]$ApiKey = $env:VT_API_KEY,
  [switch]$Private,
  [switch]$NoWait,
  [int]$PollSeconds = 4,
  [int]$MaxPolls = 150
)

if (-not $ApiKey) { throw "Set -ApiKey or `$env:VT_API_KEY`." }

$VTBase  = "https://www.virustotal.com/api/v3"
$Headers = @{ "x-apikey" = $ApiKey }

function Resolve-Targets([string[]]$Paths) {
  $items = @()
  foreach ($p in $Paths) {
    if (Test-Path -LiteralPath $p -PathType Container) {
      $items += Get-ChildItem -LiteralPath $p -Filter *.exe -File
    } else {
      $items += Get-ChildItem -LiteralPath $p -File
    }
  }
  $items | Sort-Object FullName -Unique
}

function Get-UploadUrl([int64]$Size, [switch]$UsePrivate) {
  if ($UsePrivate) {
    if ($Size -le 32MB) { return "$VTBase/private/files" }
    return (Invoke-RestMethod -Headers $Headers -Uri "$VTBase/private/files/upload_url" -Method GET).data
  } else {
    if ($Size -le 32MB) { return "$VTBase/files" }
    return (Invoke-RestMethod -Headers $Headers -Uri "$VTBase/files/upload_url" -Method GET).data
  }
}

# PS 5.1-safe multipart/form-data upload using HttpClient
Add-Type -AssemblyName System.Net.Http
Add-Type -AssemblyName System.Net.Http.WebRequest
function Post-FileMultipart([string]$Uri, [System.IO.FileInfo]$File) {
  $handler = New-Object System.Net.Http.HttpClientHandler
  $client  = New-Object System.Net.Http.HttpClient($handler)
  $client.DefaultRequestHeaders.Add("x-apikey", $ApiKey)
  $content = New-Object System.Net.Http.MultipartFormDataContent
  $fs = [System.IO.File]::OpenRead($File.FullName)
  try {
    $sc = New-Object System.Net.Http.StreamContent($fs)
    $sc.Headers.ContentType = [System.Net.Http.Headers.MediaTypeHeaderValue]::Parse("application/octet-stream")
    $content.Add($sc, "file", $File.Name)
    $resp = $client.PostAsync($Uri, $content).Result
    $resp.EnsureSuccessStatusCode() | Out-Null
    $json = $resp.Content.ReadAsStringAsync().Result
    return ($json | ConvertFrom-Json)
  } finally {
    $content.Dispose(); $client.Dispose(); $fs.Dispose()
  }
}

function Wait-Analysis([string]$Id) {
  for ($i=0; $i -lt $MaxPolls; $i++) {
    $a = Invoke-RestMethod -Headers $Headers -Uri "$VTBase/analyses/$Id" -Method GET
    if ($a.data.attributes.status -eq 'completed') { return $a }
    Start-Sleep -Seconds $PollSeconds
  }
  throw "Timeout waiting for $Id"
}

function Get-Report([string]$Sha256) {
  Invoke-RestMethod -Headers $Headers -Uri "$VTBase/files/$Sha256" -Method GET
}

$files = Resolve-Targets $Path
if (-not $files) { Write-Error "No files matched."; exit 1 }

$rows = @()
foreach ($fi in $files) {
  $sha = (Get-FileHash -LiteralPath $fi.FullName -Algorithm SHA256).Hash.ToLower()
  Write-Host ">> Uploading $($fi.Name) (SHA256=$sha)..."
  try {
    $upl = Get-UploadUrl -Size $fi.Length -UsePrivate:$Private
    $submit = Post-FileMultipart -Uri $upl -File $fi
    $analysisId = $submit.data.id

    if ($NoWait) {
      $rows += [pscustomobject]@{File=$fi.Name; Sha256=$sha; AnalysisId=$analysisId; Link="https://www.virustotal.com/gui/file/$sha"}
      continue
    }

    $done   = Wait-Analysis $analysisId
    $sha_dl = $done.meta.file_info.sha256
    if (-not $sha_dl) { $sha_dl = $sha }
    $rep    = Get-Report $sha_dl
    $s      = $rep.data.attributes.last_analysis_stats
    $link   = "https://www.virustotal.com/gui/file/$sha_dl"
    "{0}  =>  M:{1}  S:{2}  H:{3}  U:{4}  {5}" -f $fi.Name, $s.malicious, $s.suspicious, $s.harmless, $s.undetected, $link | Write-Host

    $rows += [pscustomobject]@{
      File=$fi.Name; Sha256=$sha_dl; AnalysisId=$analysisId
      Malicious=$s.malicious; Suspicious=$s.suspicious; Harmless=$s.harmless; Undetected=$s.undetected
      Link=$link
    }
  } catch {
    Write-Warning "Failed on $($fi.Name): $($_.Exception.Message)"
    $rows += [pscustomobject]@{File=$fi.Name; Sha256=$sha; AnalysisId=$null; Malicious=$null; Suspicious=$null; Harmless=$null; Undetected=$null; Link=$null; Error=$_.Exception.Message}
  }
}

if ($rows.Count) {
  $csv = "vt_results_{0:yyyyMMdd_HHmmss}.csv" -f (Get-Date)
  $rows | Export-Csv -Path $csv -NoTypeInformation -Encoding UTF8
  Write-Host "Saved CSV: $csv"
}
