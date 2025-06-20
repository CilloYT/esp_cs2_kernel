#include <ntifs.h>


extern "C" 
{
	NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName, PDRIVER_INITIALIZE InitializationFunction);
	
	NTKERNELAPI NTSTATUS MmCopyVirtualMemory
	(
		PEPROCESS SourceProcess,
		PVOID SourceAddress,
		PEPROCESS TargetProcess,
		PVOID TargetAddress,
		SIZE_T BufferSize,
		KPROCESSOR_MODE PreviousMode,
		PSIZE_T ReturnSize
	);
}


void debug_print(PCSTR txt)
{
#ifndef DEBUG
	UNREFERENCED_PARAMETER(txt);
#endif // DEBUG
	DbgPrintEx(0, 0, txt);
}


namespace driver
{
	namespace codes 
	{
		constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
		constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
	}


	struct Request 
	{
		HANDLE p_id;

		PVOID target;
		PVOID buffer;

		SIZE_T size;
		SIZE_T return_size;
	};

	NTSTATUS create(PDEVICE_OBJECT DeviceObject, PIRP irp)
	{
		UNREFERENCED_PARAMETER(DeviceObject);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}


	NTSTATUS close(PDEVICE_OBJECT DeviceObject, PIRP irp)
	{
		UNREFERENCED_PARAMETER(DeviceObject);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}

	NTSTATUS device_control(PDEVICE_OBJECT DeviceObject, PIRP irp)
	{
		UNREFERENCED_PARAMETER(DeviceObject);

		debug_print("[+] Device control called\n");

		NTSTATUS status = STATUS_UNSUCCESSFUL;

		PIO_STACK_LOCATION stack_irp = IoGetCurrentIrpStackLocation(irp);

		auto request = reinterpret_cast<Request*>(irp->AssociatedIrp.SystemBuffer);

		if (stack_irp == nullptr || request == nullptr)
		{
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return status;
		}

		static PEPROCESS target_process = nullptr;

		const ULONG control_code = stack_irp->Parameters.DeviceIoControl.IoControlCode;
		
		switch (control_code)
		{
			case codes::attach:
				status = PsLookupProcessByProcessId(request->p_id, &target_process);
				break;

			case codes::read:
				if (target_process != nullptr)
				{
					status = MmCopyVirtualMemory(
						target_process,
						request->target,
						PsGetCurrentProcess(),
						request->buffer,
						request->size,
						KernelMode,
						&request->return_size
					);
				}
				break;

			case codes::write:
				if (target_process != nullptr)
				{
					status = MmCopyVirtualMemory(
						PsGetCurrentProcess(),
						request->buffer,
						target_process,
						request->target,
						request->size,
						KernelMode,
						&request->return_size
					);
				}
				break;

			default:
				break;
		}

		irp->IoStatus.Status = status;
		irp->IoStatus.Information = sizeof(request);

		IoCompleteRequest(irp, IO_NO_INCREMENT);

		return irp->IoStatus.Status;
	}
}


NTSTATUS driver_main(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{

	UNREFERENCED_PARAMETER(RegistryPath);

	UNICODE_STRING device_name = {};
	RtlInitUnicodeString(&device_name, L"\\Device\\crazydriver");

	PDEVICE_OBJECT device_object = nullptr;
	NTSTATUS status = IoCreateDevice(DriverObject, 0, &device_name,		
		FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &device_object);

	if (!NT_SUCCESS(status))
	{
		debug_print("[-] Couldn't create device\n");
		return status;
	}

	debug_print("[+] Created device!\n");

	UNICODE_STRING symbolic_link = {};
	RtlInitUnicodeString(&symbolic_link, L"\\DosDevices\\crazydriver");

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);

	if (status != STATUS_SUCCESS)
	{
		debug_print("[-] Couldn't create symbolic link\n");
		return status;
	}

	debug_print("[+] Created symbolic link!\n");


	SetFlag(device_object->Flags, DO_BUFFERED_IO);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = driver::create;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = driver::close;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = driver::device_control;


	ClearFlag(device_object->Flags, DO_DEVICE_INITIALIZING);

	debug_print("[+] Driver initialized successfully!\n");

	return status;
}

NTSTATUS DriverEntry()
{
	debug_print("[+] driver loaded\n");


	UNICODE_STRING driver_name = {};
	RtlInitUnicodeString(&driver_name, L"\\Driver\\crazydriver"); 



	return IoCreateDriver(&driver_name, &driver_main);
}