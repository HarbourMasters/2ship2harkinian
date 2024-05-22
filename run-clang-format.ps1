$url = "https://github.com/llvm/llvm-project/releases/download/llvmorg-14.0.6/LLVM-14.0.6-win64.exe"
$outputPath = ".\LLVM-14.0.6-win64.exe"
$extractionPath = ".\"
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
    $wc.Downloadfile($url, $outputPath)

    $sevenZipPath = "C:\Program Files\7-Zip\7z.exe"
    & "$sevenZipPath" x $outputPath -o$extractionPath

    $specificFileInArchive = "bin\clang-format.exe"
    & "$sevenZipPath" e $outputPath -o$extractionPath $specificFileInArchive

    Remove-Item $outputPath -Force
}

$baseDir = "mm"

$files = Get-ChildItem -Path $baseDir -Recurse -Include *.c, *.cpp, *.h |
    Where-Object {
        $_.FullName -notlike "*mm/assets/*" -and
        ($_.Extension -ne ".h" -or 
         ($_.FullName -notlike "*mm/src/*" -and $_.FullName -notlike "*mm/include/*"))
    }

$files | ForEach-Object {
    $escapedFileName = $_.FullName -replace ' ', '\\ '
    & .\clang-format.exe -i $escapedFileName
}
