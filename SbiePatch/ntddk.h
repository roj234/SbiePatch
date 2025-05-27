/*
本文件的所有部分均来自learn.microsoft.com上的公开文本，本文件与Microsoft及其关联公司没有任何关系，本文件代码的版权归其所有者。
*/

#include <windows.h>
#include <stdint.h>

typedef int16_t CSHORT;
typedef struct {} _DRIVER_OBJECT;
typedef struct {} _DEVICE_OBJECT;
typedef void *PDRIVER_EXTENSION;
typedef void *PFAST_IO_DISPATCH;
typedef void* PMDL;
#define IRP_MJ_MAXIMUM_FUNCTION 27
#define STATUS_SUCCESS 0
#define PAGED_CODE()



typedef struct _IO_STATUS_BLOCK {
  union {
    NTSTATUS Status;
    PVOID    Pointer;
  };
  ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _IRP {
  CSHORT                    Type;
  USHORT                    Size;
  PMDL                      MdlAddress;
  ULONG                     Flags;
  union {
    struct _IRP     *MasterIrp;
    __volatile LONG IrpCount;
    PVOID           SystemBuffer;
  } AssociatedIrp;
  LIST_ENTRY                ThreadListEntry;
  IO_STATUS_BLOCK           IoStatus;
/*  KPROCESSOR_MODE           RequestorMode;
  BOOLEAN                   PendingReturned;
  CHAR                      StackCount;
  CHAR                      CurrentLocation;
  BOOLEAN                   Cancel;
  KIRQL                     CancelIrql;
  CCHAR                     ApcEnvironment;
  UCHAR                     AllocationFlags;
  union {
    PIO_STATUS_BLOCK UserIosb;
    PVOID            IoRingContext;
  };
  PKEVENT                   UserEvent;
  union {
    struct {
      union {
        PIO_APC_ROUTINE UserApcRoutine;
        PVOID           IssuingProcess;
      };
      union {
        PVOID                 UserApcContext;
#if ...
        _IORING_OBJECT        *IoRing;
#else
        struct _IORING_OBJECT *IoRing;
#endif
      };
    } AsynchronousParameters;
    LARGE_INTEGER AllocationSize;
  } Overlay;
  __volatile PDRIVER_CANCEL CancelRoutine;
  PVOID                     UserBuffer;
  union {
    struct {
      union {
        KDEVICE_QUEUE_ENTRY DeviceQueueEntry;
        struct {
          PVOID DriverContext[4];
        };
      };
      PETHREAD     Thread;
      PCHAR        AuxiliaryBuffer;
      struct {
        LIST_ENTRY ListEntry;
        union {
          struct _IO_STACK_LOCATION *CurrentStackLocation;
          ULONG                     PacketType;
        };
      };
      PFILE_OBJECT OriginalFileObject;
    } Overlay;
    KAPC  Apc;
    PVOID CompletionKey;
  } Tail;*/
} IRP, *PIRP;

ULONG DbgPrint(
  PCSTR Format,
  ...   
);

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

NTSYSAPI SIZE_T RtlCompareMemory(
IN const VOID *Source1,
IN const VOID *Source2,
IN SIZE_T Length
);
NTSYSAPI LONG RtlCompareUnicodeString(
  IN PUNICODE_STRING String1,
  IN PUNICODE_STRING String2,
  IN BOOLEAN          CaseInSensitive
);
NTSYSAPI VOID RtlInitUnicodeString(
  OUT          PUNICODE_STRING         DestinationString,
  IN OPTIONAL PCWSTR SourceString
);

PVOID MmGetSystemRoutineAddress(
  IN PUNICODE_STRING SystemRoutineName
);

typedef NTSTATUS DRIVER_INITIALIZE(
  IN _DRIVER_OBJECT *DriverObject,
  IN PUNICODE_STRING RegistryPath
), *PDRIVER_INITIALIZE;
typedef NTSTATUS DRIVER_STARTIO(
  IN OUT _DEVICE_OBJECT *DeviceObject,
  IN OUT IRP *Irp
), *PDRIVER_STARTIO;
typedef NTSTATUS DRIVER_UNLOAD(
  IN OUT _DEVICE_OBJECT *DeviceObject
), *PDRIVER_UNLOAD;
typedef NTSTATUS DRIVER_DISPATCH(
  IN OUT _DEVICE_OBJECT *DeviceObject,
  IN OUT IRP *Irp
), *PDRIVER_DISPATCH;

typedef struct{} IO_TIMER, *PIO_TIMER;
typedef struct{} VPB, *PVPB;

typedef struct _DEVICE_OBJECT {
  CSHORT                   Type;
  USHORT                   Size;
  LONG                     ReferenceCount;
  struct _DRIVER_OBJECT    *DriverObject;
  struct _DEVICE_OBJECT    *NextDevice;
  struct _DEVICE_OBJECT    *AttachedDevice;
  struct _IRP              *CurrentIrp;
  PIO_TIMER                Timer;
  ULONG                    Flags;
  /*ULONG                    Characteristics;
  __volatile PVPB          Vpb;
  PVOID                    DeviceExtension;
  DEVICE_TYPE              DeviceType;
  CCHAR                    StackSize;
  union {
    LIST_ENTRY         ListEntry;
    WAIT_CONTEXT_BLOCK Wcb;
  } Queue;
  ULONG                    AlignmentRequirement;
  KDEVICE_QUEUE            DeviceQueue;
  KDPC                     Dpc;
  ULONG                    ActiveThreadCount;
  PSECURITY_DESCRIPTOR     SecurityDescriptor;
  KEVENT                   DeviceLock;
  USHORT                   SectorSize;
  USHORT                   Spare1;
  struct _DEVOBJ_EXTENSION *DeviceObjectExtension;
  PVOID                    Reserved;*/
} DEVICE_OBJECT, *PDEVICE_OBJECT;

typedef struct _DRIVER_OBJECT {
  CSHORT             Type;
  CSHORT             Size;
  PDEVICE_OBJECT     DeviceObject;
  ULONG              Flags;
  PVOID              DriverStart;
  ULONG              DriverSize;
  PVOID              DriverSection;
  PDRIVER_EXTENSION  DriverExtension;
  UNICODE_STRING     DriverName;
  PUNICODE_STRING    HardwareDatabase;
  PFAST_IO_DISPATCH  FastIoDispatch;
  PDRIVER_INITIALIZE DriverInit;
  PDRIVER_STARTIO    DriverStartIo;
  PDRIVER_UNLOAD     DriverUnload;
  PDRIVER_DISPATCH   MajorFunction[IRP_MJ_MAXIMUM_FUNCTION + 1];
} DRIVER_OBJECT, *PDRIVER_OBJECT;

typedef struct _LDR_DATA_TABLE_ENTRY {
LIST_ENTRY InLoadOrderLinks;
LIST_ENTRY InMemoryOrderLinks;
LIST_ENTRY InInitializationOrderLinks;
PVOID DllBase;
PVOID EntryPoint;
ULONG SizeOfImage;
UNICODE_STRING FullDllName;
UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


typedef enum _MODE { 
    KernelMode, 
    UserMode, 
    MaximumMode 
} MODE;
typedef MODE KPROCESSOR_MODE;

typedef enum _MEMORY_CACHING_TYPE {
  MmNonCached,
  MmCached,
  MmWriteCombined,
  MmHardwareCoherentCached,
  MmNonCachedUnordered,
  MmUSWCCached,
  MmMaximumCacheType,
  MmNotMapped
} MEMORY_CACHING_TYPE;

typedef enum _LOCK_OPERATION {
  IoReadAccess,
  IoWriteAccess,
  IoModifyAccess
} LOCK_OPERATION;

PMDL IoAllocateMdl(
  IN OPTIONAL       PVOID VirtualAddress,
  IN                ULONG                  Length,
  IN                BOOLEAN                SecondaryBuffer,
  IN                BOOLEAN                ChargeQuota,
  IN OUT OPTIONAL PIRP                   Irp
);
void IoFreeMdl(
  IN PMDL Mdl
);

PVOID MmMapLockedPagesSpecifyCache(
  IN           PMDL                                                                          MemoryDescriptorList,
  IN           KPROCESSOR_MODE AccessMode,
  IN           MEMORY_CACHING_TYPE                      CacheType,
  IN OPTIONAL PVOID                                                                         RequestedAddress,
  IN           ULONG                                                                         BugCheckOnFailure,
  IN           ULONG                                                                         Priority
);
void MmProbeAndLockPages(
  IN OUT PMDL            MemoryDescriptorList,
  IN      KPROCESSOR_MODE AccessMode,
  IN      LOCK_OPERATION  Operation
);
void MmUnmapLockedPages(
  IN PVOID BaseAddress,
  IN PMDL  MemoryDescriptorList
);
void MmUnlockPages(
  IN OUT PMDL MemoryDescriptorList
);

#define NT_SUCCESS(x) ((x) == 0)

typedef struct{} KEVENT, *PKEVENT;
typedef struct{} SECTION_OBJECT_POINTERS, *PSECTION_OBJECT_POINTERS;

typedef struct _FILE_OBJECT {
  CSHORT                                Type;
  CSHORT                                Size;
  PDEVICE_OBJECT                        DeviceObject;
  PVPB                                  Vpb;
  PVOID                                 FsContext;
  PVOID                                 FsContext2;
  PSECTION_OBJECT_POINTERS              SectionObjectPointer;
  PVOID                                 PrivateCacheMap;
  NTSTATUS                              FinalStatus;
  struct _FILE_OBJECT                   *RelatedFileObject;
  BOOLEAN                               LockOperation;
  BOOLEAN                               DeletePending;
  BOOLEAN                               ReadAccess;
  BOOLEAN                               WriteAccess;
  BOOLEAN                               DeleteAccess;
  BOOLEAN                               SharedRead;
  BOOLEAN                               SharedWrite;
  BOOLEAN                               SharedDelete;
  ULONG                                 Flags;
  UNICODE_STRING                        FileName;
  LARGE_INTEGER                         CurrentByteOffset;
  __volatile ULONG                      Waiters;
  __volatile ULONG                      Busy;
  PVOID                                 LastLock;
  KEVENT                                Lock;
  KEVENT                                Event;
/*  __volatile PIO_COMPLETION_CONTEXT     CompletionContext;
  KSPIN_LOCK                            IrpListLock;
  LIST_ENTRY                            IrpList;
  __volatile _IOP_FILE_OBJECT_EXTENSION *FileObjectExtension;
  struct                                _IOP_FILE_OBJECT_EXTENSION;*/
} FILE_OBJECT, *PFILE_OBJECT;

NTSTATUS IoGetDeviceObjectPointer(
  IN  PUNICODE_STRING ObjectName,
  IN  ACCESS_MASK     DesiredAccess,
  OUT PFILE_OBJECT    *FileObject,
  OUT PDEVICE_OBJECT  *DeviceObject
);

typedef enum {
  NotificationEvent = 3
} EVENT_TYPE;

void KeInitializeEvent(
  OUT PKEVENT   Event,
  IN  EVENT_TYPE Type,
  IN  BOOLEAN    State
);

typedef enum {
   Executive = 0, 
} KWAIT_REASON;

NTSTATUS KeWaitForSingleObject (
    PVOID Object,
    KWAIT_REASON WaitReason,
    KPROCESSOR_MODE WaitMode,
    BOOLEAN Alertable,
    PLARGE_INTEGER Timeout
);

NTSTATUS IoCallDriver(
  PDEVICE_OBJECT        DeviceObject,
  PIRP Irp
);
__drv_aliasesMem PIRP IoBuildDeviceIoControlRequest(
  IN            ULONG            IoControlCode,
  IN            PDEVICE_OBJECT   DeviceObject,
  IN OPTIONAL  PVOID            InputBuffer,
  IN            ULONG            InputBufferLength,
  OUT OPTIONAL PVOID            OutputBuffer,
  IN            ULONG            OutputBufferLength,
  IN            BOOLEAN          InternalDeviceIoControl,
  IN OPTIONAL  PKEVENT          Event,
   OUT           PIO_STATUS_BLOCK IoStatusBlock
);

void IoFreeIrp(
  IN PIRP Irp
);

typedef enum {
  IRP_MJ_CREATE = 0, IRP_wobuzhidao1, IRP_MJ_CLOSE, IRP_MJ_READ, IRP_MJ_WRITE, 
  IRP_MJ_DEVICE_CONTROL = 14, IRP_MJ_CLEANUP = 18
} IRP_ID;

void IoCompleteRequest(
  PIRP  Irp,
  CCHAR PriorityBoost
);
NTSTATUS IoCreateDevice(
  IN           PDRIVER_OBJECT  DriverObject,
  IN           ULONG           DeviceExtensionSize,
  IN OPTIONAL  PUNICODE_STRING DeviceName,
  IN           DEVICE_TYPE     DeviceType,
  IN           ULONG           DeviceCharacteristics,
  IN           BOOLEAN         Exclusive,
  OUT          PDEVICE_OBJECT  *DeviceObject
);


typedef struct {} IO_SECURITY_CONTEXT, *PIO_SECURITY_CONTEXT;
// 还好我用不到
#define POINTER_ALIGNMENT 
typedef struct _IO_STACK_LOCATION {
  UCHAR                  MajorFunction;
  UCHAR                  MinorFunction;
  UCHAR                  Flags;
  UCHAR                  Control;
  union {
    struct {
      PIO_SECURITY_CONTEXT     SecurityContext;
      ULONG                    Options;
      USHORT POINTER_ALIGNMENT FileAttributes;
      USHORT                   ShareAccess;
      ULONG POINTER_ALIGNMENT  EaLength;
    } Create;
    /*struct {
      PIO_SECURITY_CONTEXT          SecurityContext;
      ULONG                         Options;
      USHORT POINTER_ALIGNMENT      Reserved;
      USHORT                        ShareAccess;
      PNAMED_PIPE_CREATE_PARAMETERS Parameters;
    } CreatePipe;
    struct {
      PIO_SECURITY_CONTEXT        SecurityContext;
      ULONG                       Options;
      USHORT POINTER_ALIGNMENT    Reserved;
      USHORT                      ShareAccess;
      PMAILSLOT_CREATE_PARAMETERS Parameters;
    } CreateMailslot;*/
    struct {
      ULONG                   Length;
      ULONG POINTER_ALIGNMENT Key;
      ULONG                   Flags;
      LARGE_INTEGER           ByteOffset;
    } Read;
    struct {
      ULONG                   Length;
      ULONG POINTER_ALIGNMENT Key;
      ULONG                   Flags;
      LARGE_INTEGER           ByteOffset;
    } Write;
    /*struct {
      ULONG                   Length;
      PUNICODE_STRING         FileName;
      FILE_INFORMATION_CLASS  FileInformationClass;
      ULONG POINTER_ALIGNMENT FileIndex;
    } QueryDirectory;
    struct {
      ULONG                   Length;
      ULONG POINTER_ALIGNMENT CompletionFilter;
    } NotifyDirectory;
    struct {
      ULONG                                                Length;
      ULONG POINTER_ALIGNMENT                              CompletionFilter;
      DIRECTORY_NOTIFY_INFORMATION_CLASS POINTER_ALIGNMENT DirectoryNotifyInformationClass;
    } NotifyDirectoryEx;
    struct {
      ULONG                                    Length;
      FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
    } QueryFile;
    struct {
      ULONG                                    Length;
      FILE_INFORMATION_CLASS POINTER_ALIGNMENT FileInformationClass;
      PFILE_OBJECT                             FileObject;
      union {
        struct {
          BOOLEAN ReplaceIfExists;
          BOOLEAN AdvanceOnly;
        };
        ULONG  ClusterCount;
        HANDLE DeleteHandle;
      };
    } SetFile;
    struct {
      ULONG                   Length;
      PVOID                   EaList;
      ULONG                   EaListLength;
      ULONG POINTER_ALIGNMENT EaIndex;
    } QueryEa;
    struct {
      ULONG Length;
    } SetEa;
    struct {
      ULONG                                  Length;
      FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
    } QueryVolume;
    struct {
      ULONG                                  Length;
      FS_INFORMATION_CLASS POINTER_ALIGNMENT FsInformationClass;
    } SetVolume;
    struct {
      ULONG                   OutputBufferLength;
      ULONG POINTER_ALIGNMENT InputBufferLength;
      ULONG POINTER_ALIGNMENT FsControlCode;
      PVOID                   Type3InputBuffer;
    } FileSystemControl;
    struct {
      PLARGE_INTEGER          Length;
      ULONG POINTER_ALIGNMENT Key;
      LARGE_INTEGER           ByteOffset;
    } LockControl;
    struct {
      ULONG                   OutputBufferLength;
      ULONG POINTER_ALIGNMENT InputBufferLength;
      ULONG POINTER_ALIGNMENT IoControlCode;
      PVOID                   Type3InputBuffer;
    } DeviceIoControl;
    struct {
      SECURITY_INFORMATION    SecurityInformation;
      ULONG POINTER_ALIGNMENT Length;
    } QuerySecurity;
    struct {
      SECURITY_INFORMATION SecurityInformation;
      PSECURITY_DESCRIPTOR SecurityDescriptor;
    } SetSecurity;
    struct {
      PVPB           Vpb;
      PDEVICE_OBJECT DeviceObject;
      ULONG          OutputBufferLength;
    } MountVolume;
    struct {
      PVPB           Vpb;
      PDEVICE_OBJECT DeviceObject;
    } VerifyVolume;
    struct {
      struct _SCSI_REQUEST_BLOCK *Srb;
    } Scsi;
    struct {
      ULONG                       Length;
      PSID                        StartSid;
      PFILE_GET_QUOTA_INFORMATION SidList;
      ULONG                       SidListLength;
    } QueryQuota;
    struct {
      ULONG Length;
    } SetQuota;
    struct {
      DEVICE_RELATION_TYPE Type;
    } QueryDeviceRelations;
    struct {
      const GUID *InterfaceType;
      USHORT     Size;
      USHORT     Version;
      PINTERFACE Interface;
      PVOID      InterfaceSpecificData;
    } QueryInterface;
    struct {
      PDEVICE_CAPABILITIES Capabilities;
    } DeviceCapabilities;
    struct {
      PIO_RESOURCE_REQUIREMENTS_LIST IoResourceRequirementList;
    } FilterResourceRequirements;
    struct {
      ULONG                   WhichSpace;
      PVOID                   Buffer;
      ULONG                   Offset;
      ULONG POINTER_ALIGNMENT Length;
    } ReadWriteConfig;
    struct {
      BOOLEAN Lock;
    } SetLock;
    struct {
      BUS_QUERY_ID_TYPE IdType;
    } QueryId;
    struct {
      DEVICE_TEXT_TYPE       DeviceTextType;
      LCID POINTER_ALIGNMENT LocaleId;
    } QueryDeviceText;
    struct {
      BOOLEAN                                          InPath;
      BOOLEAN                                          Reserved[3];
      DEVICE_USAGE_NOTIFICATION_TYPE POINTER_ALIGNMENT Type;
    } UsageNotification;
    struct {
      SYSTEM_POWER_STATE PowerState;
    } WaitWake;
    struct {
      PPOWER_SEQUENCE PowerSequence;
    } PowerSequence;
#if ...
    struct {
      union {
        ULONG                      SystemContext;
        SYSTEM_POWER_STATE_CONTEXT SystemPowerStateContext;
      };
      POWER_STATE_TYPE POINTER_ALIGNMENT Type;
      POWER_STATE POINTER_ALIGNMENT      State;
      POWER_ACTION POINTER_ALIGNMENT     ShutdownType;
    } Power;
#else
    struct {
      ULONG                              SystemContext;
      POWER_STATE_TYPE POINTER_ALIGNMENT Type;
      POWER_STATE POINTER_ALIGNMENT      State;
      POWER_ACTION POINTER_ALIGNMENT     ShutdownType;
    } Power;
#endif
    struct {
      PCM_RESOURCE_LIST AllocatedResources;
      PCM_RESOURCE_LIST AllocatedResourcesTranslated;
    } StartDevice;
    struct {
      ULONG_PTR ProviderId;
      PVOID     DataPath;
      ULONG     BufferSize;
      PVOID     Buffer;
    } WMI;
    struct {
      PVOID Argument1;
      PVOID Argument2;
      PVOID Argument3;
      PVOID Argument4;
    } Others;*/
  } Parameters;
  //PDEVICE_OBJECT         DeviceObject;
  //PFILE_OBJECT           FileObject;
  //PIO_COMPLETION_ROUTINE CompletionRoutine;
  //PVOID                  Context;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

void IoDeleteDevice(
  IN PDEVICE_OBJECT DeviceObject
);

NTSTATUS IoCreateSymbolicLink(
  IN PUNICODE_STRING SymbolicLinkName,
  IN PUNICODE_STRING DeviceName
);
__drv_aliasesMem PIO_STACK_LOCATION IoGetCurrentIrpStackLocation(
  IN PIRP Irp
);

#define IO_NO_INCREMENT              0
#define DO_BUFFERED_IO               0x00000004