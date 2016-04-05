#include "HelloWDM.h"

#pragma PAGEDCODE
NTSTATUS HelloWDMAddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMAddDevice\n"));

	NTSTATUS status;
	PDEVICE_OBJECT pDeviceObject;
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, L"\\Device\\MyWDMDevice");
	status = IoCreateDevice(DriverObject,
		sizeof(DEVICE_EXTENSION),
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&pDeviceObject);

	if (!NT_SUCCESS(status))
		return status;

	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)pDeviceObject->DeviceExtension;
	pDeviceExtension->pDeviceObject = pDeviceObject;
	pDeviceExtension->pNextStackDevice = IoAttachDeviceToDeviceStack(pDeviceObject, PhysicalDeviceObject);
	pDeviceExtension->ustrDeviceName = devName;

	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, L"\\DosDevices\\HelloWDM");
	pDeviceExtension->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&symLinkName, &devName);

	if (!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&pDeviceExtension->ustrSymLinkName);
		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if (!NT_SUCCESS(status))
		{
			return status;
		}
	}

	pDeviceObject->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

	KdPrint(("Leave AddDevice\n"));
	return STATUS_SUCCESS;
}


#pragma PAGEDCODE
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter DefaultPnpHandler\n"));

	IoSkipCurrentIrpStackLocation(Irp);
	KdPrint(("Leave DefaultPnpHandler\n"));

	return IoCallDriver(pdx->pNextStackDevice, Irp);
}

#pragma PAGEDCODE
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pDeviceExtension, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleRemoveDevice\n"));

	Irp->IoStatus.Status = STATUS_SUCCESS;
	NTSTATUS status = DefaultPnpHandler(pDeviceExtension, Irp);
	IoDeleteSymbolicLink(&pDeviceExtension->ustrSymLinkName);

	if (pDeviceExtension->pNextStackDevice)
		IoDetachDevice(pDeviceExtension->pNextStackDevice);

	IoDeleteDevice(pDeviceExtension->pDeviceObject);
	KdPrint(("Leave HandleRemoveDevice\n"));

	return status;
}

#pragma PAGEDCODE
NTSTATUS HelloWDMPnp(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	PAGED_CODE();

	KdPrint(("Enter HelloWDMPnp\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pDeviceExtension = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	
	static NTSTATUS(*fcntab[])(PDEVICE_EXTENSION pdx, PIRP Irp) =
	{
		DefaultPnpHandler,
		DefaultPnpHandler,
		HandleRemoveDevice,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler,
		DefaultPnpHandler
	};

	ULONG fcn = stack->MinorFunction;
	if (fcn > arraysize(fcntab))
	{
		status = DefaultPnpHandler(pDeviceExtension, Irp);

		return status;
	}

#if DBG
	static char *fcnname[] =
	{
		"IRP_MN_START_DEVICE",
		"QUERY_REMOVE_DEVICE",
		"REMOVE_DEVICE",
		"CANCEL_REMOVE_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"IRP_MN_START_DEVICE",
		"QUERY_REMOVE_DEVICE",
		"REMOVE_DEVICE",
		"CANCEL_REMOVE_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"IRP_MN_START_DEVICE",
		"QUERY_REMOVE_DEVICE",
		"REMOVE_DEVICE",
		"CANCEL_REMOVE_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"IRP_MN_START_DEVICE",
		"QUERY_REMOVE_DEVICE",
		"REMOVE_DEVICE",
		"CANCEL_REMOVE_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
		"STOP_DEVICE",
	};
	KdPrint(("PNP Request: %s\n", fcnname[fcn]));
#endif

	status = (*fcntab[fcn])(pDeviceExtension, Irp);
	KdPrint(("Leave HelloWDMPnp\n"));
	return status;
}

#pragma PAGEDCODE
NTSTATUS HelloWDMDispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(fdo);
	KdPrint(("Enter HelloWDMDispatchRoutine\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

#pragma PAGEDCODE
void HelloWDMUnload(IN PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);
	KdPrint(("in UNload\n"));
}


#pragma INITCODE
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING RegisteryPath)
{
	UNREFERENCED_PARAMETER(RegisteryPath);

	KdPrint(("Enter DriverEntry\n"));

	pDriverObject->DriverExtension->AddDevice = HelloWDMAddDevice;
	pDriverObject->MajorFunction[IRP_MJ_PNP] = HelloWDMPnp;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = HelloWDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = HelloWDMDispatchRoutine;
	pDriverObject->DriverUnload = HelloWDMUnload;

	KdPrint(("Leave DriverEntry\n"));

	return STATUS_SUCCESS;
}