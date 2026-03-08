# create_novel_files.ps1
Write-Host "正在创建小说章节文件和JSON配置..." -ForegroundColor Green

# 设置基础路径
$BaseDir = Join-Path $PWD.Path "TheRisingDawn"
$CoverDir = Join-Path $BaseDir "Covers"
$TempDir = Join-Path $BaseDir "data\temp"  # 添加临时目录
$JsonFile = Join-Path $BaseDir "novel_config.json"

# 创建必要的目录
New-Item -ItemType Directory -Force -Path $BaseDir | Out-Null
New-Item -ItemType Directory -Force -Path $CoverDir | Out-Null
New-Item -ItemType Directory -Force -Path $TempDir | Out-Null  # 创建临时目录

# 创建封面占位文件
$CoverFile = Join-Path $CoverDir "the_rising_dawn.jpg"
"Placeholder for cover image" | Out-File -FilePath $CoverFile -Encoding UTF8

# 定义小说数据
$Novel = @{
    bookId = "urn:uuid:123e4567-e89b-12d3-a456-426614174000"  # 添加bookId
    tempPath = "./data/temp"  # 添加tempPath
    coverUrl = $CoverFile.Replace('\', '/')
    novelAuthor = "John Smith"
    novelTitle = "The Rising Dawn"
    isEpub = $false
    volumes = @(
        @{
            volumeName = "Volume 1: The Beginning"
            chapters = @(
                @{ title = "Chapter 1: A New Journey"; content = "This is the beginning of an epic journey..." },
                @{ title = "Chapter 2: Unexpected Encounters"; content = "Fate brings together unlikely allies..." },
                @{ title = "Chapter 3: Secrets Unveiled"; content = "Dark secrets from the past come to light..." }
            )
        },
        @{
            volumeName = "Volume 2: Dark Times"
            chapters = @(
                @{ title = "Chapter 4: The Shadow Falls"; content = "A great darkness descends upon the land..." },
                @{ title = "Chapter 5: Allies and Enemies"; content = "Friendships are tested and new alliances form..." },
                @{ title = "Chapter 6: The Turning Point"; content = "A crucial battle changes everything..." }
            )
        },
        @{
            volumeName = "Volume 3: Dawn Rises"
            chapters = @(
                @{ title = "Chapter 7: Preparation for Battle"; content = "The final preparations begin..." },
                @{ title = "Chapter 8: The Final Confrontation"; content = "Good and evil clash in an epic battle..." },
                @{ title = "Chapter 9: A New Beginning"; content = "A new era dawns for the kingdom..." }
            )
        }
    )
}

# 辅助函数：清理文件名中的非法字符
function Get-CleanFileName {
    param([string]$Name)
    # 替换Windows文件名中不允许的字符
    $Name = $Name -replace '[<>:"/\\|?*]', ''
    # 替换冒号和空格
    $Name = $Name -replace ':', ''
    $Name = $Name -replace ' ', '_'
    return $Name
}

# 创建章节文件并构建JSON对象
$JsonVolumes = @()

foreach ($volume in $Novel.volumes) {
    # 创建卷目录
    $VolumeDirName = Get-CleanFileName $volume.volumeName
    $VolumePath = Join-Path $BaseDir $VolumeDirName
    New-Item -ItemType Directory -Force -Path $VolumePath | Out-Null
    
    $JsonChapters = @()
    
    foreach ($chapter in $volume.chapters) {
        # 创建章节文件
        $ChapterFileName = (Get-CleanFileName $chapter.title) + ".txt"
        $ChapterPath = Join-Path $VolumePath $ChapterFileName
        
        # 写入章节内容
        @"
Title: $($chapter.title)

$($chapter.content)

---
This is a sample chapter file created by PowerShell script.
You can replace this with actual chapter content.
"@ | Out-File -FilePath $ChapterPath -Encoding UTF8
        
        # 添加到JSON章节列表（使用/作为路径分隔符）
        $JsonChapters += @{
            title = $chapter.title
            url = $ChapterPath.Replace('\', '/')
        }
    }
    
    # 添加到JSON卷列表
    $JsonVolumes += @{
        volumeName = $volume.volumeName
        chapters = $JsonChapters
    }
}

# 创建最终的JSON对象（包含bookId和tempPath）
$FinalJson = @{
    bookId = $Novel.bookId
    tempPath = $Novel.tempPath
    coverUrl = $CoverFile.Replace('\', '/')
    novelAuthor = $Novel.novelAuthor
    novelTitle = $Novel.novelTitle
    isEpub = $Novel.isEpub
    volumes = $JsonVolumes
}

# 保存JSON文件（格式化输出）
$FinalJson | ConvertTo-Json -Depth 10 | Out-File -FilePath $JsonFile -Encoding UTF8

# 显示结果
Write-Host "`n创建完成！" -ForegroundColor Green
Write-Host "目录结构：" -ForegroundColor Yellow
Write-Host "$BaseDir\" -ForegroundColor Cyan
Write-Host "  ├─ data\" -ForegroundColor Cyan
Write-Host "  │   └─ temp\" -ForegroundColor Cyan
Write-Host "  ├─ Covers\" -ForegroundColor Cyan
Write-Host "  │   └─ the_rising_dawn.jpg" -ForegroundColor Cyan

foreach ($volume in $Novel.volumes) {
    $VolumeDirName = Get-CleanFileName $volume.volumeName
    Write-Host "  ├─ $VolumeDirName\" -ForegroundColor Cyan
    $chapterCount = 0
    foreach ($chapter in $volume.chapters) {
        $ChapterFileName = (Get-CleanFileName $chapter.title) + ".txt"
        if ($chapterCount -eq ($volume.chapters.Count - 1)) {
            Write-Host "  │   └─ $ChapterFileName" -ForegroundColor Cyan
        } else {
            Write-Host "  │   ├─ $ChapterFileName" -ForegroundColor Cyan
        }
        $chapterCount++
    }
}
Write-Host "  └─ novel_config.json" -ForegroundColor Cyan

Write-Host "`nJSON文件已生成: $JsonFile" -ForegroundColor Green

# 显示JSON内容预览
Write-Host "`nJSON预览:" -ForegroundColor Yellow
$JsonContent = Get-Content $JsonFile -Raw
Write-Host $JsonContent -ForegroundColor White

Write-Host "`n"
Read-Host "按回车键退出"