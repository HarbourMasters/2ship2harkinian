Using Namespace System
$url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.6/LLVM-14.0.6-win64.exe"
$llvmInstallerPath = ".\LLVM-14.0.6-win64.exe"
$clangFormatFilePath = ".\clang-format.exe"
$requiredVersion = "clang-format version 14.0.6"
$currentVersion = ""

function Test-7ZipInstalled {
    $sevenZipPath = "C:\Program Files\7-Zip\7z.exe"
    return Test-Path $sevenZipPath -PathType Leaf
}

if (Test-Path $clangFormatFilePath) {
    $currentVersion = & $clangFormatFilePath --version
    if (-not ($currentVersion -eq $requiredVersion)) {
        # Delete the existing file if the version is incorrect
        Remove-Item $clangFormatFilePath -Force
    }
}

if (-not (Test-Path $clangFormatFilePath) -or ($currentVersion -ne $requiredVersion)) {
    if (-not (Test-7ZipInstalled)) {
        Write-Host "7-Zip is not installed. Please install 7-Zip and run the script again."
        exit
    }

    $wc = New-Object net.webclient
    $wc.Downloadfile($url, $llvmInstallerPath)

    $sevenZipPath = "C:\Program Files\7-Zip\7z.exe"
    $specificFileInArchive = "bin\clang-format.exe"
    & "$sevenZipPath" e $llvmInstallerPath $specificFileInArchive

    Remove-Item $llvmInstallerPath -Force
}

$basePath = (Resolve-Path .).Path
$files = Get-ChildItem -Path $basePath\mm -Recurse -File `
    | Where-Object { ($_.Extension -eq '.c' -or $_.Extension -eq '.cpp' -or `
                      ($_.Extension -eq '.h' -and `
                       (-not ($_.FullName -like "*\mm\src\*" -or $_.FullName -like "*\mm\include\*")))) -and `
                     (-not ($_.FullName -like "*\mm\assets\*")) }

foreach ($file in $files) {
    $relativePath = $file.FullName.Substring($basePath.Length + 1)
    Write-Host "Formatting $relativePath"
    .\clang-format.exe -i $file.FullName
}
