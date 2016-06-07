#ifndef UPDATER_H
#define UPDATER_H

namespace Updater {
    enum UpdateStatus {
        UpdateStatusDownload = 0,
        UpdateStatusDownloadProgress,
        UpdateStatusInstall,
        UpdateStatusInstallProgress,
        UpdateStatusError,
        UpdateStatusFinished,
    };
}

#endif // UPDATER_H
