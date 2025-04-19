#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device
#define DIRSIZ    14

struct stat {
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
};

// Add to kernel/stat.h (at the end of the file)
struct file_details {
  int inum;           // inode number
  short type;         // file type
  short major;        // major device number
  short minor;        // minor device number
  short nlink;        // number of links
  uint size;          // file size in bytes
  char name[DIRSIZ];  // file name
};