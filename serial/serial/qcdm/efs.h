#ifndef SERIAL_QCDM_EFS_H
#define SERIAL_QCDM_EFS_H

namespace Serial {
    namespace QCDM {
        enum DiagEFSCommandCode: unsigned short {
            DIAG_EFS_HELLO                  = 0,  // Parameter negotiation packet
            DIAG_EFS_QUERY                  = 1,  // Send information about EFS2 params
            DIAG_EFS_OPEN                   = 2,  // Open a file
            DIAG_EFS_CLOSE                  = 3,  // Close a file
            DIAG_EFS_READ                   = 4,  // Read a file
            DIAG_EFS_WRITE                  = 5,  // Write a file
            DIAG_EFS_SYMLINK                = 6,  // Create a symbolic link
            DIAG_EFS_READLINK               = 7,  // Read a symbolic link
            DIAG_EFS_UNLINK                 = 8,  // Remove a symbolic link or file
            DIAG_EFS_MKDIR                  = 9,  // Create a directory
            DIAG_EFS_RMDIR                  = 10, // Remove a directory
            DIAG_EFS_OPENDIR                = 11, // Open a directory for reading
            DIAG_EFS_READDIR                = 12, // Read a directory
            DIAG_EFS_CLOSEDIR               = 13, // Close an open directory
            DIAG_EFS_RENAME                 = 14, // Rename a file or directory
            DIAG_EFS_STAT                   = 15, // Obtain information about a named file
            DIAG_EFS_LSTAT                  = 16, // Obtain information about a symbolic link
            DIAG_EFS_FSTAT                  = 17, // Obtain information about a file descriptor
            DIAG_EFS_CHMOD                  = 18, // Change file permissions
            DIAG_EFS_STATFS                 = 19, // Obtain file system information
            DIAG_EFS_ACCESS                 = 20, // Check a named file for accessibility
            DIAG_EFS_DEV_INFO               = 21, // Get flash device info
            DIAG_EFS_FACT_IMAGE_START       = 22, // Start data output for Factory Image
            DIAG_EFS_FACT_IMAGE_READ        = 23, // Get data for Factory Image
            DIAG_EFS_FACT_IMAGE_END         = 24, // End data output for Factory Image
            DIAG_EFS_PREP_FACT_IMAGE        = 25, // Prepare file system for image dump
            DIAG_EFS_PUT_DEPRECATED         = 26, // Write an EFS item file
            DIAG_EFS_GET_DEPRECATED         = 27, // Read an EFS item file
            DIAG_EFS_ERROR                  = 28, // Semd an EFS Error Packet back through DIAG
            DIAG_EFS_EXTENDED_INFO          = 29, // Get Extra information.
            DIAG_EFS_CHOWN                  = 30, // Change ownership
            DIAG_EFS_BENCHMARK_START_TEST	= 31, // Start Benchmark
            DIAG_EFS_BENCHMARK_GET_RESULTS	= 32, // Get Benchmark Report
            DIAG_EFS_BENCHMARK_INIT			= 33, // Init/Reset Benchmark
            DIAG_EFS_SET_RESERVATION		= 34, // Set group reservation
            DIAG_EFS_SET_QUOTA              = 35, // Set group quota
            DIAG_EFS_GET_GROUP_INFO			= 36, // Retrieve Q&R values
            DIAG_EFS_DELTREE				= 37, // Delete a Directory Tree
            DIAG_EFS_PUT                    = 38, // Write a EFS item file in order
            DIAG_EFS_GET                    = 39, // Read a EFS item file in order
            DIAG_EFS_TRUNCATE				= 40, // Truncate a file by the name
            DIAG_EFS_FTRUNCATE              = 41, // Truncate a file by a descriptor
            DIAG_EFS_STATVFS_V2             = 42, // Obtains extensive file system info
            DIAG_EFS_MD5SUM                 = 43, // Calculate md5 hash of a file
            DIAG_EFS_HOTPLUG_FORMAT			= 44, // Format a Connected device
            DIAG_EFS_SHRED                  = 45, // Shred obsolete file content.
            DIAG_EFS_SET_IDLE_DEV_EVT_DUR	= 46, // Idle_dev_evt_dur value in mins
            DIAG_EFS_HOTPLUG_DEVICE_INFO	= 47, // get the hotplug device info.
            DIAG_EFS_SYNC_NO_WAIT			= 48, // non-blocking sync of remotefs device
            DIAG_EFS_SYNC_GET_STATUS		= 49, // query previously issued sync status
            DIAG_EFS_TRUNCATE64             = 50, // Truncate a file by the name.
            DIAG_EFS_FTRUNCATE64			= 51, // Truncate a file by a descriptor.
            DIAG_EFS_LSEEK64				= 52, // Seek to requested file offset.
            DIAG_EFS_MAKE_GOLDEN_COPY		= 53, // Make golden copy for Remote Storage
            DIAG_EFS_FILESYSTEM_IMAGE_OPEN	= 54, // Open FileSystem Image extraction
            DIAG_EFS_FILESYSTEM_IMAGE_READ	= 55, // Read File System Image.
            DIAG_EFS_FILESYSTEM_IMAGE_CLOSE	= 56, // Close File System Image.
        };

        enum DIAG_EFS_ERROR : unsigned char {
            DIAG_EFS_INCONSISTENT_STATE     = 0x01,
            DIAG_EFS_INVALID_SEQ_NO         = 0x02,
            DIAG_EFS_DIR_NOT_OPEN           = 0x03,
            DIAG_EFS_DIRENT_NOT_FOUND       = 0x04,
            DIAG_EFS_INVALID_PATH           = 0x05,
            DIAG_EFS_PATH_TOO_LONG          = 0x06,
            DIAG_EFS_TOO_MANY_OPEN_DIRS     = 0x07,
            DIAG_EFS_INVALID_DIR_ENTRY      = 0x08,
            DIAG_EFS_TOO_MANY_OPEN_FILES    = 0x09,
            DIAG_EFS_UNKNOWN_FILETYPE       = 0x0A,
            DIAG_EFS_NOT_NAND_FLASH         = 0x0B,
            DIAG_EFS_UNAVAILABLE_INFO       = 0x0C,
        };


        enum DIAG_EFS_FILE_TYPES : unsigned char {
            DIAG_EFS_FILE_TYPE_FILE         = 0x00,
            DIAG_EFS_FILE_TYPE_DIR          = 0x01,
            DIAG_EFS_FILE_TYPE_LINK         = 0x02,
            DIAG_EFS_FILE_TYPE_IMMOVABLE    = 0x03,
        };

        enum DIAG_EFS_DEVICE_TYPES : unsigned char {
            DIAG_EFS_DEVICE_TYPE_NOR        = 0x00,
            DIAG_EFS_DEVICE_TYPE_NAND       = 0x01
        };

        enum DIAG_EFS_FILESYSTEM_IMAGE_TYPES : unsigned char {
            DIAG_EFS_FILESYSTEM_IMAGE_TAR   = 0,
            DIAG_EFS_FILESYSTEM_IMAGE_ZIP   = 1
        };
    }
}

#endif // SERIAL_QCDM_EFS_H
