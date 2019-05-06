
#include <texteditor/fontsettings.h>
#include <utils/differ.h>

#include <QFutureInterfaceBase>
#include <QRegularExpression>

using namespace Utils;
    const QStringList newLines = text.split('\n');
                                           const QList<Diff> &rightDiffList)
            if (j == rightDiffList.count() && lastLineEqual && leftDiff.text.startsWith('\n'))
            if (i == leftDiffList.count() && lastLineEqual && rightDiff.text.startsWith('\n'))
            const QStringList newLeftLines = leftDiff.text.split('\n');
            const QStringList newRightLines = rightDiff.text.split('\n');
        line = startLineCharacter + textLine + '\n';
            line += "\\ No newline at end of file\n";
                    const QString line = makePatchLine('-',
                    const QString line = makePatchLine('+',
                const QString line = makePatchLine(' ',
    const QString chunkLine = "@@ -"
            + ','
            + " +"
            + ','
            + " @@"
            + '\n';
    const QString rightFileInfo = "+++ " + rightFileName + '\n';
    const QString leftFileInfo = "--- " + leftFileName + '\n';
static QString leftFileName(const FileData &fileData, unsigned formatFlags)
{
    QString diffText;
    QTextStream str(&diffText);
    if (fileData.fileOperation == FileData::NewFile) {
        str << "/dev/null";
    } else {
        if (formatFlags & DiffUtils::AddLevel)
            str << "a/";
        str << fileData.leftFileInfo.fileName;
    }
    return diffText;
}

static QString rightFileName(const FileData &fileData, unsigned formatFlags)
{
    QString diffText;
    QTextStream str(&diffText);
    if (fileData.fileOperation == FileData::DeleteFile) {
        str << "/dev/null";
    } else {
        if (formatFlags & DiffUtils::AddLevel)
            str << "b/";
        str << fileData.rightFileInfo.fileName;
    }
    return diffText;
}

        if (fileData.fileOperation == FileData::NewFile
                || fileData.fileOperation == FileData::DeleteFile) { // git only?
            if (fileData.fileOperation == FileData::NewFile)
                str << "new";
            else
                str << "deleted";
            str << " file mode 100644\n";
        }
        str << "index " << fileData.leftFileInfo.typeInfo << ".." << fileData.rightFileInfo.typeInfo;
        if (fileData.fileOperation == FileData::ChangeFile)
            str << " 100644";
        str << "\n";

            str << leftFileName(fileData, formatFlags);
            str << " and ";
            str << rightFileName(fileData, formatFlags);
            str << " differ\n";
            if (!fileData.chunks.isEmpty()) {
                str << "--- ";
                str << leftFileName(fileData, formatFlags) << "\n";
                str << "+++ ";
                str << rightFileName(fileData, formatFlags) << "\n";
                for (int j = 0; j < fileData.chunks.count(); j++) {
                    str << makePatch(fileData.chunks.at(j),
                                     (j == fileData.chunks.count() - 1)
                                     && fileData.lastChunkAtTheEndOfFile);
                }
static QList<RowData> readLines(QStringRef patch,
    const QChar newLine = '\n';
    const QVector<QStringRef> lines = patch.split(newLine);
        QStringRef line = lines.at(i);
        const QChar firstCharacter = line.at(0);
        if (firstCharacter == '\\') { // no new line marker
            if (firstCharacter == ' ') { // common line
            } else if (firstCharacter == '-') { // deleted line
            } else if (firstCharacter == '+') { // inserted line
            Diff diffToBeAdded(command, line.mid(1).toString() + newLine);
static QStringRef readLine(QStringRef text, QStringRef *remainingText, bool *hasNewLine)
{
    const QChar newLine('\n');
    const int indexOfFirstNewLine = text.indexOf(newLine);
    if (indexOfFirstNewLine < 0) {
        if (remainingText)
            *remainingText = QStringRef();
        if (hasNewLine)
            *hasNewLine = false;
        return text;
    }

    if (hasNewLine)
        *hasNewLine = true;

    if (remainingText)
        *remainingText = text.mid(indexOfFirstNewLine + 1);

    return text.left(indexOfFirstNewLine);
}

static bool detectChunkData(QStringRef chunkDiff,
                            ChunkData *chunkData,
                            QStringRef *remainingPatch)
{
    bool hasNewLine;
    const QStringRef chunkLine = readLine(chunkDiff, remainingPatch, &hasNewLine);

    const QLatin1String leftPosMarker("@@ -");
    const QLatin1String rightPosMarker(" +");
    const QLatin1String optionalHintMarker(" @@");

    const int leftPosIndex = chunkLine.indexOf(leftPosMarker);
    if (leftPosIndex != 0)
        return false;

    const int rightPosIndex = chunkLine.indexOf(rightPosMarker, leftPosIndex + leftPosMarker.size());
    if (rightPosIndex < 0)
        return false;

    const int optionalHintIndex = chunkLine.indexOf(optionalHintMarker, rightPosIndex + rightPosMarker.size());
    if (optionalHintIndex < 0)
        return false;

    const int leftPosStart = leftPosIndex + leftPosMarker.size();
    const int leftPosLength = rightPosIndex - leftPosStart;
    QStringRef leftPos = chunkLine.mid(leftPosStart, leftPosLength);

    const int rightPosStart = rightPosIndex + rightPosMarker.size();
    const int rightPosLength = optionalHintIndex - rightPosStart;
    QStringRef rightPos = chunkLine.mid(rightPosStart, rightPosLength);

    const int optionalHintStart = optionalHintIndex + optionalHintMarker.size();
    const int optionalHintLength = chunkLine.size() - optionalHintStart;
    const QStringRef optionalHint = chunkLine.mid(optionalHintStart, optionalHintLength);

    const QChar comma(',');
    bool ok;

    const int leftCommaIndex = leftPos.indexOf(comma);
    if (leftCommaIndex >= 0)
        leftPos = leftPos.left(leftCommaIndex);
    const int leftLineNumber = leftPos.toString().toInt(&ok);
    if (!ok)
        return false;

    const int rightCommaIndex = rightPos.indexOf(comma);
    if (rightCommaIndex >= 0)
        rightPos = rightPos.left(rightCommaIndex);
    const int rightLineNumber = rightPos.toString().toInt(&ok);
    if (!ok)
        return false;

    chunkData->leftStartingLineNumber = leftLineNumber - 1;
    chunkData->rightStartingLineNumber = rightLineNumber - 1;
    chunkData->contextInfo = optionalHint.toString();

    return true;
}

static QList<ChunkData> readChunks(QStringRef patch,
    QList<ChunkData> chunkDataList;
    int position = -1;
    QVector<int> startingPositions; // store starting positions of @@
    if (patch.startsWith(QStringLiteral("@@ -")))
        startingPositions.append(position + 1);
    while ((position = patch.indexOf(QStringLiteral("\n@@ -"), position + 1)) >= 0)
        startingPositions.append(position + 1);
    const QChar newLine('\n');
    bool readOk = true;

    const int count = startingPositions.count();
    for (int i = 0; i < count; i++) {
        const int chunkStart = startingPositions.at(i);
        const int chunkEnd = (i < count - 1)
                // drop the newline before the next chunk start
                ? startingPositions.at(i + 1) - 1
                // drop the possible newline by the end of patch
                : (patch.at(patch.count() - 1) == newLine ? patch.count() - 1 : patch.count());

        // extract just one chunk
        const QStringRef chunkDiff = patch.mid(chunkStart, chunkEnd - chunkStart);

        ChunkData chunkData;
        QStringRef lines;
        readOk = detectChunkData(chunkDiff, &chunkData, &lines);

        if (!readOk)
            break;

        chunkData.rows = readLines(lines, i == (startingPositions.size() - 1),
                                   lastChunkAtTheEndOfFile, &readOk);
        if (!readOk)
            break;

        chunkDataList.append(chunkData);
static FileData readDiffHeaderAndChunks(QStringRef headerAndChunks,
    QStringRef patch = headerAndChunks;
    const QRegularExpression leftFileRegExp(
          "(?:\\n|^)-{3} "       // "--- "
          "([^\\t\\n]+)"         // "fileName1"
          "(?:\\t[^\\n]*)*\\n"); // optionally followed by: \t anything \t anything ...)
    const QRegularExpression rightFileRegExp(
          "^\\+{3} "             // "+++ "
          "([^\\t\\n]+)"         // "fileName2"
          "(?:\\t[^\\n]*)*\\n"); // optionally followed by: \t anything \t anything ...)
    const QRegularExpression binaryRegExp(
          "^Binary files ([^\\t\\n]+) and ([^\\t\\n]+) differ$");

    // followed either by leftFileRegExp
    const QRegularExpressionMatch leftMatch = leftFileRegExp.match(patch);
    if (leftMatch.hasMatch() && leftMatch.capturedStart() == 0) {
        patch = patch.mid(leftMatch.capturedEnd());
        fileData.leftFileInfo.fileName = leftMatch.captured(1);
        const QRegularExpressionMatch rightMatch = rightFileRegExp.match(patch);
        if (rightMatch.hasMatch() && rightMatch.capturedStart() == 0) {
            patch = patch.mid(rightMatch.capturedEnd());
            fileData.rightFileInfo.fileName = rightMatch.captured(1);
    } else {
        // or by binaryRegExp
        const QRegularExpressionMatch binaryMatch = binaryRegExp.match(patch);
        if (binaryMatch.hasMatch() && binaryMatch.capturedStart() == 0) {
            fileData.leftFileInfo.fileName = binaryMatch.captured(1);
            fileData.rightFileInfo.fileName = binaryMatch.captured(2);
            fileData.binaryFiles = true;
            readOk = true;
        }
static QList<FileData> readDiffPatch(QStringRef patch,
                                     bool *ok,
                                     QFutureInterfaceBase *jobController)
    const QRegularExpression diffRegExp("(?:\\n|^)"          // new line of the beginning of a patch
                                        "("                  // either
                                        "-{3} "              // ---
                                        "[^\\t\\n]+"         // filename1
                                        "(?:\\t[^\\n]*)*\\n" // optionally followed by: \t anything \t anything ...
                                        "\\+{3} "            // +++
                                        "[^\\t\\n]+"         // filename2
                                        "(?:\\t[^\\n]*)*\\n" // optionally followed by: \t anything \t anything ...
                                        "|"                  // or
                                        "Binary files "
                                        "[^\\t\\n]+"         // filename1
                                        " and "
                                        "[^\\t\\n]+"         // filename2
                                        " differ"
                                        ")");                // end of or
    QRegularExpressionMatch diffMatch = diffRegExp.match(patch);
    if (diffMatch.hasMatch()) {
            if (jobController && jobController->isCanceled())
                return QList<FileData>();

            int pos = diffMatch.capturedStart();
                QStringRef headerAndChunks = patch.mid(lastPos,
                                                       pos - lastPos);
            pos = diffMatch.capturedEnd();
            diffMatch = diffRegExp.match(patch, pos);
        } while (diffMatch.hasMatch());
        if (readOk) {
            QStringRef headerAndChunks = patch.mid(lastPos,
                                                   patch.count() - lastPos - 1);
// The git diff patch format (ChangeFile, NewFile, DeleteFile)
// 0.  <some text lines to skip, e.g. show description>\n
// 1.  diff --git a/[fileName] b/[fileName]\n
// 2a. new file mode [fileModeNumber]\n
// 2b. deleted file mode [fileModeNumber]\n
// 2c. old mode [oldFileModeNumber]\n
//     new mode [newFileModeNumber]\n
// 2d. <Nothing, only in case of ChangeFile>
// 3a.  index [leftIndexSha]..[rightIndexSha] <optionally: octalNumber>
// 3b. <Nothing, only in case of ChangeFile, "Dirty submodule" case>
// 4a. <Nothing more, only possible in case of NewFile or DeleteFile> ???
// 4b. \nBinary files [leftFileNameOrDevNull] and [rightFileNameOrDevNull] differ
// 4c. --- [leftFileNameOrDevNull]\n
//     +++ [rightFileNameOrDevNull]\n
//     <Chunks>

// The git diff patch format (CopyFile, RenameFile)
// 0.  [some text lines to skip, e.g. show description]\n
// 1.  diff --git a/[leftFileName] b/[rightFileName]\n
// 2.  [dis]similarity index [0-100]%\n
//     [copy / rename] from [leftFileName]\n
//     [copy / rename] to [rightFileName]
// 3a. <Nothing more, only when similarity index was 100%>
// 3b. index [leftIndexSha]..[rightIndexSha] <optionally: octalNumber>
// 4.  --- [leftFileNameOrDevNull]\n
//     +++ [rightFileNameOrDevNull]\n
//     <Chunks>

static bool detectIndexAndBinary(QStringRef patch,
                                 FileData *fileData,
                                 QStringRef *remainingPatch)
    bool hasNewLine;
    *remainingPatch = patch;

    if (remainingPatch->isEmpty()) {
        switch (fileData->fileOperation) {
        case FileData::CopyFile:
        case FileData::RenameFile:
        case FileData::ChangeMode:
            // in case of 100% similarity we don't have more lines in the patch
            return true;
        default:
            break;
        }
    }
    QStringRef afterNextLine;
    // index [leftIndexSha]..[rightIndexSha] <optionally: octalNumber>
    const QStringRef nextLine = readLine(patch, &afterNextLine, &hasNewLine);

    const QLatin1String indexHeader("index ");

    if (nextLine.startsWith(indexHeader)) {
        const QStringRef indices = nextLine.mid(indexHeader.size());
        const int dotsPosition = indices.indexOf(QStringLiteral(".."));
        if (dotsPosition < 0)
            return false;
        fileData->leftFileInfo.typeInfo = indices.left(dotsPosition).toString();

        // if there is no space we take the remaining string
        const int spacePosition = indices.indexOf(QChar::Space, dotsPosition + 2);
        const int length = spacePosition < 0 ? -1 : spacePosition - dotsPosition - 2;
        fileData->rightFileInfo.typeInfo = indices.mid(dotsPosition + 2, length).toString();

        *remainingPatch = afterNextLine;
    } else if (fileData->fileOperation != FileData::ChangeFile) {
        // no index only in case of ChangeFile,
        // the dirty submodule diff case, see "Dirty Submodule" test:
        return false;
    }
    if (remainingPatch->isEmpty() && (fileData->fileOperation == FileData::NewFile
                            || fileData->fileOperation == FileData::DeleteFile)) {
        // OK in case of empty file
        return true;
    }
    const QString devNull("/dev/null");
    const QString leftFileName = fileData->fileOperation == FileData::NewFile
            ? devNull : QLatin1String("a/") + fileData->leftFileInfo.fileName;
    const QString rightFileName = fileData->fileOperation == FileData::DeleteFile
            ? devNull : QLatin1String("b/") + fileData->rightFileInfo.fileName;
    const QString binaryLine = "Binary files "
            + leftFileName + " and "
            + rightFileName + " differ";
    if (*remainingPatch == binaryLine) {
        fileData->binaryFiles = true;
        *remainingPatch = QStringRef();
        return true;
    }
    const QString leftStart = "--- " + leftFileName;
    QStringRef afterMinuses;
    // --- leftFileName
    const QStringRef minuses = readLine(*remainingPatch, &afterMinuses, &hasNewLine);
    if (!hasNewLine)
        return false; // we need to have at least one more line
    if (!minuses.startsWith(leftStart))
        return false;
    const QString rightStart = "+++ " + rightFileName;
    QStringRef afterPluses;
    // +++ rightFileName
    const QStringRef pluses = readLine(afterMinuses, &afterPluses, &hasNewLine);
    if (!hasNewLine)
        return false; // we need to have at least one more line
    if (!pluses.startsWith(rightStart))
        return false;
    *remainingPatch = afterPluses;
    return true;
}
static bool extractCommonFileName(QStringRef fileNames, QStringRef *fileName)
{
    // we should have 1 space between filenames
    if (fileNames.size() % 2 == 0)
        return false;
    if (!fileNames.startsWith(QStringLiteral("a/")))
        return false;
    // drop the space in between
    const int fileNameSize = fileNames.size() / 2;
    if (!fileNames.mid(fileNameSize).startsWith(" b/"))
        return false;
    // drop "a/"
    const QStringRef leftFileName = fileNames.mid(2, fileNameSize - 2);
    // drop the first filename + " b/"
    const QStringRef rightFileName = fileNames.mid(fileNameSize + 3, fileNameSize - 2);
    if (leftFileName != rightFileName)
        return false;
    *fileName = leftFileName;
    return true;
static bool detectFileData(QStringRef patch,
                           FileData *fileData,
                           QStringRef *remainingPatch) {
    bool hasNewLine;

    QStringRef afterDiffGit;
    // diff --git a/leftFileName b/rightFileName
    const QStringRef diffGit = readLine(patch, &afterDiffGit, &hasNewLine);
    if (!hasNewLine)
        return false; // we need to have at least one more line

    const QLatin1String gitHeader("diff --git ");
    const QStringRef fileNames = diffGit.mid(gitHeader.size());
    QStringRef commonFileName;
    if (extractCommonFileName(fileNames, &commonFileName)) {
        // change / new / delete

        fileData->fileOperation = FileData::ChangeFile;
        fileData->leftFileInfo.fileName = fileData->rightFileInfo.fileName = commonFileName.toString();

        QStringRef afterSecondLine;
        const QStringRef secondLine = readLine(afterDiffGit, &afterSecondLine, &hasNewLine);

        if (secondLine.startsWith(QStringLiteral("new file mode "))) {
            fileData->fileOperation = FileData::NewFile;
            *remainingPatch = afterSecondLine;
        } else if (secondLine.startsWith(QStringLiteral("deleted file mode "))) {
            fileData->fileOperation = FileData::DeleteFile;
            *remainingPatch = afterSecondLine;
        } else if (secondLine.startsWith(QStringLiteral("old mode "))) {
            QStringRef afterThirdLine;
            // new mode
            readLine(afterSecondLine, &afterThirdLine, &hasNewLine);
            if (!hasNewLine)
                fileData->fileOperation = FileData::ChangeMode;

            // TODO: validate new mode line
            *remainingPatch = afterThirdLine;
        } else {
            *remainingPatch = afterDiffGit;
        }
    } else {
        // copy / rename

        QStringRef afterSimilarity;
        // (dis)similarity index [0-100]%
        readLine(afterDiffGit, &afterSimilarity, &hasNewLine);
        if (!hasNewLine)
            return false; // we need to have at least one more line

        // TODO: validate similarity line

        QStringRef afterCopyRenameFrom;
        // [copy / rename] from leftFileName
        const QStringRef copyRenameFrom = readLine(afterSimilarity, &afterCopyRenameFrom, &hasNewLine);
        if (!hasNewLine)
            return false; // we need to have at least one more line

        const QLatin1String copyFrom("copy from ");
        const QLatin1String renameFrom("rename from ");
        if (copyRenameFrom.startsWith(copyFrom)) {
            fileData->fileOperation = FileData::CopyFile;
            fileData->leftFileInfo.fileName = copyRenameFrom.mid(copyFrom.size()).toString();
        } else if (copyRenameFrom.startsWith(renameFrom)) {
            fileData->fileOperation = FileData::RenameFile;
            fileData->leftFileInfo.fileName = copyRenameFrom.mid(renameFrom.size()).toString();
        } else {
            return false;
        }
        QStringRef afterCopyRenameTo;
        // [copy / rename] to rightFileName
        const QStringRef copyRenameTo = readLine(afterCopyRenameFrom, &afterCopyRenameTo, &hasNewLine);
        // if (dis)similarity index is 100% we don't have more lines
        const QLatin1String copyTo("copy to ");
        const QLatin1String renameTo("rename to ");
        if (fileData->fileOperation == FileData::CopyFile && copyRenameTo.startsWith(copyTo)) {
            fileData->rightFileInfo.fileName = copyRenameTo.mid(copyTo.size()).toString();
        } else if (fileData->fileOperation == FileData::RenameFile && copyRenameTo.startsWith(renameTo)) {
            fileData->rightFileInfo.fileName = copyRenameTo.mid(renameTo.size()).toString();
        } else {
            return false;
        }
        *remainingPatch = afterCopyRenameTo;
    }
    return detectIndexAndBinary(*remainingPatch, fileData, remainingPatch);
}
static QList<FileData> readGitPatch(QStringRef patch, bool *ok,
                                    QFutureInterfaceBase *jobController)
{
    int position = -1;
    QVector<int> startingPositions; // store starting positions of git headers
    if (patch.startsWith(QStringLiteral("diff --git ")))
        startingPositions.append(position + 1);
    while ((position = patch.indexOf(QStringLiteral("\ndiff --git "), position + 1)) >= 0)
        startingPositions.append(position + 1);
    class PatchInfo {
    public:
        QStringRef patch;
        FileData fileData;
    };
    const QChar newLine('\n');
    bool readOk = true;
    QVector<PatchInfo> patches;
    const int count = startingPositions.count();
    for (int i = 0; i < count; i++) {
        if (jobController && jobController->isCanceled())
            return QList<FileData>();
        const int diffStart = startingPositions.at(i);
        const int diffEnd = (i < count - 1)
                // drop the newline before the next header start
                ? startingPositions.at(i + 1) - 1
                // drop the possible newline by the end of file
                : (patch.at(patch.count() - 1) == newLine ? patch.count() - 1 : patch.count());
        // extract the patch for just one file
        const QStringRef fileDiff = patch.mid(diffStart, diffEnd - diffStart);
        FileData fileData;
        QStringRef remainingFileDiff;
        readOk = detectFileData(fileDiff, &fileData, &remainingFileDiff);
        if (!readOk)
            break;
        patches.append(PatchInfo { remainingFileDiff, fileData });
    }
    if (!readOk) {
        if (ok)
            *ok = readOk;
        return QList<FileData>();
    }
    if (jobController)
        jobController->setProgressRange(0, patches.count());
    QList<FileData> fileDataList;
    readOk = false;
    int i = 0;
    for (const auto &patchInfo : qAsConst(patches)) {
        if (jobController) {
            if (jobController->isCanceled())
                return QList<FileData>();
            jobController->setProgressValue(i++);
        }
        FileData fileData = patchInfo.fileData;
        if (!patchInfo.patch.isEmpty() || fileData.fileOperation == FileData::ChangeFile)
            fileData.chunks = readChunks(patchInfo.patch, &fileData.lastChunkAtTheEndOfFile, &readOk);
        else
            readOk = true;
        if (!readOk)
            break;
        fileDataList.append(fileData);
QList<FileData> DiffUtils::readPatch(const QString &patch, bool *ok,
                                     QFutureInterfaceBase *jobController)
    if (jobController) {
        jobController->setProgressRange(0, 1);
        jobController->setProgressValue(0);
    }
    QStringRef croppedPatch(&patch);
    const QRegularExpression formatPatchEndingRegExp("(\\n-- \\n\\S*\\n\\n$)");
    const QRegularExpressionMatch match = formatPatchEndingRegExp.match(croppedPatch);
    if (match.hasMatch())
        croppedPatch = croppedPatch.left(match.capturedStart() + 1);
    fileDataList = readGitPatch(croppedPatch, &readOk, jobController);
        fileDataList = readDiffPatch(croppedPatch, &readOk, jobController);